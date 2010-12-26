#include "dev.h"
#include "list.h"

struct list_head dev_list;

int init_dev()
{
    INIT_LIST_HEAD(&dev_list);
	return 0;
}

/**
 * 
 * 
 * @author chenxb (9/26/2010)
 * 
 * @param dev  [OUT] 
 * @param type [IN]
 * @param id     [IN]
 * @param des  [IN]
 * @param prs232  [IN]
 */
void add_dev( struct st_dev *dev)
{
    list_add(&(dev->node), &dev_list);
}

/*  */
void del_dev( struct st_dev *dev)
{
    list_del(&(dev->node));
}

struct st_dev *seach_dev(int addr)
{
    struct list_head *pos;
    struct st_dev *dev;
    list_for_each(pos, &(dev_list)){
        dev = list_entry(pos, struct st_dev, node);
        if(dev->addr == addr)  return dev;

    }
    return NULL;
}

void dev_reset(int flag, struct st_dev *set_dev)
{
    struct st_dev *dev;
    struct list_head *pos;
    switch(flag) {
    case 1:
		
        list_for_each_prev(pos, &dev_list){
            dev = list_entry(pos, struct st_dev, node);
            printf(" dev.addr =  %d\n", dev->addr);
			
			if( dev->dev_reset ) dev->dev_reset(dev);
            
        }
        break;
    case 0:
        if(set_dev) {
            if( set_dev->dev_reset ) 
                 set_dev->dev_reset(set_dev);
        }
        break;
    default:
        if(set_dev) {
            if( set_dev->dev_reset )
                set_dev->dev_reset(set_dev);
        }
        break;
    }
    printf("end reset\n");
}
