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
#include "rpdma.h"

struct dma_chan *tx_chan, *rx_chan;
static dev_t dev_num;
static struct cdev c_dev;
static struct class *cl;
struct resource *res;
enum dma_status status;

struct platform_device *rpdev;

struct completion rx_cmp;
struct completion tx_cmp;
/*
#define PATTERN_SRC        0x80
#define PATTERN_DST        0x00
#define PATTERN_COPY        0x40
#define PATTERN_OVERWRITE    0x20
#define PATTERN_COUNT_MASK    0x1f
*/


dma_addr_t rpdma_handle;

struct dma_device *tx_dev;
struct dma_device *rx_dev;
    
struct dma_async_tx_descriptor *txd = NULL;
struct dma_async_tx_descriptor *rxd = NULL;
    
dma_cookie_t rx_cookie, tx_cookie;

//number of segments

int rx_segment_cnt=RX_SGMNT_CNT;
long rx_segment_size = RX_SGMNT_SIZE;
int tx_segment_cnt=TX_SGMNT_CNT;
long tx_segment_size = TX_SGMNT_SIZE;

unsigned char* rpdma_rx_addrv;
unsigned char* rpdma_tx_addrv;

dma_addr_t rpdma_rx_addrp;
dma_addr_t rpdma_tx_addrp;

dma_addr_t dma_rx_segment;
dma_addr_t dma_tx_segment;

int sync=1;


//for blocking read
static int flag = 0;
static DECLARE_WAIT_QUEUE_HEAD(wq);

// write shall wrte users buffer to dma memory, so that loopback fpga code can write that data to dma buffer 
static ssize_t rpdma_write(struct file *f, const char __user * buf,size_t len, loff_t * off){
    int j;
    //unsigned char c=0;
    printk("rpdma: write()\n");
    for (j=0; j <  tx_segment_size*tx_segment_cnt; j++) {
             rpdma_tx_addrv[j]=(unsigned char)j%255;
    }
    return 0;//(ssize_t)copy_from_user(rpdma_tx_addrv, buf, len);
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
    flag=1;
    wake_up_interruptible(&wq);
}


int rpdma_open(struct inode * i, struct file * f) { 
    printk("rpdma:open\n");
    return 0;
}

//function blocks its user until rpdma_slave_rx_callback is called by dma engine
int rpdma_read(struct file *filep, char *buff, size_t len, loff_t *off){
    
    printk("rpdma:read wait\n");
    wait_event_interruptible(wq, flag != 0); 
    flag = 0;
    printk("rpdma: read go\n");
    return len;
}

