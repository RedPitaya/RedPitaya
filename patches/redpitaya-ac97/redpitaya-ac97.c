/*
 * ALSA driver for RedPitaya AC97 Controller Emulation of the RadioBox sub-module
 *
 *  Copyright (c) by 2016  Ulrich Habel <espero7757@gmx.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

/* Some notes / status of this driver:
 *
 * This module is based on the ml403-ac97cr sound driver for the Xilinx ML403 board.
 * The RedPitaya allows additional applications to be run which can include its own
 * FPGA configuration. Known as the red_pitaya_ac97ctrl.sv sub-module, that one
 * features an AC97 controller that conforms nearly to the XILINX ML403 board AC97 IP.
 *
 * This module enables the ALSA sound system to access that FPGA to give a stereo
 * LINE-OUT stream and a stereo LINE-IN stream to the Linux kernel @ 48 kHz, LE. The
 * module shadows some AC97 registers, the FPGA shadows all 64 words of the AC97 CODEC
 * added by the status information register(s). The FPGA mimics the AC97 controller &
 * CODEC to access the ADC and DACs of the RedPitaya board. Currently hardware version
 * V1.1 is tested to work.
 *
 * Before ejecting the ALSA enabled FPGA module it is adviced to rmmod this sound module
 * first.
 *
 * At the time of writing the FPGA sub-module RadioBox uses this ALSA device to make
 * RadioBox attached to the Linux kernel sound system ALSA.
 *
 * Notice: Both, the FPGA registers are little endian and the kernel runs as little endian, too.
 * Thus no byte swapping is needed. That is in contrast to the ml403-ac97cr former driver.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/regmap.h>

#include <linux/platform_device.h>

#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/interrupt.h>

/* HZ */
#include <linux/param.h>
/* jiffies, time_*() */
#include <linux/jiffies.h>
/* schedule_timeout*() */
#include <linux/sched.h>
/* spin_lock*() */
#include <linux/spinlock.h>
/* struct mutex, mutex_init(), mutex_*lock() */
#include <linux/mutex.h>

/* snd_printk(), snd_printd() */
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/ac97_codec.h>

#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include "pcm-indirect2.h"


#define SND_REDPITAYA_AC97_DRIVER "redpitaya-ac97"

MODULE_AUTHOR("Ulrich Habel <espero7757@gmx.net>");
MODULE_DESCRIPTION("RedPitaya-AC97 FPGA sound system");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("{{RedPitaya,red_pitaya_ac97ctrl AC97 sub-module}}");


static int index = SNDRV_DEFAULT_IDX1;
static char *id = SNDRV_DEFAULT_STR1;
static bool enable = SNDRV_DEFAULT_ENABLE1;

module_param(index, int, 0444);
MODULE_PARM_DESC(index, "Index value for RedPitaya-AC97 FPGA sound system.");
module_param(id, charp, 0444);
MODULE_PARM_DESC(id, "ID string for RedPitaya-AC97 FPGA sound system.");
module_param(enable, bool, 0444);
MODULE_PARM_DESC(enable, "Enable this RedPitaya-AC97 FPGA sound system.");

/* Special feature options */
/*#define CODEC_WRITE_CHECK_RAF*/	/* don't return after a write to a codec
					 * register, while RAF bit is not set
					 */
/* Debug options for code which may be removed completely in a final version */
#undef CODEC_STAT
#undef SND_PCM_INDIRECT2_STAT
#undef CONFIG_SND_DEBUG

#ifdef CONFIG_SND_DEBUG
/*#define CODEC_STAT*/			/* turn on some minimal "statistics"
					 * about codec register usage
					 */
/*#define SND_PCM_INDIRECT2_STAT*/	/* turn on some "statistics" about the
					 * process of copying bytes from the
					 * intermediate buffer to the hardware
					 * fifo and the other way round
					 */
#endif


/* Definition of a "level/facility dependent" printk(); may be removed
 * completely in a final version
 */
#undef PDEBUG
#ifdef CONFIG_SND_DEBUG
/* "facilities" for PDEBUG */
#define UNKNOWN 	(1<<0)
#define CODEC_SUCCESS	(1<<1)
#define CODEC_FAKE	(1<<2)
#define INIT_INFO	(1<<3)
#define INIT_FAILURE	(1<<4)
#define WORK_INFO	(1<<5)
#define WORK_FAILURE	(1<<6)
#define ISR_INFO	(1<<7)

#define PDEBUG_FACILITIES (UNKNOWN | INIT_FAILURE | WORK_FAILURE)

#define PDEBUG(fac, fmt, args...) do { \
	if (fac & PDEBUG_FACILITIES) \
		snd_printd(KERN_DEBUG SND_REDPITAYA_AC97_DRIVER ": " \
			   fmt, ##args); \
	} while (0)
#else
#define PDEBUG(fac, fmt, args...) /* nothing */
#endif



/* Defines delay in millisecs */
#define CODEC_TIMEOUT_ON_INIT	  250	/* timeout for checking for codec
					 * readiness (after insmod)
					 */
#ifndef CODEC_WRITE_CHECK_RAF
#define CODEC_WAIT_AFTER_WRITE	    0	/* general, static wait after a write
					 * access to a codec register, may be
					 * 0 to completely remove wait
					 */
#else
#define CODEC_TIMEOUT_AFTER_WRITE  30	/* timeout after a write access to a
					 * codec register, if RAF bit is used
					 */
#endif
#define CODEC_TIMEOUT_AFTER_READ   30	/* timeout after a read access to a
					 * codec register (checking RAF bit)
					 */

#define BUFFER_MAX_B            96000
#define PERIODS_MAX_B           24000

/* Infrastructure for codec register shadowing */
#define LM4550_REG_OK		(1<<0)  /* register exists */
#define LM4550_REG_DONEREAD	(1<<1)  /* read register once, value should be
					 * the same currently in the register
					 */
#define LM4550_REG_NOSAVE	(1<<2)  /* values written to this register will
					 * not be saved in the register
					 */
#define LM4550_REG_NOSHADOW	(1<<3)  /* don't do register shadowing, use plain
					 * hardware access
					 */
#define LM4550_REG_READONLY	(1<<4)  /* register is read only */
#define LM4550_REG_FAKEPROBE	(1<<5)  /* fake write _and_ read actions during
					 * probe() correctly
					 */
#define LM4550_REG_FAKEREAD	(1<<6)  /* fake read access, always return
					 * default value
					 */
#define LM4550_REG_ALLFAKE	(LM4550_REG_FAKEREAD | LM4550_REG_FAKEPROBE)

struct lm4550_reg {
	u16 value;
	u16 flag;
	u16 wmask;
	u16 def;
};

