/*
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2012 Daniel Schwierzeck <daniel.schwierzeck@googlemail.com>
 *
 *   PHY MDIO register interface:
 *   Copyright (C) 2013 Ales Bardorfer <ales.bardorfer@redpitaya.com>
 *
 *   LED blinking configuration:
 *   Copyright (C) 2013 Tomaz Beltram <tomaz.beltram@i-tech.si>
 *
 */

#include <linux/module.h>
#include <linux/phy.h>
#include <linux/sysfs.h>

#define MII_MMDCTRL		0x0d
#define MII_MMDDATA		0x0e

#define MII_VR9_11G_IMASK	0x19	/* interrupt mask */
#define MII_VR9_11G_ISTAT	0x1a	/* interrupt status */

#define INT_VR9_11G_WOL		BIT(15)	/* Wake-On-LAN */
#define INT_VR9_11G_ANE		BIT(11)	/* Auto-Neg error */
#define INT_VR9_11G_ANC		BIT(10)	/* Auto-Neg complete */
#define INT_VR9_11G_ADSC	BIT(5)	/* Link auto-downspeed detect */
#define INT_VR9_11G_DXMC	BIT(2)	/* Duplex mode change */
#define INT_VR9_11G_LSPC	BIT(1)	/* Link speed change */
#define INT_VR9_11G_LSTC	BIT(0)	/* Link state change */
#define INT_VR9_11G_MASK	(INT_VR9_11G_LSTC | INT_VR9_11G_ADSC)

#define ADVERTISED_MPD		BIT(10)	/* Multi-port device */

#define MMD_DEVAD		0x1f
#define MMD_ACTYPE_SHIFT	14
#define MMD_ACTYPE_ADDRESS	(0 << MMD_ACTYPE_SHIFT)
#define MMD_ACTYPE_DATA		(1 << MMD_ACTYPE_SHIFT)
#define MMD_ACTYPE_DATA_PI	(2 << MMD_ACTYPE_SHIFT)
#define MMD_ACTYPE_DATA_PIWR	(3 << MMD_ACTYPE_SHIFT)

#define MDIO_ADDR_MAX           0x1f
#define MDIO_VAL_MAX            0xffff

/* MMD LED configuration registers */
#define INT_LED0H 0x1e2
#define INT_LED0L 0x1e3
#define INT_LED1H 0x1e4
#define INT_LED1L 0x1e5

/* LED function bits */
#define INT_LED_TX 0x01
#define INT_LED_RX 0x02
#define INT_LED_LINK10 0x1
#define INT_LED_LINK100 0x2
#define INT_LED_LINK1000 0x4

/* Simple "address-value" interface to PHY MDIO registers */
static ssize_t show_mdio_addr_attr(struct device *dev,
                                   struct device_attribute *devattr,
                                   char *buf);
static ssize_t set_mdio_addr_attr (struct device *dev,
                                   struct device_attribute *attr,
                                   const char *buf, size_t count);
static ssize_t show_mdio_val_attr (struct device *dev,
                                   struct device_attribute *devattr,
                                   char *buf);
static ssize_t set_mdio_val_attr  (struct device *dev,
                                   struct device_attribute *attr,
                                   const char *buf, size_t count);

static DEVICE_ATTR(mdio_addr, S_IWUSR | S_IRUGO,
                   show_mdio_addr_attr, set_mdio_addr_attr);
static DEVICE_ATTR(mdio_val, S_IWUSR | S_IRUGO,
                   show_mdio_val_attr, set_mdio_val_attr);

struct attribute_group attrs;

static struct attribute *phy11g_attrs[] = {
	&dev_attr_mdio_addr.attr,
	&dev_attr_mdio_val.attr,
	NULL
};

// TODO: Make these part of device insted of globals.
unsigned short g_addr;
struct phy_device* g_phydev = 0;

/** 
 * Get MDIO register address of the PHY to act upon by "mdio_val" sysfs entry.
 */
static ssize_t show_mdio_addr_attr(struct device *dev,
                                   struct device_attribute *devattr,
                                   char *buf)
{
        return sprintf(buf, "0x%02x\n", g_addr);
}

/** 
 * Set MDIO register address of the PHY to act upon by "mdio_val" sysfs entry.
 */
static ssize_t set_mdio_addr_attr(struct device *dev,
                                  struct device_attribute *attr,
                                  const char *buf, size_t count)
{
	unsigned long addr;
	int err;

	err = kstrtoul(buf, 0, &addr);
	if (err)
		return err;

	if (addr > MDIO_ADDR_MAX) {
		dev_err(dev,
			"MDIO address 0x%08lx out of range [0x0 - 0x%02x].\n",
                        addr, MDIO_ADDR_MAX);
		return -EINVAL;
	}
        
        g_addr = addr;

	return count;
}

/** 
 * Get value of MDIO register, pointed to by "mdio_addr" sysfs entry.
 */