//function to start and stop dma transfers
static long rpdma_ioctl(struct file *file, unsigned int cmd , unsigned long arg) {

        unsigned long rx_tmo = msecs_to_jiffies(3000000); 
        unsigned long tx_tmo = msecs_to_jiffies(2000000);
        smp_rmb();
        printk("rpdma:ioctl cmd:%d arg%d\n",cmd,(int)arg);
    switch(cmd){
    case STOP_TX:{
        printk("rpdma:ioctl terminate all tx\n");
        dmaengine_terminate_all(tx_chan); 
        if(dma_tx_segment)
        dma_unmap_single(tx_dev->dev, dma_tx_segment, tx_segment_size*tx_segment_cnt, DMA_FROM_DEVICE);
      }break;  
    case STOP_RX:{
        printk("rpdma:ioctl terminate all rx\n");
        dmaengine_terminate_all(rx_chan); 
        if(dma_rx_segment)
        dma_unmap_single(rx_dev->dev, dma_rx_segment, rx_segment_size*rx_segment_cnt, DMA_FROM_DEVICE);
   } break;
    case CYCLIC_TX:{    
            printk("rpdma:ioctl cyclic tx\n");
            txd = tx_chan->device->device_prep_dma_cyclic(tx_chan, rpdma_tx_addrp, tx_segment_size*tx_segment_cnt, tx_segment_size, DMA_TO_DEVICE, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
            if(!txd){printk("rpdma: txd not set properly\n");}
            else{
            init_completion(&tx_cmp);
            txd->callback = rpdma_slave_tx_callback; //set up completition callback
            txd->callback_param = &tx_cmp;
            tx_cookie = txd->tx_submit(txd);    
            printk("rpdma: tx submit\n"); 
            if (dma_submit_error(tx_cookie)) {
                printk("rpdma tx submit error %d \n", tx_cookie);
            }
            dma_async_issue_pending(tx_chan);
        }
      } break;
 
    case CYCLIC_RX://rx prepair
    {   
            printk("rpdma:ioctl cyclic rx\n");

            rxd = rx_dev->device_prep_dma_cyclic(rx_chan,rpdma_rx_addrp, rx_segment_size*rx_segment_cnt, rx_segment_size,DMA_DEV_TO_MEM, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
            if(!rxd){printk("rpdma:rxd not set properly\n");}
            else{
            init_completion(&rx_cmp);
            rxd->callback = rpdma_slave_rx_callback; //set completion callback
            rxd->callback_param = &rx_cmp;
            rx_cookie = rxd->tx_submit(rxd);    
            printk("rpdma:rx_submit\n");
            
        if(dma_submit_error(rx_cookie)){
            printk("rpdma rx submit error %d \n", rx_cookie);
        }
        dma_async_issue_pending(rx_chan);
        flag = 0;
    }
    }break;
    case SINGLE_RX:{
        printk("rpdma:rx single dma \n");
        dma_rx_segment = dma_map_single(rx_dev->dev, rpdma_rx_addrv, rx_segment_size*rx_segment_cnt, DMA_FROM_DEVICE);
        if(dma_mapping_error(tx_dev->dev,dma_rx_segment)){printk("rpdma:dma_rx_segment not set properly\n");}
        else{
        rxd = dmaengine_prep_slave_single(rx_chan,rpdma_rx_addrp, rx_segment_size*rx_segment_cnt, DMA_DEV_TO_MEM, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
            if(!rxd){printk("rpdma:rxd not set properly\n");}
            else{
                init_completion(&rx_cmp);
                rxd->callback = rpdma_slave_rx_callback;
                rxd->callback_param = &rx_cmp;
                rx_cookie = rxd->tx_submit(rxd);
                printk("rpdma:rx submit \n");
            }
        }

        printk("rpdma:rx dma_async_issue_pending \n");
        dma_async_issue_pending(rx_chan);    

        
            
    break;
    }
    case SINGLE_TX:{
        printk("rpdma:tx single dma \n");
        dma_tx_segment = dma_map_single(tx_dev->dev, rpdma_tx_addrv, tx_segment_size*tx_segment_cnt,DMA_TO_DEVICE);
        if(dma_mapping_error(tx_dev->dev,dma_tx_segment)){printk("rpdma:dma_tx_segment not set properly\n");}
        else{
            txd = dmaengine_prep_slave_single(tx_chan, dma_tx_segment, tx_segment_size*tx_segment_cnt, DMA_MEM_TO_DEV, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
            if(!txd){printk("rpdma: txd not set properly\n");}
            else{
                init_completion(&tx_cmp);
                txd->callback = rpdma_slave_tx_callback;
                txd->callback_param = &tx_cmp;
                tx_cookie = txd->tx_submit(txd);
                printk("rpdma:tx submit \n");
            }
        }
        printk("rpdma:rx dma_async_issue_pending \n");
        dma_async_issue_pending(tx_chan);

        
    break;
    }
    
        case SIMPLE_RX:{
        printk("rpdma:rx single dma \n");
        dma_rx_segment = dma_map_single(rx_dev->dev, rpdma_rx_addrv, rx_segment_size*rx_segment_cnt, DMA_FROM_DEVICE);
        if(dma_mapping_error(tx_dev->dev,dma_rx_segment)){printk("rpdma:dma_rx_segment not set properly\n");}
        else{
        rxd = dmaengine_prep_slave_single(rx_chan,rpdma_rx_addrp, rx_segment_size*rx_segment_cnt, DMA_DEV_TO_MEM, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
            if(!rxd){printk("rpdma:rxd not set properly\n");}
            else{
                init_completion(&rx_cmp);
                rxd->callback = rpdma_slave_rx_callback;
                rxd->callback_param = &rx_cmp;
                rx_cookie = rxd->tx_submit(rxd);
                printk("rpdma:rx submit \n");
            }
        }

        printk("rpdma:rx dma_async_issue_pending \n");
        dma_async_issue_pending(rx_chan);    
        flag = 0;
        rx_tmo = wait_for_completion_timeout(&rx_cmp, rx_tmo);
        
            
    break;
    }
    case SIMPLE_TX:{
        printk("rpdma:tx single dma \n");
        dma_tx_segment = dma_map_single(tx_dev->dev, rpdma_tx_addrv, tx_segment_size*tx_segment_cnt,DMA_TO_DEVICE);
        if(dma_mapping_error(tx_dev->dev,dma_tx_segment)){printk("rpdma:dma_tx_segment not set properly\n");}
        else{
            txd = dmaengine_prep_slave_single(tx_chan, dma_tx_segment, tx_segment_size*tx_segment_cnt, DMA_MEM_TO_DEV, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
            if(!txd){printk("rpdma: txd not set properly\n");}
            else{
                init_completion(&tx_cmp);
                txd->callback = rpdma_slave_tx_callback;
                txd->callback_param = &tx_cmp;
                tx_cookie = txd->tx_submit(txd);
                printk("rpdma:tx submit \n");
            }
        }
        printk("rpdma:rx dma_async_issue_pending \n");
        dma_async_issue_pending(tx_chan);
        
        tx_tmo = wait_for_completion_timeout(&tx_cmp, tx_tmo);
        dmaengine_terminate_all(tx_chan);
        if(dma_tx_segment)
        dma_unmap_single(tx_dev->dev, dma_tx_segment, tx_segment_size*tx_segment_cnt, DMA_TO_DEVICE);
        
    break;
}
    case SIMPLE:{
        printk("rpdma:tx single dma \n");
        dma_tx_segment = dma_map_single(tx_dev->dev, rpdma_tx_addrv, tx_segment_size*tx_segment_cnt,DMA_TO_DEVICE);
        if(dma_mapping_error(tx_dev->dev,dma_tx_segment)){printk("rpdma:dma_tx_segment not set properly\n");}
        else{
            txd = dmaengine_prep_slave_single(tx_chan, dma_tx_segment, tx_segment_size*tx_segment_cnt, DMA_MEM_TO_DEV, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
            if(!txd){printk("rpdma: txd not set properly\n");}
            else{
                init_completion(&tx_cmp);
                txd->callback = rpdma_slave_tx_callback;
                txd->callback_param = &tx_cmp;
                tx_cookie = txd->tx_submit(txd);
                printk("rpdma:tx submit \n");
            }
        }
        printk("rpdma:rx single dma \n");
        dma_rx_segment = dma_map_single(rx_dev->dev, rpdma_rx_addrv, rx_segment_size*rx_segment_cnt, DMA_FROM_DEVICE);
        if(dma_mapping_error(tx_dev->dev,dma_rx_segment)){printk("rpdma:dma_rx_segment not set properly\n");}
        else{
        rxd = dmaengine_prep_slave_single(rx_chan,rpdma_rx_addrp, rx_segment_size*rx_segment_cnt, DMA_DEV_TO_MEM, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
            if(!rxd){printk("rpdma:rxd not set properly\n");}
            else{
                init_completion(&rx_cmp);
                rxd->callback = rpdma_slave_rx_callback;
                rxd->callback_param = &rx_cmp;
                rx_cookie = rxd->tx_submit(rxd);
                printk("rpdma:rx submit \n");
            }
        }
        printk("rpdma:tx dma_async_issue_pending \n");
        dma_async_issue_pending(tx_chan);
        printk("rpdma:rx dma_async_issue_pending \n");
        dma_async_issue_pending(rx_chan);    
        

        tx_tmo = wait_for_completion_timeout(&tx_cmp, tx_tmo);
        
        dmaengine_terminate_all(rx_chan);
        dmaengine_terminate_all(tx_chan);
        
        if(dma_tx_segment)
        dma_unmap_single(tx_dev->dev, dma_tx_segment, tx_segment_size*tx_segment_cnt, DMA_TO_DEVICE);
        if(dma_rx_segment)
        dma_unmap_single(rx_dev->dev, dma_rx_segment, rx_segment_size*rx_segment_cnt, DMA_FROM_DEVICE);
    break;
    }
    
    case SET_TX_SEGMENT_CNT:{
        printk("rpdma:ioctl tx segment cnt set to %lx \n",arg);
        tx_segment_cnt=arg;
    }break;  
    case SET_TX_SEGMENT_SIZE :{
        printk("rpdma:ioctl tx segment size set to %lx \n",arg);
        tx_segment_size=arg;
   }break;
    case SET_RX_SEGMENT_CNT:{
        printk("rpdma:ioctl rx segment cnt set to %lx \n",arg);
        rx_segment_cnt=arg;
   }break;
    case SET_RX_SEGMENT_SIZE:{
        printk("rpdma:ioctl rx segment size set to %lx \n",arg);
        rx_segment_size=arg;
    }break;  
              
    }
    return 0;
}

//dealocate everything for dma
static int rpdma_release(struct inode *ino, struct file *file)
{
    //  dmaengine_terminate_all(tx_chan); 
   //     dmaengine_terminate_all(rx_chan); 
    //if(dma_tx_segment)
    //    dma_unmap_single(tx_dev->dev, dma_tx_segment, tx_segment_size*tx_segment_cnt, DMA_TO_DEVICE);
    //        if(dma_rx_segment)
//        dma_unmap_single(rx_dev->dev, dma_rx_segment, rx_segment_size*rx_segment_cnt, DMA_FROM_DEVICE);
            flag=1;
    wake_up_interruptible(&wq);
    printk("rpdma:release\n");
    return 0;
}


//mmaps receiver buffer to userspace
static int rpdma_mmap(struct file * f, struct vm_area_struct * v){
    printk("rpdma:mmap\n");
    return dma_common_mmap(&rpdev->dev, v, rpdma_rx_addrv, rpdma_rx_addrp,v->vm_end - v->vm_start);    
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
    tx_chan = dma_request_slave_channel(&pdev->dev, "axidma0");
    if (IS_ERR(tx_chan)) {
        pr_err("rpdma: No Tx channel\n");
        return PTR_ERR(tx_chan);
    }

    rx_chan = dma_request_slave_channel(&pdev->dev, "axidma1");
    if (IS_ERR(rx_chan)) {
        err = PTR_ERR(rx_chan);
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
    
    tx_dev = tx_chan->device;
    rx_dev = rx_chan->device;
    
    rpdma_tx_addrv =  dma_alloc_coherent(tx_dev->dev, TX_SGMNT_CNT*TX_SGMNT_SIZE, &rpdma_tx_addrp, GFP_ATOMIC);
    if(rpdma_tx_addrv==NULL){
        printk("rpdma: ERROR tx not allocated\n");
    }else printk("rpdma: reserved %ld for tx \n",TX_SGMNT_CNT*TX_SGMNT_SIZE);
    rpdma_rx_addrv=  dma_alloc_coherent(rx_dev->dev, RX_SGMNT_CNT*RX_SGMNT_SIZE, &rpdma_rx_addrp, GFP_ATOMIC);
    if(rpdma_tx_addrv==NULL){
    printk("rpdma: ERROR rx not allocated\n");
    }else printk("rpdma: reserved %ld for rx \n",RX_SGMNT_CNT*RX_SGMNT_SIZE);
    return 0;
    
rmdev:
    device_destroy(cl, dev_num);
    class_destroy(cl);
    unregister_chrdev_region(dev_num, 1);
    dma_release_channel(rx_chan);
free_tx:
    dma_release_channel(tx_chan);

    return err;
}

//character device dealocation
//dma channel dealocation
//cma buffer dealocation
static int rpdma_remove(struct platform_device *pdev)
{
    printk("rpdma:remove\n");
    if(rx_chan){
        dma_release_channel(rx_chan);
    }
    if(tx_chan){
        dma_release_channel(tx_chan);
    }
    
    cdev_del(&c_dev);
    device_destroy(cl,dev_num);
    class_destroy(cl);
    unregister_chrdev_region(dev_num,1);
    
    //if dma memmory is still allocated return it to cma memory allocator pool
    if(rpdma_rx_addrv){
       dma_free_coherent(NULL, RX_SGMNT_CNT*RX_SGMNT_SIZE, rpdma_rx_addrv, rpdma_handle);
    }
    
    if(rpdma_tx_addrv){
       dma_free_coherent(NULL,TX_SGMNT_CNT*TX_SGMNT_SIZE, rpdma_tx_addrv, rpdma_handle);
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