struct lm4550_reg lm4550_regfile[64] = {
	[AC97_RESET >> 1]		= {.flag = LM4550_REG_OK \
						 | LM4550_REG_NOSAVE \
						 | LM4550_REG_FAKEREAD,
					   .def = 0x0D50},
	[AC97_MASTER >> 1]		= {.flag = LM4550_REG_OK
						 | LM4550_REG_FAKEPROBE,
					   .wmask = 0x9F1F,
					   .def = 0x8000},
	[AC97_HEADPHONE >> 1]		= {.flag = LM4550_REG_OK \
						 | LM4550_REG_FAKEPROBE,
					   .wmask = 0x9F1F,
					   .def = 0x8000},
	[AC97_MASTER_MONO >> 1]		= {.flag = LM4550_REG_OK \
						 | LM4550_REG_FAKEPROBE,
					   .wmask = 0x801F,
					   .def = 0x8000},
	[AC97_PC_BEEP >> 1]		= {.flag = LM4550_REG_OK \
						 | LM4550_REG_FAKEPROBE,
					   .wmask = 0x801E,
					   .def = 0x0},
	[AC97_PHONE >> 1]		= {.flag = LM4550_REG_OK \
						 | LM4550_REG_FAKEPROBE,
					   .wmask = 0x801F,
					   .def = 0x8008},
	[AC97_MIC >> 1]			= {.flag = LM4550_REG_OK \
						 | LM4550_REG_FAKEPROBE,
					   .wmask = 0x805F,
					   .def = 0x8008},
	[AC97_LINE >> 1]			= {.flag = LM4550_REG_OK \
						 | LM4550_REG_FAKEPROBE,
					   .wmask = 0x9F1F,
					   .def = 0x8808},
	[AC97_CD >> 1]			= {.flag = LM4550_REG_OK \
						 | LM4550_REG_FAKEPROBE,
					   .wmask = 0x9F1F,
					   .def = 0x8808},
	[AC97_VIDEO >> 1]		= {.flag = LM4550_REG_OK \
						 | LM4550_REG_FAKEPROBE,
					   .wmask = 0x9F1F,
					   .def = 0x8808},
	[AC97_AUX >> 1]			= {.flag = LM4550_REG_OK \
						 | LM4550_REG_FAKEPROBE,
					   .wmask = 0x9F1F,
					   .def = 0x8808},
	[AC97_PCM >> 1]			= {.flag = LM4550_REG_OK \
						 | LM4550_REG_FAKEPROBE,
					   .wmask = 0x9F1F,
					   .def = 0x8008},
	[AC97_REC_SEL >> 1]		= {.flag = LM4550_REG_OK \
						 | LM4550_REG_FAKEPROBE,
					   .wmask = 0x707,
					   .def = 0x0},
	[AC97_REC_GAIN >> 1]		= {.flag = LM4550_REG_OK \
						 | LM4550_REG_FAKEPROBE,
					   .wmask = 0x8F0F,
					   .def = 0x8000},
	[AC97_GENERAL_PURPOSE >> 1]	= {.flag = LM4550_REG_OK \
						 | LM4550_REG_FAKEPROBE,
					   .def = 0x0,
					   .wmask = 0xA380},
	[AC97_3D_CONTROL >> 1]		= {.flag = LM4550_REG_OK \
						 | LM4550_REG_FAKEREAD \
						 | LM4550_REG_READONLY,
					   .def = 0x0101},
	[AC97_POWERDOWN >> 1]		= {.flag = LM4550_REG_OK \
						 | LM4550_REG_NOSHADOW \
						 | LM4550_REG_NOSAVE,
					   .wmask = 0xFF00},
					/* may not write ones to
					 * REF/ANL/DAC/ADC bits
					 * FIXME: Is this ok?
					 */
	[AC97_EXTENDED_ID >> 1]		= {.flag = LM4550_REG_OK \
						 | LM4550_REG_FAKEREAD \
						 | LM4550_REG_READONLY,
					   .def = 0x0201}, /* primary codec */
	[AC97_EXTENDED_STATUS >> 1]	= {.flag = LM4550_REG_OK \
						 | LM4550_REG_NOSHADOW \
						 | LM4550_REG_NOSAVE,
					   .wmask = 0x1},
	[AC97_PCM_FRONT_DAC_RATE >> 1]   = {.flag = LM4550_REG_OK \
						 | LM4550_REG_FAKEPROBE,
					   .def = 0xBB80,
					   .wmask = 0xFFFF},
	[AC97_PCM_LR_ADC_RATE >> 1]	= {.flag = LM4550_REG_OK \
						 | LM4550_REG_FAKEPROBE,
					   .def = 0xBB80,
					   .wmask = 0xFFFF},
	[AC97_VENDOR_ID1 >> 1]		= {.flag = LM4550_REG_OK \
						 | LM4550_REG_READONLY \
						 | LM4550_REG_FAKEREAD,
					   .def = 0x4E53},
	[AC97_VENDOR_ID2 >> 1]		= {.flag = LM4550_REG_OK \
						 | LM4550_REG_READONLY \
						 | LM4550_REG_FAKEREAD,
					   .def = 0x4350}
};

#define LM4550_RF_OK(reg)	(lm4550_regfile[reg >> 1].flag & LM4550_REG_OK)

static void lm4550_regfile_init(void)
{
	int i;
	for (i = 0; i < 64; i++)
		if (lm4550_regfile[i].flag & LM4550_REG_FAKEPROBE)
			lm4550_regfile[i].value = lm4550_regfile[i].def;
}

static void lm4550_regfile_write_values_after_init(struct snd_ac97 *ac97)
{
	int i;
	for (i = 0; i < 64; i++)
		if ((lm4550_regfile[i].flag & LM4550_REG_FAKEPROBE) &&
			(lm4550_regfile[i].value != lm4550_regfile[i].def)) {
			PDEBUG(CODEC_FAKE, "lm4550_regfile_write_values_after_"
				   "init(): reg=0x%02x val=0x%04x / %d is different "
				   "from def=0x%04x / %d - snd_ac97_write() called to update\n",
				   (i << 1), lm4550_regfile[i].value,
				   lm4550_regfile[i].value, lm4550_regfile[i].def,
				   lm4550_regfile[i].def);
			snd_ac97_write(ac97, (i << 1), lm4550_regfile[i].value);
			lm4550_regfile[i].flag |= LM4550_REG_DONEREAD;
		}
}


/* direct registers */
#define CTRL_REG(redpitaya_ac97, x) (redpitaya_ac97->port + (CTRL_REG_##x))

#define CTRL_REG_PLAYFIFO 	    0x00
#define CTRL_PLAYDATA(a)	    ((a) & 0xFFFF)

#define CTRL_REG_RECFIFO	    0x04
#define CTRL_RECDATA(a) 	    ((a) & 0xFFFF)

#define CTRL_REG_STATUS		    0x08
#define   CTRL_RECOVER		    (1<<7)
#define   CTRL_PLAYUNDER	    (1<<6)
#define   CTRL_CODECREADY 	    (1<<5)
#define   CTRL_RAF		    (1<<4)
#define   CTRL_RECEMPTY		    (1<<3)
#define   CTRL_RECFULL		    (1<<2)
#define   CTRL_PLAYHALF		    (1<<1)
#define   CTRL_PLAYFULL		    (1<<0)

#define CTRL_REG_RESETFIFO	    0x0C
#define   CTRL_RECRESET		    (1<<1)
#define   CTRL_PLAYRESET	    (1<<0)

#define CTRL_REG_CODEC_ADDR	    0x10
#define   CTRL_CODEC_ADDR(a)	    (((a) & 0x7E)<<0)
#define   CTRL_CODEC_READ 	    (1<<7)
#define   CTRL_CODEC_WRITE	    (0<<7)

#define CTRL_REG_CODEC_DATAREAD     0x14
#define   CTRL_CODEC_DATAREAD(v)    ((v) & 0xFFFF)

#define CTRL_REG_CODEC_DATAWRITE    0x18
#define   CTRL_CODEC_DATAWRITE(v)   ((v) & 0xFFFF)

#define CTRL_FIFO_SIZE		    32

struct snd_redpitaya_ac97 {
	/* lock for access to (controller) registers */
	spinlock_t reg_lock;
	/* mutex for the whole sequence of accesses to (controller) registers
	 * which affect codec registers
	 */
	struct mutex cdc_mutex;

	int playback_irq;
	int enable_playback_irq;

	int capture_irq;
	int enable_capture_irq;

	void *port_phy;
	void __iomem *port;
	u32 port_size;

	struct snd_ac97 *ac97;
	int ac97_fake;
#ifdef CODEC_STAT
	int ac97_read;
	int ac97_write;
#endif

	struct platform_device *pdev;
	struct snd_card *card;
	struct snd_pcm *pcm;
	struct snd_pcm_substream *playback_substream;
	struct snd_pcm_substream *capture_substream;

	struct snd_pcm_indirect2 playback_ind2_pcm;
	struct snd_pcm_indirect2 capture_ind2_pcm;
};

