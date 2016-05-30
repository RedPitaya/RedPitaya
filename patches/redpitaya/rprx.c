/*
 * Red Pitaya 2016
 * dma client diver
 * Based on XILINX AXI DMA test
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/dmaengine.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/kdev_t.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/of_dma.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/dma/xilinx_dma.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/dma-direction.h>
#include <linux/of_address.h>
#include "rpdma.h"

struct rprx_channel{
	dma_addr_t handle;
	dma_addr_t rprx_handle;
	struct dma_chan *chan;
	unsigned char dmastatus;
	enum dma_status status;
	struct completion cmp;
	struct dma_device *dev;
	struct dma_async_tx_descriptor *d;
	struct platform_device *rpdev;
	dev_t dev_num;
	struct cdev c_dev;
	struct class *cl;
	struct resource *res;
	struct device_node *memory;
	dma_cookie_t cookie;
	int segment_cnt;
	long segment_size;
	unsigned char* addrv;
	dma_addr_t addrp;
	dma_addr_t segment;
	int flag;
	int flags;
	enum dma_data_direction direction;
	wait_queue_head_t wq;
	u64 *memory_size;
};

static ssize_t rprx_write(struct file *f, const char __user * buf,size_t len, loff_t * off){
	return 0;
}

/*
 * calback set at start of dma transfer
 * it signals wake up for unblocking user
 * */
static void rprx_slave_callback(void *completion)
{
	struct rprx_channel *rx;

	rx = (struct rprx_channel *)completion;
	dev_info((const struct device *)&rx->rpdev->dev, "complete\n");
	//complete(completion);
	rx->flag=1;
	wake_up_interruptible(&rx->wq);
}

int rprx_open(struct inode * i, struct file * f)
{
	struct rprx_channel *rx;
	rx = container_of(i->i_cdev, struct rprx_channel ,c_dev );
	f->private_data = (void*)rx;
	dev_info((const struct device *)&rx->rpdev->dev, "open\n");
	return 0;
}

/*
 * function blocks its user until rprx_slave_callback is called by dma engine
 */
int rprx_read(struct file *filep, char *buff, size_t len, loff_t *off)
{
	struct rprx_channel *rx = (struct rprx_channel *)filep->private_data;
	dev_info((const struct device *)&rx->rpdev->dev, "read wait flag:%d\n",rx->flag);
	wait_event_interruptible(rx->wq, rx->flag != 0);
	rx->flag = 0;
	dev_info((const struct device *)&rx->rpdev->dev, "read go\n");
	return len;
}

/*
 * function to start and stop dma transfers
 */