static ssize_t show_mdio_val_attr(struct device *dev,
                                  struct device_attribute *devattr,
                                  char *buf)
{
    unsigned short val;

    val = phy_read(g_phydev, g_addr);

    return sprintf(buf, "0x%04x\n", val);
}

/** 
 * Set value of MDIO register, pointed to by "mdio_addr" sysfs entry.
 */
static ssize_t set_mdio_val_attr(struct device *dev,
                                 struct device_attribute *attr,
                                 const char *buf, size_t count)
{
	unsigned long val;
	int err;

	err = kstrtoul(buf, 0, &val);
	if (err)
		return err;

	if (val > MDIO_VAL_MAX) {
		dev_err(dev,
			"requested MDIO value 0x%08lx out of range.\n",
                        val);
		return -EINVAL;
	}

        err = phy_write(g_phydev, g_addr, val);
	if (err)
		return err;

	return count;
}

static __maybe_unused int vr9_gphy_mmd_read(struct phy_device *phydev,
						u16 regnum)
{
	phy_write(phydev, MII_MMDCTRL, MMD_ACTYPE_ADDRESS | MMD_DEVAD);
	phy_write(phydev, MII_MMDDATA, regnum);
	phy_write(phydev, MII_MMDCTRL, MMD_ACTYPE_DATA | MMD_DEVAD);

	return phy_read(phydev, MII_MMDDATA);
}

static __maybe_unused int vr9_gphy_mmd_write(struct phy_device *phydev,
						u16 regnum, u16 val)
{
	phy_write(phydev, MII_MMDCTRL, MMD_ACTYPE_ADDRESS | MMD_DEVAD);
	phy_write(phydev, MII_MMDDATA, regnum);
	phy_write(phydev, MII_MMDCTRL, MMD_ACTYPE_DATA | MMD_DEVAD);
	phy_write(phydev, MII_MMDDATA, val);

	return 0;
}

static int vr9_gphy_config_init(struct phy_device *phydev)
{
	int err;

	dev_dbg(&phydev->dev, "%s\n", __func__);

        /* Set LED0 blinking on rx/tx. */
        vr9_gphy_mmd_write(phydev, INT_LED0H, 0);
        vr9_gphy_mmd_write(phydev, INT_LED0L,
                 INT_LED_RX | INT_LED_TX);

        /* Set LED1 binking on link speed: slow=10M, fast=100M, on=1G. */
        vr9_gphy_mmd_write(phydev, INT_LED1H,
                 INT_LED_LINK1000 << 4 | INT_LED_LINK100);
        vr9_gphy_mmd_write(phydev, INT_LED1L,
                 INT_LED_LINK10 << 4);

	/* Mask all interrupts */
	err = phy_write(phydev, MII_VR9_11G_IMASK, 0);
	if (err)
		return err;

	/* Clear all pending interrupts */
	phy_read(phydev, MII_VR9_11G_ISTAT);

        /* Set SGMII RX & TX timing skew to 2 ns & 2.5 ns respectively. */
        /* Set MII power supply to 2V5. */
        err = phy_write(phydev, 0x17, 0x4d00);
	if (err)
		return err;

        /* Disable all 10M modes due to Xilinx EMACPS driver bug - #3120. */
        err = phy_write(phydev, 0x04, 0x0581);
        if (err)
                return err;

        /* Register sysfs interface */
        // TODO: Are there any register/unregister functions available for
        //       PHY drivers? This should really be moved there.
        // TODO: Clanup! But not strictly necessary,
        //       since we do not run as a module.
        if (!g_phydev) {
            attrs.attrs = phy11g_attrs;
            err = sysfs_create_group(&phydev->dev.kobj, &attrs);
            if (err)
                return(err);
            g_phydev = phydev;
        }

	return 0;
}

static int vr9_gphy_config_aneg(struct phy_device *phydev)
{
	int reg, err;

	/* Advertise as multi-port device */
	reg = phy_read(phydev, MII_CTRL1000);
	reg |= ADVERTISED_MPD;
	err = phy_write(phydev, MII_CTRL1000, reg);
	if (err)
		return err;

	return genphy_config_aneg(phydev);
}

static int vr9_gphy_ack_interrupt(struct phy_device *phydev)
{
	int reg;

	/*
	 * Possible IRQ numbers:
	 * - IM3_IRL18 for GPHY0
	 * - IM3_IRL17 for GPHY1
	 *
	 * Due to a silicon bug IRQ lines are not really independent from
	 * each other. Sometimes the two lines are driven at the same time
	 * if only one GPHY core raises the interrupt.
	 */

	reg = phy_read(phydev, MII_VR9_11G_ISTAT);

	return (reg < 0) ? reg : 0;
}

static int vr9_gphy_did_interrupt(struct phy_device *phydev)
{
	int reg;

	reg = phy_read(phydev, MII_VR9_11G_ISTAT);

	return reg > 0;
}