static struct snd_pcm_hardware snd_redpitaya_ac97_playback = {
	.info =			(SNDRV_PCM_INFO_MMAP |
				 SNDRV_PCM_INFO_INTERLEAVED |
				 SNDRV_PCM_INFO_MMAP_VALID),
	.formats =		 SNDRV_PCM_FMTBIT_S16_LE,
	.rates =		 SNDRV_PCM_RATE_48000,
	.rate_min =		48000,
	.rate_max =		48000,
	.channels_min =		1,
	.channels_max =		2,
	.buffer_bytes_max =     BUFFER_MAX_B,
	.period_bytes_min =     CTRL_FIFO_SIZE >> 1,
	.period_bytes_max =     PERIODS_MAX_B,
	.periods_min =		2,
	.periods_max =		BUFFER_MAX_B / (CTRL_FIFO_SIZE >> 1),
	.fifo_size =		0,
};

static struct snd_pcm_hardware snd_redpitaya_ac97_capture = {
	.info =			(SNDRV_PCM_INFO_MMAP |
				 SNDRV_PCM_INFO_INTERLEAVED |
				 SNDRV_PCM_INFO_MMAP_VALID),
	.formats =		 SNDRV_PCM_FMTBIT_S16_LE,
	.rates =		 SNDRV_PCM_RATE_48000,
	.rate_min =		48000,
	.rate_max =		48000,
	.channels_min =		1,
	.channels_max =		2,
	.buffer_bytes_max =     BUFFER_MAX_B,
	.period_bytes_min =     CTRL_FIFO_SIZE >> 1,
	.period_bytes_max =     PERIODS_MAX_B,
	.periods_min =		2,
	.periods_max =		BUFFER_MAX_B / (CTRL_FIFO_SIZE >> 1),
	.fifo_size =		0,
};

static size_t
snd_redpitaya_ac97_playback_ind2_zero(struct snd_pcm_substream *substream,
				      struct snd_pcm_indirect2 *pcm)
{
	struct snd_redpitaya_ac97 *redpitaya_ac97;
	int copied_words = 0;
	u32 status = 0;

	redpitaya_ac97 = snd_pcm_substream_chip(substream);

	spin_lock(&redpitaya_ac97->reg_lock);
	while ((status = (ioread32(CTRL_REG(redpitaya_ac97, STATUS)) &
				   CTRL_PLAYFULL)) != CTRL_PLAYFULL) {
		iowrite32(0, CTRL_REG(redpitaya_ac97, PLAYFIFO));
		copied_words++;
	}
	pcm->hw_ready = 0;
	spin_unlock(&redpitaya_ac97->reg_lock);

	return (size_t) (copied_words << 1);
}

static size_t
snd_redpitaya_ac97_playback_ind2_copy(struct snd_pcm_substream *substream,
				     struct snd_pcm_indirect2 *pcm,
				     size_t bytes)
{
	struct snd_redpitaya_ac97 *redpitaya_ac97;
	u16 *src;
	int copied_words = 0;
	u32 status = 0;

	redpitaya_ac97 = snd_pcm_substream_chip(substream);
	src = (u16 *)(substream->runtime->dma_area + pcm->sw_data);

        PDEBUG(ISR_INFO, "ind2_copy(playback): copying %d bytes to the FPGA ...\n", bytes);
	spin_lock(&redpitaya_ac97->reg_lock);
	while (((status = (ioread32(CTRL_REG(redpitaya_ac97, STATUS)) &
				    CTRL_PLAYFULL)) != CTRL_PLAYFULL) && (bytes > 1)) {
		iowrite32(CTRL_PLAYDATA(src[copied_words++]), CTRL_REG(redpitaya_ac97, PLAYFIFO));
		bytes -= 2;
	}
	if (status != CTRL_PLAYFULL)
		pcm->hw_ready = 1;
	else
		pcm->hw_ready = 0;
	spin_unlock(&redpitaya_ac97->reg_lock);
        PDEBUG(ISR_INFO, "ind2_copy(playback): ... copied_words = %d, hw_ready = %d, done.\n", copied_words, pcm->hw_ready);

	return (size_t) (copied_words << 1);
}

static size_t
snd_redpitaya_ac97_capture_ind2_null(struct snd_pcm_substream *substream,
				     struct snd_pcm_indirect2 *pcm)
{
	struct snd_redpitaya_ac97 *redpitaya_ac97;
	int copied_words = 0;

	redpitaya_ac97 = snd_pcm_substream_chip(substream);

	spin_lock(&redpitaya_ac97->reg_lock);
	while ((ioread32(CTRL_REG(redpitaya_ac97, STATUS)) &
			 CTRL_RECEMPTY) != CTRL_RECEMPTY) {
		(void) ioread32(CTRL_REG(redpitaya_ac97, RECFIFO));
		copied_words++;
	}
	pcm->hw_ready = 1;
	spin_unlock(&redpitaya_ac97->reg_lock);

	return (size_t) (copied_words << 1);
}

static size_t
snd_redpitaya_ac97_capture_ind2_copy(struct snd_pcm_substream *substream,
				     struct snd_pcm_indirect2 *pcm, size_t bytes)
{
	struct snd_redpitaya_ac97 *redpitaya_ac97;
	u16 *dst;
	int copied_words = 0;
	u32 status = 0;

	redpitaya_ac97 = snd_pcm_substream_chip(substream);
	dst = (u16 *)(substream->runtime->dma_area + pcm->sw_data);

	PDEBUG(ISR_INFO, "ind2_copy(capture): copying %d bytes from the FPGA ...\n", bytes);
	spin_lock(&redpitaya_ac97->reg_lock);
	while (((status = (ioread32(CTRL_REG(redpitaya_ac97, STATUS)) &
				    CTRL_RECEMPTY)) != CTRL_RECEMPTY) && (bytes > 1)) {
		dst[copied_words++] = CTRL_RECDATA(ioread32(CTRL_REG(redpitaya_ac97,
							    RECFIFO)));
		bytes -= 2;
	}
	if (status != CTRL_RECEMPTY)
		pcm->hw_ready = 1;
	else
		pcm->hw_ready = 0;
	spin_unlock(&redpitaya_ac97->reg_lock);
        PDEBUG(ISR_INFO, "ind2_copy(capture): ... done. hw_ready = %d\n", pcm->hw_ready);

	return (size_t) (copied_words << 1);
}

static snd_pcm_uframes_t
snd_redpitaya_ac97_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct snd_redpitaya_ac97 *redpitaya_ac97;
	struct snd_pcm_indirect2 *ind2_pcm = NULL;

	redpitaya_ac97 = snd_pcm_substream_chip(substream);

	if (substream == redpitaya_ac97->playback_substream)
		ind2_pcm = &redpitaya_ac97->playback_ind2_pcm;
	if (substream == redpitaya_ac97->capture_substream)
		ind2_pcm = &redpitaya_ac97->capture_ind2_pcm;

	if (ind2_pcm != NULL)
		return snd_pcm_indirect2_pointer(substream, ind2_pcm);
	return (snd_pcm_uframes_t) 0;
}

static int
snd_redpitaya_ac97_pcm_playback_trigger(struct snd_pcm_substream *substream,
				        int cmd)
{
	struct snd_redpitaya_ac97 *redpitaya_ac97;
	int err = 0;

	redpitaya_ac97 = snd_pcm_substream_chip(substream);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		PDEBUG(WORK_INFO, "trigger(playback): START\n");
		redpitaya_ac97->playback_ind2_pcm.hw_ready = 1;

		/* clear play FIFO */
		iowrite32(CTRL_PLAYRESET, CTRL_REG(redpitaya_ac97, RESETFIFO));

		/* enable play irq */
		redpitaya_ac97->enable_playback_irq = 1;
		enable_irq(redpitaya_ac97->playback_irq);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		PDEBUG(WORK_INFO, "trigger(playback): STOP\n");

		/* disable play irq */
		disable_irq_nosync(redpitaya_ac97->playback_irq);
		redpitaya_ac97->enable_playback_irq = 0;

		redpitaya_ac97->playback_ind2_pcm.hw_ready = 0;
#ifdef SND_PCM_INDIRECT2_STAT
		snd_pcm_indirect2_stat(substream, &redpitaya_ac97->playback_ind2_pcm);
#endif
		break;
	default:
		err = -EINVAL;
		break;
	}
	PDEBUG(WORK_INFO, "trigger(playback): (done)\n");
	return err;
}