static long rprx_ioctl(struct file *file, unsigned int cmd , unsigned long arg)
{
	struct rprx_channel *rx = (struct rprx_channel *)file->private_data;
	const struct device * dev=(const struct device *)&rx->rpdev->dev;
	smp_rmb();
	dev_info(dev, "ioctl cmd:%d arg%d\n",cmd,(int)arg);
	switch (cmd){
	case STOP_RX:{
		dev_info(dev,"ioctl terminate all rx\n");
		dmaengine_terminate_all(rx->chan);
		rx->flag = 1;
		wake_up_interruptible(&rx->wq);
		rx->dmastatus=STATUS_STOPPED;
	}break;
	case CYCLIC_RX:
	{
		dev_info(dev, "ioctl cyclic rx s:0x%lx c:0x%x\n",rx->segment_size,rx->segment_cnt);
		smp_rmb();
		rx->dmastatus=STATUS_BUSSY;
		rx->d = rx->dev->device_prep_dma_cyclic(rx->chan,rx->addrp, rx->segment_size*rx->segment_cnt, rx->segment_size,DMA_DEV_TO_MEM, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
		if(!rx->d){
			dev_err(dev, "rxd not set properly\n");
			rx->dmastatus=STATUS_ERROR;
		}
		else{
			init_completion(&rx->cmp);
			rx->d->callback = rprx_slave_callback;
			rx->d->callback_param = rx;
			rx->cookie = rx->d->tx_submit(rx->d);
			dev_info(dev, "submit\n");
			if(dma_submit_error(rx->cookie)){
				dev_err(dev, "submit error %d \n", rx->cookie);
				rx->flag=0;
				dma_async_issue_pending(rx->chan);
				rx->dmastatus=STATUS_ERROR;
			}
			else{
				rx->flag=0;
				dma_async_issue_pending(rx->chan);
				rx->dmastatus=STATUS_READY;
			}
		}
	}break;
	case SINGLE_RX:{
		dev_info(dev, "single dma s:0x%lx c:0x%x\n",rx->segment_size,rx->segment_cnt);smp_rmb();
		rx->d = dmaengine_prep_slave_single(rx->chan,rx->addrp, rx->segment_size*rx->segment_cnt, DMA_DEV_TO_MEM, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
		if(!rx->d){
			dev_err(dev, "rxd not set properly\n");
			rx->dmastatus=STATUS_ERROR;
		}else{
			init_completion(&rx->cmp);
			rx->d->callback = rprx_slave_callback;
			rx->d->callback_param = &rx;
			rx->cookie = rx->d->tx_submit(rx->d);
			dev_info(dev, "submit \n");
		}
		dev_info(dev, "dma_async_issue_pending \n");
		dma_async_issue_pending(rx->chan);
		rx->dmastatus=STATUS_READY;
	break;
	}
	case SET_RX_SGMNT_CNT:{
		rx->segment_cnt=arg;
		dev_info(dev, "ioctl segment cnt set to 0x%x \n",rx->segment_cnt);
	}break;
	case SET_RX_SGMNT_SIZE:{
		rx->segment_size=arg;
		dev_info(dev, "ioctl segment size set to 0x%lx \n",rx->segment_size);
	}break;
	case STATUS:{
		put_user(rx->dmastatus,(char*)arg);
	}break;
	default:
		dev_info(dev, "ioctl %d-%lx not inmplemented \n",cmd,arg);
	}return 0;
}

static int rprx_release(struct inode *ino, struct file *file)
{
	struct rprx_channel *rx = (struct rprx_channel *)file->private_data;
	const struct device * dev =(const struct device *)&rx->rpdev->dev;
	dev_info(dev, "release\n");
	return 0;
}


/*
 * call dma_common_mmap to enable user to use mmap dma reserved memory
 * */
static int rprx_mmap(struct file * f, struct vm_area_struct * v)
{
	struct rprx_channel *rx = (struct rprx_channel *)f->private_data;
	const struct device * dev =(const struct device *)&rx->rpdev->dev;
	dev_info(dev, "mmap\n");
	return dma_common_mmap(&rx->rpdev->dev, v, rx->addrv, rx->addrp,v->vm_end - v->vm_start);
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = rprx_read,
	.open = rprx_open,
	.release = rprx_release,
	.unlocked_ioctl = rprx_ioctl,
	.mmap = rprx_mmap,
	.write = rprx_write,
};

/*
 * allocate device data structure
 * request for dma channels
 * create character device
 * get buffer location from device tree
 * */
static int rprx_probe(struct platform_device *pd)
{
	int err;

	struct rprx_channel *rx ;
	const struct device * dev =(const struct device *)&pd->dev;
	rx = devm_kzalloc( &pd->dev, sizeof(struct rprx_channel), GFP_KERNEL );
	if (!rx){
		dev_err(dev, "no memory for device structrure\n");
		return -ENOMEM;
	}
	rx->rpdev=pd;

	platform_set_drvdata( rx->rpdev, rx );

	rx->chan = dma_request_slave_channel(&rx->rpdev->dev, "axidma1");
	if (IS_ERR(rx->chan)) {
		err = PTR_ERR(rx->chan);
		dev_err(dev,"No DMA channel\n");
		goto rmdev;
	}

	if (alloc_chrdev_region(&rx->dev_num, 0, 1, "rprx") < 0) {
		return -1;
	}
	if ((rx->cl = class_create(THIS_MODULE, "chardrv")) == NULL) {
		unregister_chrdev_region(rx->dev_num, 1);
		return -1;
	}
	if (device_create(rx->cl, NULL, rx->dev_num, NULL, "rprx") == NULL) {
		class_destroy(rx->cl);
		unregister_chrdev_region(rx->dev_num, 1);
		return -1;
	}

	cdev_init(&rx->c_dev, &fops);
	if (cdev_add(&rx->c_dev, rx->dev_num, 1) == -1) {
		device_destroy(rx->cl, rx->dev_num);
		class_destroy(rx->cl);
		unregister_chrdev_region(rx->dev_num, 1);
		goto rmdev;
	}

	rx->dev = rx->chan->device;

	rx->memory = of_parse_phandle(rx->rpdev->dev.of_node, "memory-region", 0);
	if (!rx->memory) {
		return -ENODEV;
	}

	rx->addrp = of_translate_address(rx->memory,of_get_address(rx->memory, 0, rx->memory_size, NULL));

	rx->addrv = phys_to_virt(rx->addrp);

	if (rx->addrv==NULL){
		dev_err(dev, "DMA reserved memory not allocated\n");
	goto rmdev;
	}else {
		dev_info(dev, "reserved dma: %p and 0x%x KiB @0x%x\n",(void*)rx->chan,(RX_SGMNT_CNT*RX_SGMNT_SIZE)/1024,rx->addrp);
	}

	rx->segment_cnt=RX_SGMNT_CNT;
	rx->segment_size=RX_SGMNT_SIZE;
	rx->flag=0;
	init_waitqueue_head(&rx->wq);
	return 0;

rmdev:
	device_destroy(rx->cl, rx->dev_num);
	class_destroy(rx->cl);
	unregister_chrdev_region(rx->dev_num, 1);
	dma_release_channel(rx->chan);

	return err;
}

static int rprx_remove(struct platform_device *pdev)
{
	struct rprx_channel *rx = (struct rprx_channel *)platform_get_drvdata(pdev);
	const struct device * dev =(const struct device *)&rx->rpdev->dev;
	dev_info(dev, "remove\n");
	if(rx->chan){
		dma_release_channel(rx->chan);
	}

	cdev_del(&rx->c_dev);
	device_destroy(rx->cl,rx->dev_num);
	class_destroy(rx->cl);
	unregister_chrdev_region(rx->dev_num,1);

	return 0;
}


static const struct of_device_id rprx_of_ids[] = {
	{ .compatible = "redpitaya,rprx",},
	{}
};
MODULE_DEVICE_TABLE(of, rprx_of_ids);

static struct platform_driver rprx_platform_driver = {
	.driver = {
		.name = "rprx",
		.owner = THIS_MODULE,
		.of_match_table = rprx_of_ids,
	},
	.probe = rprx_probe,
	.remove = rprx_remove,
};

module_platform_driver(rprx_platform_driver);

static void __exit rprx_exit(void)
{
	printk(KERN_INFO "exit\n");
	platform_driver_unregister(&rprx_platform_driver);
}
module_exit(rprx_exit)

MODULE_AUTHOR("Red Pitaya");
MODULE_DESCRIPTION("Red Pitaya DMA Client Driver");
MODULE_LICENSE("GPL v2");