static int vr9_gphy_config_intr(struct phy_device *phydev)
{
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		err = phy_write(phydev, MII_VR9_11G_IMASK, INT_VR9_11G_MASK);
	else
		err = phy_write(phydev, MII_VR9_11G_IMASK, 0);

	return err;
}

static struct phy_driver lantiq_phy[] = {
	{
		.phy_id		= 0xd565a400,
		.phy_id_mask	= 0xfffffffe,
		.name		= "Lantiq XWAY PEF7071",
		.features	= (PHY_GBIT_FEATURES | SUPPORTED_Pause),
		.flags		= PHY_HAS_MAGICANEG, /*PHY_HAS_INTERRUPT,*/
		.config_init	= vr9_gphy_config_init,
		.config_aneg	= vr9_gphy_config_aneg,
		.read_status	= genphy_read_status,
		.ack_interrupt	= vr9_gphy_ack_interrupt,
		.did_interrupt	= vr9_gphy_did_interrupt,
		.config_intr	= vr9_gphy_config_intr,
		.driver		= { .owner = THIS_MODULE },
	}, {
		.phy_id		= 0x030260D0,
		.phy_id_mask	= 0xfffffff0,
		.name		= "Lantiq XWAY VR9 GPHY 11G v1.3",
		.features	= (PHY_GBIT_FEATURES | SUPPORTED_Pause),
		.flags		= 0, /*PHY_HAS_INTERRUPT,*/
		.config_init	= vr9_gphy_config_init,
		.config_aneg	= vr9_gphy_config_aneg,
		.read_status	= genphy_read_status,
		.ack_interrupt	= vr9_gphy_ack_interrupt,
		.did_interrupt	= vr9_gphy_did_interrupt,
		.config_intr	= vr9_gphy_config_intr,
		.driver		= { .owner = THIS_MODULE },
	}, {
		.phy_id		= 0xd565a408,
		.phy_id_mask	= 0xfffffff8,
		.name		= "Lantiq XWAY VR9 GPHY 11G v1.4",
		.features	= (PHY_GBIT_FEATURES | SUPPORTED_Pause),
		.flags		= 0, /*PHY_HAS_INTERRUPT,*/
		.config_init	= vr9_gphy_config_init,
		.config_aneg	= vr9_gphy_config_aneg,
		.read_status	= genphy_read_status,
		.ack_interrupt	= vr9_gphy_ack_interrupt,
		.did_interrupt	= vr9_gphy_did_interrupt,
		.config_intr	= vr9_gphy_config_intr,
		.driver		= { .owner = THIS_MODULE },
	}, {
		.phy_id		= 0xd565a401,
		.phy_id_mask	= 0xffffffff,
		.name		= "Lantiq XWAY VR9 GPHY 11G v1.5",
		.features	= (PHY_GBIT_FEATURES | SUPPORTED_Pause),
		.flags		= 0, /*PHY_HAS_INTERRUPT,*/
		.config_init	= vr9_gphy_config_init,
		.config_aneg	= vr9_gphy_config_aneg,
		.read_status	= genphy_read_status,
		.ack_interrupt	= vr9_gphy_ack_interrupt,
		.did_interrupt	= vr9_gphy_did_interrupt,
		.config_intr	= vr9_gphy_config_intr,
		.driver		= { .owner = THIS_MODULE },
	}, {
		.phy_id		= 0xd565a418,
		.phy_id_mask	= 0xfffffff8,
		.name		= "Lantiq XWAY XRX PHY22F v1.4",
		.features	= (PHY_BASIC_FEATURES | SUPPORTED_Pause),
		.flags		= 0, /*PHY_HAS_INTERRUPT,*/
		.config_init	= vr9_gphy_config_init,
		.config_aneg	= vr9_gphy_config_aneg,
		.read_status	= genphy_read_status,
		.ack_interrupt	= vr9_gphy_ack_interrupt,
		.did_interrupt	= vr9_gphy_did_interrupt,
		.config_intr	= vr9_gphy_config_intr,
		.driver		= { .owner = THIS_MODULE },
	},
};

static int __init ltq_phy_init(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(lantiq_phy); i++) {
		int err = phy_driver_register(&lantiq_phy[i]);
		if (err)
			pr_err("lantiq_phy: failed to load %s\n", lantiq_phy[i].name);
	}

	return 0;
}

static void __exit ltq_phy_exit(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(lantiq_phy); i++)
		phy_driver_unregister(&lantiq_phy[i]);
}

module_init(ltq_phy_init);
module_exit(ltq_phy_exit);

MODULE_DESCRIPTION("Lantiq PHY drivers");
MODULE_AUTHOR("Daniel Schwierzeck <daniel.schwierzeck@googlemail.com>");
MODULE_LICENSE("GPL");