static int
snd_redpitaya_ac97_pcm_capture_trigger(struct snd_pcm_substream *substream,
				       int cmd)
{
	struct snd_redpitaya_ac97 *redpitaya_ac97;
	int err = 0;

	redpitaya_ac97 = snd_pcm_substream_chip(substream);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		PDEBUG(WORK_INFO, "trigger(capture): START\n");
		redpitaya_ac97->capture_ind2_pcm.hw_ready = 1;

		/* clear record FIFO */
		iowrite32(CTRL_RECRESET, CTRL_REG(redpitaya_ac97, RESETFIFO));

		/* enable record irq */
		redpitaya_ac97->enable_capture_irq = 1;
		enable_irq(redpitaya_ac97->capture_irq);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		PDEBUG(WORK_INFO, "trigger(capture): STOP\n");

		/* disable capture irq */
		disable_irq_nosync(redpitaya_ac97->capture_irq);
		redpitaya_ac97->enable_capture_irq = 0;

		redpitaya_ac97->capture_ind2_pcm.hw_ready = 0;
#ifdef SND_PCM_INDIRECT2_STAT
		snd_pcm_indirect2_stat(substream,
				       &redpitaya_ac97->capture_ind2_pcm);
#endif
		break;
	default:
		err = -EINVAL;
		break;
	}
	PDEBUG(WORK_INFO, "trigger(capture): (done)\n");
	return err;
}

static int
snd_redpitaya_ac97_pcm_playback_prepare(struct snd_pcm_substream *substream)
{
	struct snd_redpitaya_ac97 *redpitaya_ac97;
	struct snd_pcm_runtime *runtime;

	redpitaya_ac97 = snd_pcm_substream_chip(substream);
	runtime = substream->runtime;

	PDEBUG(WORK_INFO,
		   "prepare(): period_bytes=%d, minperiod_bytes=%d\n",
		   snd_pcm_lib_period_bytes(substream), CTRL_FIFO_SIZE >> 1);

	/* set sampling rate */
	snd_ac97_set_rate(redpitaya_ac97->ac97, AC97_PCM_FRONT_DAC_RATE,
			  runtime->rate);
	PDEBUG(WORK_INFO, "prepare(): rate=%d\n", runtime->rate);

	/* init struct for intermediate buffer */
	memset(&redpitaya_ac97->playback_ind2_pcm, 0, sizeof(struct snd_pcm_indirect2));
	redpitaya_ac97->playback_ind2_pcm.hw_buffer_size = CTRL_FIFO_SIZE;
	redpitaya_ac97->playback_ind2_pcm.sw_buffer_size =
		snd_pcm_lib_buffer_bytes(substream);
	redpitaya_ac97->playback_ind2_pcm.min_periods = -1;
	redpitaya_ac97->playback_ind2_pcm.min_multiple =
		snd_pcm_lib_period_bytes(substream) / (CTRL_FIFO_SIZE >> 1);
	PDEBUG(WORK_INFO, "prepare(): hw_buffer_size=%d, "
		   "sw_buffer_size=%d, min_multiple=%d\n",
		   CTRL_FIFO_SIZE, redpitaya_ac97->playback_ind2_pcm.sw_buffer_size,
		   redpitaya_ac97->playback_ind2_pcm.min_multiple);
	return 0;
}

static int
snd_redpitaya_ac97_pcm_capture_prepare(struct snd_pcm_substream *substream)
{
	struct snd_redpitaya_ac97 *redpitaya_ac97;
	struct snd_pcm_runtime *runtime;

	redpitaya_ac97 = snd_pcm_substream_chip(substream);
	runtime = substream->runtime;

	PDEBUG(WORK_INFO,
		   "prepare(capture): period_bytes=%d, minperiod_bytes=%d\n",
		   snd_pcm_lib_period_bytes(substream), CTRL_FIFO_SIZE >> 1);

	/* set sampling rate */
	snd_ac97_set_rate(redpitaya_ac97->ac97, AC97_PCM_LR_ADC_RATE,
			  runtime->rate);
	PDEBUG(WORK_INFO, "prepare(capture): rate=%d\n", runtime->rate);

	/* init struct for intermediate buffer */
	memset(&redpitaya_ac97->capture_ind2_pcm, 0,
	       sizeof(struct snd_pcm_indirect2));
	redpitaya_ac97->capture_ind2_pcm.hw_buffer_size = CTRL_FIFO_SIZE;
	redpitaya_ac97->capture_ind2_pcm.sw_buffer_size =
		snd_pcm_lib_buffer_bytes(substream);
	redpitaya_ac97->capture_ind2_pcm.min_multiple =
		snd_pcm_lib_period_bytes(substream) / (CTRL_FIFO_SIZE >> 1);
	PDEBUG(WORK_INFO, "prepare(capture): hw_buffer_size=%d, "
	       "sw_buffer_size=%d, min_multiple=%d\n", CTRL_FIFO_SIZE,
	       redpitaya_ac97->capture_ind2_pcm.sw_buffer_size,
	       redpitaya_ac97->capture_ind2_pcm.min_multiple);
	return 0;
}

static int
snd_redpitaya_ac97_hw_free(struct snd_pcm_substream *substream)
{
	PDEBUG(WORK_INFO, "hw_free()\n");
	return snd_pcm_lib_free_pages(substream);
}

static int
snd_redpitaya_ac97_hw_params(struct snd_pcm_substream *substream,
			     struct snd_pcm_hw_params *hw_params)
{
	PDEBUG(WORK_INFO, "hw_params() - desired buffer bytes=%d, desired "
		   "period bytes=%d\n",
		   params_buffer_bytes(hw_params), params_period_bytes(hw_params));
	return snd_pcm_lib_malloc_pages(substream,
					params_buffer_bytes(hw_params));
}

static int
snd_redpitaya_ac97_playback_open(struct snd_pcm_substream *substream)
{
	struct snd_redpitaya_ac97 *redpitaya_ac97;
	struct snd_pcm_runtime *runtime;

	redpitaya_ac97 = snd_pcm_substream_chip(substream);
	runtime = substream->runtime;

	PDEBUG(WORK_INFO, "open(playback)\n");
	redpitaya_ac97->playback_substream = substream;
	runtime->hw = snd_redpitaya_ac97_playback;

	snd_pcm_hw_constraint_step(runtime, 0,
				   SNDRV_PCM_HW_PARAM_PERIOD_BYTES,
				   CTRL_FIFO_SIZE >> 1);
	return 0;
}

static int
snd_redpitaya_ac97_capture_open(struct snd_pcm_substream *substream)
{
	struct snd_redpitaya_ac97 *redpitaya_ac97;
	struct snd_pcm_runtime *runtime;

	redpitaya_ac97 = snd_pcm_substream_chip(substream);
	runtime = substream->runtime;

	PDEBUG(WORK_INFO, "open(capture)\n");
	redpitaya_ac97->capture_substream = substream;
	runtime->hw = snd_redpitaya_ac97_capture;

	snd_pcm_hw_constraint_step(runtime, 0,
				   SNDRV_PCM_HW_PARAM_PERIOD_BYTES,
				   CTRL_FIFO_SIZE >> 1);
	return 0;
}

