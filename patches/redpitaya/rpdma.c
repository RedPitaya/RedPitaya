/*
 * Red Pitaya 2016
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
#include <linux/amba/xilinx_dma.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/dma-direction.h>
#include "rpdma.h"


struct platform_device *rpdev;
static dev_t dev_num;
static struct cdev c_dev;
static struct class *cl;
struct resource *res;
enum dma_status status;


struct rpdma_channel{
    dma_addr_t handle;
    dma_addr_t rpdma_handle;
    struct dma_chan *chan;
    dev_t dev_num;
    struct cdev c_dev;
    struct class *cl;
    enum dma_status status;
    struct platform_device *rpdev;
    struct completion cmp;
    struct dma_device *dev;
    struct dma_async_tx_descriptor *d;
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
    unsigned long tmo;
}rx, tx;


// write shall wrte users buffer to dma memory, so that loopback fpga code can write that data to dma buffer 
static ssize_t rpdma_write(struct file *f, const char __user * buf,size_t len, loff_t * off){
    int j;
    //unsigned char c=0;
    printk("rpdma: write()\n");
    for (j=0; j <  tx.segment_size*tx.segment_cnt; j++) {
             tx.addrv[j]=(unsigned char)j%256;
    }
    return 0;//(ssize_t)copy_from_user(tx.addrv, buf, len);
}

//callback
static void rpdma_slave_tx_callback(void *completion)
{   
    printk("rpdma:tx complete\n");
    complete(completion);
}

//callback
static void rpdma_slave_rx_callback(void *completion)
{    
    printk("rpdma:rx complete\n");
    complete(completion);
    rx.flag=1;
    wake_up_interruptible(&rx.wq);
}


int rpdma_open(struct inode * i, struct file * f) { 
    printk("rpdma:open\n");
    return 0;
}

//function blocks its user until rpdma_slave_rx_callback is called by dma engine
int rpdma_read(struct file *filep, char *buff, size_t len, loff_t *off){
    
    printk("rpdma:read wait flag:%d\n",rx.flag);
    wait_event_interruptible(rx.wq, rx.flag != 0); 
    rx.flag = 0;
    printk("rpdma: read go\n");
    return len;
}

//function to start and stop dma transfers
static long rpdma_ioctl(struct file *file, unsigned int cmd , unsigned long arg) {

        rx.tmo = msecs_to_jiffies(3000000); 
        tx.tmo = msecs_to_jiffies(2000000);
        smp_rmb();
        printk("rpdma:ioctl cmd:%d arg%d\n",cmd,(int)arg);
    switch(cmd){
    case STOP_TX:{
        printk("rpdma:ioctl terminate all tx\n");
        dmaengine_terminate_all(tx.chan); 
      }break;  
    case STOP_RX:{
        printk("rpdma:ioctl terminate all rx\n");
        dmaengine_terminate_all(rx.chan); 
        rx.flag = 1;
        wake_up_interruptible(&rx.wq);
   } break;
    case CYCLIC_TX:{    
        printk("rpdma:ioctl cyclic tx s:%d c:%d\n", tx.segment_size,tx.segment_cnt );smp_rmb();
        tx.d = tx.chan->device->device_prep_dma_cyclic(tx.chan, tx.addrp, tx.segment_size*tx.segment_cnt, tx.segment_size, DMA_MEM_TO_DEV, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
        if(!tx.d){printk("rpdma: txd not set properly\n");}
        else{
            init_completion(&tx.cmp);
            tx.d->callback = rpdma_slave_tx_callback; //set up completition callback
            tx.d->callback_param = &tx.cmp;
            tx.cookie = tx.d->tx_submit(tx.d);    
            printk("rpdma: tx submit\n"); 
            if (dma_submit_error(tx.cookie)) {
                printk("rpdma tx submit error %d \n", tx.cookie);
            }
            dma_async_issue_pending(tx.chan);
        }
      } break;
 
    case CYCLIC_RX://rx prepair
    {   
            printk("rpdma:ioctl cyclic rx s:%d c:%d\n",rx.segment_size,rx.segment_cnt);
smp_rmb();
        rx.d = rx.dev->device_prep_dma_cyclic(rx.chan,rx.addrp, rx.segment_size*rx.segment_cnt, rx.segment_size,DMA_DEV_TO_MEM, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
        if(!rx.d){printk("rpdma:rxd not set properly\n");}
        else{
        init_completion(&rx.cmp);
        rx.d->callback = rpdma_slave_rx_callback; //set completion callback
        rx.d->callback_param = &rx.cmp;
        rx.cookie = rx.d->tx_submit(rx.d);    
        printk("rpdma:rx_submit\n");
            
        if(dma_submit_error(rx.cookie)){
            printk("rpdma rx submit error %d \n", rx.cookie);
        }
        rx.flag=0;
        dma_async_issue_pending(rx.chan);

    }
    }break;
   case SINGLE_RX:{
        printk("rpdma:rx single dma s:%d c:%d\n",rx.segment_size,rx.segment_cnt);smp_rmb();
        
        rx.d = dmaengine_prep_slave_single(rx.chan,rx.addrp, rx.segment_size*rx.segment_cnt, DMA_DEV_TO_MEM, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
        if(!rx.d){printk("rpdma:rxd not set properly\n");}
        else{
            init_completion(&rx.cmp);
            rx.d->callback = rpdma_slave_rx_callback;
            rx.d->callback_param = &rx.cmp;
            rx.cookie = rx.d->tx_submit(rx.d);
            printk("rpdma:rx submit \n");
        }

        printk("rpdma:rx dma_async_issue_pending \n");
        dma_async_issue_pending(rx.chan);    

        
            
    break;
    }
    case SINGLE_TX:{
        printk("rpdma:tx single dma s:%d c:%d\n", tx.segment_size,tx.segment_cnt );
        smp_rmb();
        
        tx.d = dmaengine_prep_slave_single(tx.chan, tx.addrp, tx.segment_size*tx.segment_cnt, DMA_MEM_TO_DEV, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
        if(!tx.d){printk("rpdma: txd not set properly\n");}
        else{
            init_completion(&tx.cmp);
            tx.d->callback = rpdma_slave_tx_callback;
            tx.d->callback_param = &tx.cmp;
            tx.cookie = tx.d->tx_submit(tx.d);
            printk("rpdma:tx submit \n");
        }
        
        printk("rpdma:tx dma_async_issue_pending \n");
        dma_async_issue_pending(tx.chan);

        
    break;
    }
    
        case SIMPLE_RX:{
        printk("rpdma:rx single dma s:%d c:%d\n",rx.segment_size,rx.segment_cnt);smp_rmb();
        
        rx.d = dmaengine_prep_slave_single(rx.chan,rx.addrp, rx.segment_size*rx.segment_cnt, DMA_DEV_TO_MEM, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
        if(!rx.d){printk("rpdma:rxd not set properly\n");}
        else{
            init_completion(&rx.cmp);
            rx.d->callback = rpdma_slave_rx_callback;
            rx.d->callback_param = &rx.cmp;
            rx.cookie = rx.d->tx_submit(rx.d);
            printk("rpdma:rx submit \n");
        }
        

        printk("rpdma:rx dma_async_issue_pending \n");
        dma_async_issue_pending(rx.chan);    
        rx.tmo = wait_for_completion_timeout(&rx.cmp, rx.tmo);
            
    break;
    }
    case SIMPLE_TX:{
        printk("rpdma:tx single dma s:%d c:%d\n", tx.segment_size,tx.segment_cnt );smp_rmb();
        tx.d = dmaengine_prep_slave_single(tx.chan, tx.addrp, tx.segment_size*tx.segment_cnt, DMA_MEM_TO_DEV, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
        if(!tx.d){printk("rpdma: txd not set properly\n");}
        else{
            init_completion(&tx.cmp);
            tx.d->callback = rpdma_slave_tx_callback;
            tx.d->callback_param = &tx.cmp;
            tx.cookie = tx.d->tx_submit(tx.d);
            printk("rpdma:tx submit \n");
        }
        
        printk("rpdma:tx dma_async_issue_pending \n");
        dma_async_issue_pending(tx.chan);
        
        tx.tmo = wait_for_completion_timeout(&tx.cmp, tx.tmo);
        dmaengine_terminate_all(tx.chan);

    break;
    }
    case SIMPLE:{
        printk("rpdma:tx single dma \n");
        smp_rmb();
        tx.d = dmaengine_prep_slave_single(tx.chan, tx.addrp, tx.segment_size*tx.segment_cnt, DMA_MEM_TO_DEV, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
        if(!tx.d){printk("rpdma: txd not set properly\n");}
        else{
            init_completion(&tx.cmp);
            tx.d->callback = rpdma_slave_tx_callback;
            tx.d->callback_param = &tx.cmp;
            tx.cookie = tx.d->tx_submit(tx.d);
            printk("rpdma:tx submit \n");
        }
        
        printk("rpdma:rx single dma \n");
        
        rx.d = dmaengine_prep_slave_single(rx.chan,rx.addrp, rx.segment_size*rx.segment_cnt, DMA_DEV_TO_MEM, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
        if(!rx.d){printk("rpdma:rxd not set properly\n");}
        else{
            init_completion(&rx.cmp);
            rx.d->callback = rpdma_slave_rx_callback;
            rx.d->callback_param = &rx.cmp;
            rx.cookie = rx.d->tx_submit(rx.d);
            printk("rpdma:rx submit \n");
        }
        
        printk("rpdma:rx dma_async_issue_pending \n");
        dma_async_issue_pending(rx.chan);  
        printk("rpdma:tx dma_async_issue_pending \n");
        dma_async_issue_pending(tx.chan);
  
        tx.tmo = wait_for_completion_timeout(&tx.cmp, tx.tmo);
        
        
        dmaengine_terminate_all(tx.chan);
        dmaengine_terminate_all(rx.chan);

    break;
    }
    
    case SET_TX_SGMNT_CNT:{
        printk("rpdma:ioctl tx segment cnt set to %lx \n",arg);
        tx.segment_cnt=arg;
    }break;  
    case SET_TX_SGMNT_SIZE :{
        printk("rpdma:ioctl tx segment size set to %lx \n",arg);
        tx.segment_size=arg;
   }break;
    case SET_RX_SGMNT_CNT:{
        printk("rpdma:ioctl rx segment cnt set to %lx \n",arg);
        rx.segment_cnt=arg;
   }break;
    case SET_RX_SGMNT_SIZE:{
        printk("rpdma:ioctl rx segment size set to %lx \n",arg);
        rx.segment_size=arg;
    }break;  
              
    }
    return 0;
}

//dealocate everything for dma
static int rpdma_release(struct inode *ino, struct file *file)
{
    //  dmaengine_terminate_all(tx.chan); 
   //     dmaengine_terminate_all(rx.chan); 
    //if(tx.segment)
    //    dma_unmap_single(tx.dev->dev, tx.segment, tx.segment_size*tx.segment_cnt, DMA_TO_DEVICE);
    //        if(rx.segment)
//        dma_unmap_single(rx.dev->dev, rx.segment, rx.segment_size*rx.segment_cnt, DMA_FROM_DEVICE);
    //rx.flag=1;
    //wake_up_interruptible(&rx.wq);
    printk("rpdma:release\n");
    return 0;
}


//mmaps receiver buffer to userspace
static int rpdma_mmap(struct file * f, struct vm_area_struct * v){
    printk("rpdma:mmap\n");
    return dma_common_mmap(&rpdev->dev, v, rx.addrv, rx.addrp,v->vm_end - v->vm_start);    
}

static struct file_operations fops = {
    .owner    = THIS_MODULE,
    .read = rpdma_read, //it shall inform user of data availibility using blocking read
    .open     = rpdma_open, 
    .release  = rpdma_release, //dealocation of dma buffers
    .unlocked_ioctl = rpdma_ioctl, //start stop functionality per channel
    .mmap    = rpdma_mmap, //enables zerro copy from device to user
    .write = rpdma_write,
};

//dma channels allocation
//character device registration
static int rpdma_probe(struct platform_device *pdev)
{
    int err;
    printk("rpdma:probe\n");
    rpdev=pdev;
    tx.chan = dma_request_slave_channel(&pdev->dev, "axidma0");
    if (IS_ERR(tx.chan)) {
        pr_err("rpdma: No Tx channel\n");
        return PTR_ERR(tx.chan);
    }

    rx.chan = dma_request_slave_channel(&pdev->dev, "axidma1");
    if (IS_ERR(rx.chan)) {
        err = PTR_ERR(rx.chan);
        pr_err("rpdma: No Rx channel\n");
        goto free_tx;
    }
    
    if (alloc_chrdev_region(&dev_num, 0, 1, "rpdma") < 0) {
        return -1;
    }
    if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL) {
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }
    if (device_create(cl, NULL, dev_num, NULL, "rpdma") == NULL) {
        class_destroy(cl);
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }
    printk("rpdma:probe,cdevinit\n");
    cdev_init(&c_dev, &fops);
    if (cdev_add(&c_dev, dev_num, 1) == -1) {
        device_destroy(cl, dev_num);
        class_destroy(cl);
        unregister_chrdev_region(dev_num, 1);
        goto rmdev;
    }
    
    tx.dev = tx.chan->device;
    rx.dev = rx.chan->device;
    
    tx.addrv =  dma_alloc_coherent(tx.dev->dev, TX_SGMNT_CNT*TX_SGMNT_SIZE, &tx.addrp, GFP_ATOMIC);
    if(tx.addrv==NULL){
        printk("rpdma: ERROR tx not allocated\n");
    }else printk("rpdma: reserved %ld for tx at %x\n",TX_SGMNT_CNT*TX_SGMNT_SIZE,tx.addrp);
    rx.addrv=  dma_alloc_coherent(rx.dev->dev, RX_SGMNT_CNT*RX_SGMNT_SIZE, &rx.addrp, GFP_ATOMIC);
    if(rx.addrv==NULL){
    printk("rpdma: ERROR rx not allocated\n");
    }else {
		printk("rpdma: reserved %ld for rx at %x\n",RX_SGMNT_CNT*RX_SGMNT_SIZE,rx.addrp);
	}
        
    tx.segment_cnt=TX_SGMNT_CNT;
    tx.segment_size=TX_SGMNT_SIZE;
    rx.segment_cnt=RX_SGMNT_CNT;
    rx.segment_size=RX_SGMNT_SIZE;
    rx.flag=0;
    init_waitqueue_head(&rx.wq);
    return 0;
    
rmdev:
    device_destroy(cl, dev_num);
    class_destroy(cl);
    unregister_chrdev_region(dev_num, 1);
    dma_release_channel(rx.chan);
free_tx:
    dma_release_channel(tx.chan);

    return err;
}

//character device dealocation
//dma channel dealocation
//cma buffer dealocation
static int rpdma_remove(struct platform_device *pdev)
{
    printk("rpdma:remove\n");
    if(rx.chan){
        dma_release_channel(rx.chan);
    }
    if(tx.chan){
        dma_release_channel(tx.chan);
    }
    
    cdev_del(&c_dev);
    device_destroy(cl,dev_num);
    class_destroy(cl);
    unregister_chrdev_region(dev_num,1);
    
    //if dma memmory is still allocated return it to cma memory allocator pool
    if(rx.addrv){
       dma_free_coherent(NULL, RX_SGMNT_CNT*RX_SGMNT_SIZE, rx.addrv, rx.addrp);
    }
    
    if(tx.addrv){
       dma_free_coherent(NULL,TX_SGMNT_CNT*TX_SGMNT_SIZE, tx.addrv, tx.addrp);
    }
    return 0;
}


static const struct of_device_id rpdma_of_ids[] = {
    { .compatible = "redpitaya,rpdma",},
    {}
};
MODULE_DEVICE_TABLE(of, rpdma_of_ids);

static struct platform_driver rpdma_platform_driver = {
    .driver = {
        .name = "rpdma",
        .owner = THIS_MODULE,
        .of_match_table = rpdma_of_ids,
    },
    .probe = rpdma_probe,
    .remove = rpdma_remove,
};

module_platform_driver(rpdma_platform_driver);

static void __exit rpdma_exit(void)
{
    printk("rpdma:exit\n");
    platform_driver_unregister(&rpdma_platform_driver);
}
module_exit(rpdma_exit)

MODULE_AUTHOR("Red Pitaya");
MODULE_DESCRIPTION("Red Pitaya DMA Client Driver");
MODULE_LICENSE("GPL v2");