static int
snd_redpitaya_ac97_playback_close(struct snd_pcm_substream *substream)
{
	struct snd_redpitaya_ac97 *redpitaya_ac97;

	redpitaya_ac97 = snd_pcm_substream_chip(substream);

	PDEBUG(WORK_INFO, "close(playback)\n");
	redpitaya_ac97->playback_substream = NULL;
	return 0;
}

static int
snd_redpitaya_ac97_capture_close(struct snd_pcm_substream *substream)
{
	struct snd_redpitaya_ac97 *redpitaya_ac97;

	redpitaya_ac97 = snd_pcm_substream_chip(substream);

	PDEBUG(WORK_INFO, "close(capture)\n");
	redpitaya_ac97->capture_substream = NULL;
	return 0;
}

static struct snd_pcm_ops snd_redpitaya_ac97_playback_ops = {
        .open        = snd_redpitaya_ac97_playback_open,
	.close       = snd_redpitaya_ac97_playback_close,
	.ioctl       = snd_pcm_lib_ioctl,
	.hw_params   = snd_redpitaya_ac97_hw_params,
	.hw_free     = snd_redpitaya_ac97_hw_free,
	.prepare     = snd_redpitaya_ac97_pcm_playback_prepare,
	.trigger     = snd_redpitaya_ac97_pcm_playback_trigger,
	.pointer     = snd_redpitaya_ac97_pcm_pointer,
};

static struct snd_pcm_ops snd_redpitaya_ac97_capture_ops = {
	.open        = snd_redpitaya_ac97_capture_open,
	.close       = snd_redpitaya_ac97_capture_close,
	.ioctl       = snd_pcm_lib_ioctl,
	.hw_params   = snd_redpitaya_ac97_hw_params,
	.hw_free     = snd_redpitaya_ac97_hw_free,
	.prepare     = snd_redpitaya_ac97_pcm_capture_prepare,
	.trigger     = snd_redpitaya_ac97_pcm_capture_trigger,
	.pointer     = snd_redpitaya_ac97_pcm_pointer,
};


/*********************************************************************
 * of_device_id
 *********************************************************************/
static struct of_device_id redpitaya_ac97_dt_ids[] = {
	{ .compatible = "redpitaya,redpitaya-ac97", },
	{ }
};
MODULE_DEVICE_TABLE(of, redpitaya_ac97_dt_ids);

/* work with hotplug and coldplug */
//MODULE_ALIAS("platform:" SND_REDPITAYA_AC97_DRIVER);
//MODULE_ALIAS("of:" SND_REDPITAYA_AC97_DRIVER);

/* forward declarations */
static int snd_redpitaya_ac97_probe(struct platform_device *pdev);
static int snd_redpitaya_ac97_remove(struct platform_device *pdev);

static struct platform_driver snd_redpitaya_ac97_driver = {
	.probe       = snd_redpitaya_ac97_probe,
	.remove      = snd_redpitaya_ac97_remove,
	.driver      = {
		         .name           = SND_REDPITAYA_AC97_DRIVER,
		         .owner          = THIS_MODULE,
		         .of_match_table = of_match_ptr(redpitaya_ac97_dt_ids),
	},
};


static irqreturn_t
snd_redpitaya_ac97_irq(int irq, void *dev_id)
{
	struct snd_redpitaya_ac97 *redpitaya_ac97;

	PDEBUG(ISR_INFO, "irq(): IRQ = %d, dev_id = 0x%p\n", irq, dev_id);

	redpitaya_ac97 = (struct snd_redpitaya_ac97 *)dev_id;
	if (redpitaya_ac97 == NULL)
		return IRQ_NONE;

	if (irq == redpitaya_ac97->playback_irq) {	/* playback interrupt */
		PDEBUG(ISR_INFO, "irq(): play - enable_playback_irq = %d\n", redpitaya_ac97->enable_playback_irq);
		if (redpitaya_ac97->enable_playback_irq) {
			snd_pcm_indirect2_playback_interrupt(
				redpitaya_ac97->playback_substream,
				&redpitaya_ac97->playback_ind2_pcm,
				snd_redpitaya_ac97_playback_ind2_copy,
				snd_redpitaya_ac97_playback_ind2_zero);
			return IRQ_HANDLED;
		}

	} else if (irq == redpitaya_ac97->capture_irq) {  /* capture interrupt */
		PDEBUG(ISR_INFO, "irq(): capture - enable_capture_irq = %d\n", redpitaya_ac97->enable_capture_irq);
		if (redpitaya_ac97->enable_capture_irq) {
			snd_pcm_indirect2_capture_interrupt(
				redpitaya_ac97->capture_substream,
				&redpitaya_ac97->capture_ind2_pcm,
				snd_redpitaya_ac97_capture_ind2_copy,
				snd_redpitaya_ac97_capture_ind2_null);
			return IRQ_HANDLED;
		}

	} else
		return IRQ_NONE;

	/* superfluous IRQ catched */
	PDEBUG(WORK_FAILURE, "irq(): irq %d is meant to be disabled! So, now try "
		   "to disable it _really_!\n", irq);
	disable_irq_nosync(irq);
	return IRQ_HANDLED;
}


static unsigned short
snd_redpitaya_ac97_codec_read_internal(struct snd_redpitaya_ac97 *redpitaya_ac97, unsigned short reg)
{
#ifdef CODEC_STAT
	u32 stat;
	u32 rafaccess = 0;
#endif
	unsigned long end_time;
	u16 value = 0;

	reg &= 0xfe;

	if (!LM4550_RF_OK(reg)) {
		snd_printk(KERN_WARNING SND_REDPITAYA_AC97_DRIVER ": "
			   "access to unknown / unused codec register 0x%x "
			   "ignored!\n", reg);
		return 0;
	}

	/* check if we can fake / answer this access from our shadow register */
	if ((lm4550_regfile[reg >> 1].flag &
		 (LM4550_REG_DONEREAD | LM4550_REG_ALLFAKE)) &&
		!(lm4550_regfile[reg >> 1].flag & LM4550_REG_NOSHADOW)) {
		if (lm4550_regfile[reg >> 1].flag & LM4550_REG_FAKEREAD) {
			PDEBUG(CODEC_FAKE, "codec_read(): faking read from "
				   "reg=0x%02x, val=0x%04x / %d\n",
				   reg, lm4550_regfile[reg >> 1].def,
				   lm4550_regfile[reg >> 1].def);
			return lm4550_regfile[reg >> 1].def;
		} else if ((lm4550_regfile[reg >> 1].flag &
				LM4550_REG_FAKEPROBE) &&
			   redpitaya_ac97->ac97_fake) {
			PDEBUG(CODEC_FAKE, "codec_read(): faking read from "
				   "reg=0x%02x, val=0x%04x / %d (probe)\n",
				   reg, lm4550_regfile[reg >> 1].value,
				   lm4550_regfile[reg >> 1].value);
			return lm4550_regfile[reg >> 1].value;
		} else {
#ifdef CODEC_STAT
			PDEBUG(CODEC_FAKE, "codec_read(): read access "
				   "answered by shadow register 0x%x (value=0x%x "
				   "/ %d) (cw=%d cr=%d)\n",
				   reg, lm4550_regfile[reg >> 1].value,
				   lm4550_regfile[reg >> 1].value,
				   redpitaya_ac97->ac97_write,
				   redpitaya_ac97->ac97_read);
#else
			PDEBUG(CODEC_FAKE, "codec_read(): read access "
				   "answered by shadow register 0x%x (value=0x%x "
				   "/ %d)\n",
				   reg, lm4550_regfile[reg >> 1].value,
				   lm4550_regfile[reg >> 1].value);
#endif
			return lm4550_regfile[reg >> 1].value;
		}
	}
	/* if we are here, we _have_ to access the codec really, no faking */
	if (mutex_lock_interruptible(&redpitaya_ac97->cdc_mutex) != 0)
		return 0;
#ifdef CODEC_STAT
	redpitaya_ac97->ac97_read++;
#endif
	spin_lock(&redpitaya_ac97->reg_lock);
	iowrite32(CTRL_CODEC_ADDR(reg) | CTRL_CODEC_READ, CTRL_REG(redpitaya_ac97, CODEC_ADDR));
	spin_unlock(&redpitaya_ac97->reg_lock);
	end_time = jiffies + ((HZ * CODEC_TIMEOUT_AFTER_READ) / 1000);
	do {
		spin_lock(&redpitaya_ac97->reg_lock);
#ifdef CODEC_STAT
		rafaccess++;
		stat = ioread32(CTRL_REG(redpitaya_ac97, STATUS));
		if ((stat & CTRL_RAF) == CTRL_RAF) {
			value = CTRL_CODEC_DATAREAD(
				   ioread32(CTRL_REG(redpitaya_ac97, CODEC_DATAREAD)));
			PDEBUG(CODEC_SUCCESS, "codec_read(): (done) reg=0x%02x, "
				   "val=0x%04x / %d (STATUS=0x%04x)\n",
				   reg, value, value, stat);
#else
		if ((ioread32(CTRL_REG(redpitaya_ac97, STATUS)) &
			 CTRL_RAF) == CTRL_RAF) {
			value = CTRL_CODEC_DATAREAD(
				   ioread32(CTRL_REG(redpitaya_ac97, CODEC_DATAREAD)));
			PDEBUG(CODEC_SUCCESS, "codec_read(): (done) "
				   "reg=0x%02x, val=0x%04x / %d\n",
				   reg, value, value);
#endif
			lm4550_regfile[reg >> 1].value = value;
			lm4550_regfile[reg >> 1].flag |= LM4550_REG_DONEREAD;
			spin_unlock(&redpitaya_ac97->reg_lock);
			mutex_unlock(&redpitaya_ac97->cdc_mutex);
			return value;
		}
		spin_unlock(&redpitaya_ac97->reg_lock);
		schedule_timeout_uninterruptible(1);
	} while (time_after(end_time, jiffies));
	/* read the DATAREAD register anyway, see comment below */
	spin_lock(&redpitaya_ac97->reg_lock);
	value = CTRL_CODEC_DATAREAD(ioread32(CTRL_REG(redpitaya_ac97, CODEC_DATAREAD)));
	spin_unlock(&redpitaya_ac97->reg_lock);
#ifdef CODEC_STAT
	snd_printk(KERN_WARNING SND_REDPITAYA_AC97_DRIVER ": "
		   "timeout while codec read! "
		   "(reg=0x%02x, last STATUS=0x%04x, DATAREAD=0x%04x / %d, %d) "
		   "(cw=%d, cr=%d)\n",
		   reg, stat, value, value, rafaccess,
		   redpitaya_ac97->ac97_write, redpitaya_ac97->ac97_read);
#else
	snd_printk(KERN_WARNING SND_REDPITAYA_AC97_DRIVER ": "
		   "timeout while codec read! "
		   "(reg=0x%02x, DATAREAD=0x%04x / %d)\n",
		   reg, value, value);
#endif
	/* BUG: This is PURE speculation! But after _most_ read timeouts the
	 * value in the register is ok!
	 */
	lm4550_regfile[reg >> 1].value = value;
	lm4550_regfile[reg >> 1].flag |= LM4550_REG_DONEREAD;
	mutex_unlock(&redpitaya_ac97->cdc_mutex);
	return value;
}

static unsigned short
snd_redpitaya_ac97_codec_read(struct snd_ac97 *ac97, unsigned short reg)
{
	return snd_redpitaya_ac97_codec_read_internal((struct snd_redpitaya_ac97 *)ac97->private_data, reg);
}


static void
snd_redpitaya_ac97_codec_write_internal(struct snd_redpitaya_ac97 *redpitaya_ac97, unsigned short reg, unsigned short val)
{
#ifdef CODEC_STAT
	u32 stat;
	u32 rafaccess = 0;
#endif
#ifdef CODEC_WRITE_CHECK_RAF
	unsigned long end_time;
#endif

	reg &= 0xfe;
	PDEBUG(CODEC_FAKE, "write(reg=%02x, val=%04x / %d)\n", reg, val, val);

	if (!LM4550_RF_OK(reg)) {
		snd_printk(KERN_WARNING SND_REDPITAYA_AC97_DRIVER ": "
			   "access to unknown / unused codec register 0x%x "
			   "ignored!\n", reg);
		return;
	}
	if (lm4550_regfile[reg >> 1].flag & LM4550_REG_READONLY) {
		snd_printk(KERN_WARNING SND_REDPITAYA_AC97_DRIVER ": "
			   "write access to read only codec register 0x%x "
			   "ignored!\n", reg);
		return;
	}
	if ((val & lm4550_regfile[reg >> 1].wmask) != val) {
		snd_printk(KERN_WARNING SND_REDPITAYA_AC97_DRIVER ": "
			   "write access to codec register 0x%x "
			   "with bad value 0x%x / %d!\n",
			   reg, val, val);
		val = val & lm4550_regfile[reg >> 1].wmask;
	}
	if (((lm4550_regfile[reg >> 1].flag & LM4550_REG_FAKEPROBE) &&
	      redpitaya_ac97->ac97_fake) &&
		!(lm4550_regfile[reg >> 1].flag & LM4550_REG_NOSHADOW)) {
		PDEBUG(CODEC_FAKE, "codec_write(): faking write to reg=0x%02x, "
			   "val=0x%04x / %d\n", reg, val, val);
		lm4550_regfile[reg >> 1].value = (val &
						 lm4550_regfile[reg >> 1].wmask);
		return;
	}
	if (mutex_lock_interruptible(&redpitaya_ac97->cdc_mutex) != 0)
		return;
#ifdef CODEC_STAT
	redpitaya_ac97->ac97_write++;
#endif
	spin_lock(&redpitaya_ac97->reg_lock);
	iowrite32(CTRL_CODEC_DATAWRITE(val), CTRL_REG(redpitaya_ac97, CODEC_DATAWRITE));
	iowrite32(CTRL_CODEC_ADDR(reg) | CTRL_CODEC_WRITE, CTRL_REG(redpitaya_ac97, CODEC_ADDR));
	spin_unlock(&redpitaya_ac97->reg_lock);
#ifdef CODEC_WRITE_CHECK_RAF
	/* check CTRL_CODEC_RAF bit to see if write access to register is done;
	 * loop until bit is set or timeout happens
	 */
	end_time = jiffies + ((HZ * CODEC_TIMEOUT_AFTER_WRITE) / 1000);
	do {
		spin_lock(&redpitaya_ac97->reg_lock);
#ifdef CODEC_STAT
		rafaccess++;
		stat = ioread32(CTRL_REG(redpitaya_ac97, STATUS))
		if ((stat & CTRL_RAF) == CTRL_RAF) {
#else
		if ((ioread32(CTRL_REG(redpitaya_ac97, STATUS)) &
			 CTRL_RAF) == CTRL_RAF) {
#endif
			PDEBUG(CODEC_SUCCESS, "codec_write(): (done) "
			       "reg=0x%02x, val=0x%04x / %d\n",
			       reg, val, val);
			if (!(lm4550_regfile[reg >> 1].flag &
			      LM4550_REG_NOSHADOW) &&
				!(lm4550_regfile[reg >> 1].flag &
				  LM4550_REG_NOSAVE))
				lm4550_regfile[reg >> 1].value = val;
			lm4550_regfile[reg >> 1].flag |= LM4550_REG_DONEREAD;
			spin_unlock(&redpitaya_ac97->reg_lock);
			mutex_unlock(&redpitaya_ac97->cdc_mutex);
			return;
		}
		spin_unlock(&redpitaya_ac97->reg_lock);
		schedule_timeout_uninterruptible(1);
	} while (time_after(end_time, jiffies));
#ifdef CODEC_STAT
	snd_printk(KERN_WARNING SND_REDPITAYA_AC97_DRIVER ": "
		   "timeout while codec write "
		   "(reg=0x%02x, val=0x%04x / %d, last STATUS=0x%04x, %d) "
		   "(cw=%d, cr=%d)\n",
		   reg, val, val, stat, rafaccess, redpitaya_ac97->ac97_write,
		   redpitaya_ac97->ac97_read);
#else
	snd_printk(KERN_WARNING SND_REDPITAYA_AC97_DRIVER ": "
		   "timeout while codec write (reg=0x%02x, val=0x%04x / %d)\n",
		   reg, val, val);
#endif
#else   /* CODEC_WRITE_CHECK_RAF */
#if CODEC_WAIT_AFTER_WRITE > 0
	/* officially, in AC97 spec there is no possibility for a AC97
	 * controller to determine, if write access is done or not - so: How
	 * is Xilinx able to provide a RAF bit for write access?
	 * => very strange, thus just don't check RAF bit (compare with
	 * Xilinx's example app in EDK 8.1i) and wait
	 */
	schedule_timeout_uninterruptible(HZ / CODEC_WAIT_AFTER_WRITE);
#endif
	PDEBUG(CODEC_SUCCESS, "codec_write(): (done) "
	       "reg=0x%02x, val=0x%04x / %d (no RAF check)\n",
	       reg, val, val);
#endif
	mutex_unlock(&redpitaya_ac97->cdc_mutex);
	return;
}

static void
snd_redpitaya_ac97_codec_write(struct snd_ac97 *ac97, unsigned short reg, unsigned short val)
{
	return snd_redpitaya_ac97_codec_write_internal((struct snd_redpitaya_ac97 *)ac97->private_data, reg, val);
}


static int
snd_redpitaya_ac97_chip_init(struct snd_redpitaya_ac97 *redpitaya_ac97)
{
	unsigned long end_time;
	PDEBUG(INIT_INFO, "chip_init()\n");

	end_time = jiffies + ((HZ * CODEC_TIMEOUT_ON_INIT) / 1000);
	do {
		if (ioread32(CTRL_REG(redpitaya_ac97, STATUS)) & CTRL_CODECREADY) {
			/* clear both hardware FIFOs */
			iowrite32(CTRL_RECRESET | CTRL_PLAYRESET, CTRL_REG(redpitaya_ac97, RESETFIFO));
			PDEBUG(INIT_INFO, "chip_init(): (done)\n");
			return 0;
		}
		schedule_timeout_uninterruptible(1);
	} while (time_after(end_time, jiffies));
	snd_printk(KERN_ERR SND_REDPITAYA_AC97_DRIVER ": "
		   "timeout while waiting for codec, "
		   "not ready!\n");
	return -EBUSY;
}

static int
snd_redpitaya_ac97_free(struct snd_redpitaya_ac97 *redpitaya_ac97)
{
	PDEBUG(INIT_INFO, "free()\n");
	/* irq release */
	if (redpitaya_ac97->playback_irq >= 0)
		free_irq(redpitaya_ac97->playback_irq, redpitaya_ac97);
	if (redpitaya_ac97->capture_irq >= 0)
		free_irq(redpitaya_ac97->capture_irq, redpitaya_ac97);
	PDEBUG(INIT_INFO, "free() IRQs returned\n");
	/* give back "port" */
	iounmap(redpitaya_ac97->port);
	PDEBUG(INIT_INFO, "free() iounmap() done\n");
	release_mem_region((u32)redpitaya_ac97->port_phy, redpitaya_ac97->port_size);
	PDEBUG(INIT_INFO, "free() release_mem_region() done\n");
	kfree(redpitaya_ac97);
	PDEBUG(INIT_INFO, "free() (done)\n");
	return 0;
}

static int
snd_redpitaya_ac97_dev_free(struct snd_device *snddev)
{
	struct snd_redpitaya_ac97 *redpitaya_ac97 = snddev->device_data;
	PDEBUG(INIT_INFO, "dev_free()\n");
	return snd_redpitaya_ac97_free(redpitaya_ac97);
}

static int
snd_redpitaya_ac97_create(struct snd_card *card, struct platform_device *pdev,
			  struct snd_redpitaya_ac97 **rredpitaya_ac97)
{
	struct snd_redpitaya_ac97 *redpitaya_ac97;
	int err;
	static struct snd_device_ops ops = {
		.dev_free = snd_redpitaya_ac97_dev_free,
	};
	//const struct of_device_id *of_id;
	struct resource *resource;
	struct resource res_mem_dt;
	struct device_node *np;
	int irq_pos0_dt;
	int irq_pos1_dt;
	int irq;

	PDEBUG(INIT_INFO, "create()\n");

	res_mem_dt.start = 0;
	irq_pos1_dt = irq_pos0_dt = -1;

	*rredpitaya_ac97 = NULL;
	redpitaya_ac97 = kzalloc(sizeof(*redpitaya_ac97), GFP_KERNEL);
	if (redpitaya_ac97 == NULL)
		return -ENOMEM;
	spin_lock_init(&redpitaya_ac97->reg_lock);
	mutex_init(&redpitaya_ac97->cdc_mutex);
	redpitaya_ac97->card = card;
	redpitaya_ac97->pdev = pdev;
	redpitaya_ac97->playback_irq = -1;
	redpitaya_ac97->enable_playback_irq = 0;
	redpitaya_ac97->capture_irq = -1;
	redpitaya_ac97->enable_capture_irq = 0;
	redpitaya_ac97->port = NULL;
	redpitaya_ac97->port_phy = NULL;
	redpitaya_ac97->port_size = 0;

	PDEBUG(INIT_INFO, "Trying to reserve resources now ...\n");

	/* get device tree data */
	//of_id = of_match_device(redpitaya_ac97_dt_ids, &pdev->dev);
	np = of_find_matching_node(NULL, redpitaya_ac97_dt_ids);
	if (np) {
		of_address_to_resource(np, 0, &res_mem_dt);
		irq_pos0_dt = irq_of_parse_and_map(np, 0);
		irq_pos1_dt = irq_of_parse_and_map(np, 1);
		of_node_put(np);
		np = NULL;
	}

	/* get "port" */
	if (res_mem_dt.start) {
		resource = &res_mem_dt;
	} else {
		resource = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	}
	if (!resource) {
		snd_printk(KERN_ERR SND_REDPITAYA_AC97_DRIVER ": can not get memory resource entry");
		return -1;
	}
	{
		// fake entries of the resource struct
		PDEBUG(INIT_INFO, "resource check: resource->start = 0x%08x, resource->end = 0x%08x\n", resource->start, resource->end);
	}

	request_mem_region(resource->start, resource_size(resource), pdev->name);
	redpitaya_ac97->port = ioremap_nocache(resource->start, resource_size(resource));
	redpitaya_ac97->port_phy = (void *)resource->start;
	redpitaya_ac97->port_size = resource_size(resource);
	if (redpitaya_ac97->port == NULL) {
		snd_printk(KERN_ERR SND_REDPITAYA_AC97_DRIVER ": "
			   "unable to remap memory region (%pR)\n",
			   resource);
		snd_redpitaya_ac97_free(redpitaya_ac97);
		return -EBUSY;
	}
	snd_printk(KERN_INFO SND_REDPITAYA_AC97_DRIVER ": "
		   "remap controller memory region from 0x%lx to "
		   "0x%lx with length of %ld bytes done\n",
		   (unsigned long) redpitaya_ac97->port_phy,
		   (unsigned long) redpitaya_ac97->port,
		   (unsigned long) redpitaya_ac97->port_size);

	/* get irq */
        irq = -1;
	if (irq_pos0_dt > 0) {
		irq = irq_pos0_dt;
	} else {
		irq = platform_get_irq(pdev, 0);
	}
	snd_printk(KERN_INFO SND_REDPITAYA_AC97_DRIVER ": "
		   "got play IRQ = %d\n", irq);
	if (request_irq(irq, snd_redpitaya_ac97_irq, 0,
			dev_name(&pdev->dev), (void *)redpitaya_ac97)) {
		snd_printk(KERN_ERR SND_REDPITAYA_AC97_DRIVER ": "
			   "unable to request play IRQ %d\n",
			   irq);
		snd_redpitaya_ac97_free(redpitaya_ac97);
		return -EBUSY;
	}
	redpitaya_ac97->playback_irq = irq;
	snd_printk(KERN_INFO SND_REDPITAYA_AC97_DRIVER ": "
		   "request (playback) irq %d done\n",
		   redpitaya_ac97->playback_irq);
	irq = -1;
	if (irq_pos1_dt > 0) {
		irq = irq_pos1_dt;
	} else {
		irq = platform_get_irq(pdev, 1);
	}
	snd_printk(KERN_INFO SND_REDPITAYA_AC97_DRIVER ": "
		   "got capture IRQ = %d\n", irq);
	if (request_irq(irq, snd_redpitaya_ac97_irq, 0,
			dev_name(&pdev->dev), (void *)redpitaya_ac97)) {
		snd_printk(KERN_ERR SND_REDPITAYA_AC97_DRIVER ": "
			   "unable to request capture IRQ %d\n",
			   irq);
		snd_redpitaya_ac97_free(redpitaya_ac97);
		return -EBUSY;
	}
	redpitaya_ac97->capture_irq = irq;
	snd_printk(KERN_INFO SND_REDPITAYA_AC97_DRIVER ": "
		   "request (capture) irq %d done\n",
		   redpitaya_ac97->capture_irq);

	err = snd_redpitaya_ac97_chip_init(redpitaya_ac97);
	if (err < 0) {
		snd_redpitaya_ac97_free(redpitaya_ac97);
		return err;
	}

	err = snd_device_new(card, SNDRV_DEV_LOWLEVEL, redpitaya_ac97, &ops);
	if (err < 0) {
		PDEBUG(INIT_FAILURE, "probe(): snd_device_new() failed!\n");
		snd_redpitaya_ac97_free(redpitaya_ac97);
		return err;
	}

	*rredpitaya_ac97 = redpitaya_ac97;
	return 0;
}

static void snd_redpitaya_ac97_mixer_free(struct snd_ac97 *ac97)
{
	struct snd_redpitaya_ac97 *redpitaya_ac97 = ac97->private_data;
	PDEBUG(INIT_INFO, "mixer_free()\n");
	redpitaya_ac97->ac97 = NULL;
	PDEBUG(INIT_INFO, "mixer_free() (done)\n");
}

static int
snd_redpitaya_ac97_mixer(struct snd_redpitaya_ac97 *redpitaya_ac97)
{
	struct snd_ac97_bus *bus;
	struct snd_ac97_template ac97;
	int err;
	static struct snd_ac97_bus_ops ops = {
		.write = snd_redpitaya_ac97_codec_write,
		.read = snd_redpitaya_ac97_codec_read,
	};
	PDEBUG(INIT_INFO, "mixer()\n");
	err = snd_ac97_bus(redpitaya_ac97->card, 0, &ops, NULL, &bus);
	if (err < 0)
		return err;

	memset(&ac97, 0, sizeof(ac97));
	redpitaya_ac97->ac97_fake = 1;
	lm4550_regfile_init();
#ifdef CODEC_STAT
	redpitaya_ac97->ac97_read = 0;
	redpitaya_ac97->ac97_write = 0;
#endif
	ac97.private_data = redpitaya_ac97;
	ac97.private_free = snd_redpitaya_ac97_mixer_free;
	ac97.scaps = AC97_SCAP_AUDIO | AC97_SCAP_SKIP_MODEM |
		AC97_SCAP_NO_SPDIF;
	err = snd_ac97_mixer(bus, &ac97, &redpitaya_ac97->ac97);
	redpitaya_ac97->ac97_fake = 0;
	lm4550_regfile_write_values_after_init(redpitaya_ac97->ac97);
	PDEBUG(INIT_INFO, "mixer(): (done) snd_ac97_mixer()=%d\n", err);
	return err;
}

static int
snd_redpitaya_ac97_pcm(struct snd_redpitaya_ac97 *redpitaya_ac97, int device)
{
	struct snd_pcm *pcm;
	int err;

	PDEBUG(INIT_INFO, "pcm()\n");

	err = snd_pcm_new(redpitaya_ac97->card, "RP-AC97/1", device, 1, 1,
			  &pcm);
	if (err < 0)
		return err;
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK,
			&snd_redpitaya_ac97_playback_ops);
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE,
			&snd_redpitaya_ac97_capture_ops);
	pcm->private_data = redpitaya_ac97;
	pcm->info_flags = 0;
	strcpy(pcm->name, "RP-AC97 stereo LineOut/LineIn");
	redpitaya_ac97->pcm = pcm;

	snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_CONTINUOUS,
					  snd_dma_continuous_data(GFP_KERNEL),
					  PERIODS_MAX_B,
					  BUFFER_MAX_B);
	return 0;
}

static int snd_redpitaya_ac97_probe(struct platform_device *pdev)
{
	struct snd_card *card;
	struct snd_redpitaya_ac97 *redpitaya_ac97 = NULL;
	int err;
	int dev = pdev->id;

	PDEBUG(INIT_INFO, "probe(dev=%d)\n", dev);

	if (dev >= SNDRV_CARDS)
		return -ENODEV;
	if (!enable)
		return -ENOENT;

	if ((err = snd_card_new(&pdev->dev, index, id, THIS_MODULE,
				0, &card)) < 0) {
		snd_printk(KERN_ERR SND_REDPITAYA_AC97_DRIVER ": Cannot create card\n");
		return err;
	}
	err = snd_redpitaya_ac97_create(card, pdev, &redpitaya_ac97);
	if (err < 0) {
		PDEBUG(INIT_FAILURE, "probe(): create failed!\n");
		snd_card_free(card);
		return err;
	}
	PDEBUG(INIT_INFO, "probe(): create done\n");
	card->private_data = redpitaya_ac97;
	if ((err = snd_redpitaya_ac97_mixer(redpitaya_ac97)) < 0) {
		snd_card_free(card);
		return err;
	}
	PDEBUG(INIT_INFO, "probe(): mixer done\n");
	if ((err = snd_redpitaya_ac97_pcm(redpitaya_ac97, 0)) < 0) {
		snd_card_free(card);
		return err;
	}
	PDEBUG(INIT_INFO, "probe(): PCM done\n");
	strcpy(card->driver, SND_REDPITAYA_AC97_DRIVER);
	strcpy(card->shortname, "RedPitaya-AC97");
	sprintf(card->longname, "%s %s at 0x%lx, irq %i & %i, device %i",
		card->shortname, card->driver,
		(unsigned long)redpitaya_ac97->port, redpitaya_ac97->playback_irq,
		redpitaya_ac97->capture_irq, dev + 1);

	platform_set_drvdata(pdev, card);

	/* At this point card will be usable */
	if ((err = snd_card_register(card)) < 0) {
		snd_card_free(card);
		return err;
	}
	PDEBUG(INIT_INFO, "probe(): (done)\n");
	return 0;
}

static int snd_redpitaya_ac97_remove(struct platform_device *pdev)
{
	struct snd_card *card;

	PDEBUG(INIT_INFO, "remove()\n");

	if ((card = platform_get_drvdata(pdev)) != NULL)
		snd_card_free(card);
	return 0;
}


/*********************************************************************
 * module init stuff
 *********************************************************************/

// automatic does platform_driver_register()
module_platform_driver(snd_redpitaya_ac97_driver);

