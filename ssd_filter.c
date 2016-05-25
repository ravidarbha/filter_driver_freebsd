#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/bio.h>
#include <sys/conf.h>
#include <sys/bus.h>
#include <geom/geom_disk.h>
#include <dev/ata/ata-disk.h>

disk_strategy_t *original_strategy_fn; 
int init_filter(struct module *module, int event, void *arg); 
int change_request_fn(void);
int restore_request_fn(void);
void *blkdev_get_by_path(char *path);
void my_filter_request(struct bio *bio);

void my_filter_request(struct bio *bio)
{
    // Tamper with the sector and the sizes for reads to fail. 
  
    if(bio->bio_cmd == BIO_READ)
    {

         printf("entering the filter for the request ..%lld\n",bio->bio_pblkno);
         bio->bio_pblkno = -100;
    }
    // MAke the original request.
    original_strategy_fn(bio);
 
    return ;
}
struct disk *disk;
int change_request_fn(void)
{
   char path[10] = "ad1";
   disk = blkdev_get_by_path(path);
   if(disk)
   {
       original_strategy_fn = disk->d_strategy;
       disk->d_strategy = my_filter_request;
       printf("SUccess \n");
   }

   printf("change request fn \n"); 
   return 0;
}
 
int restore_request_fn(void)
{
    printf("restore requestfn \n"); 
    if( disk)
    disk->d_strategy = original_strategy_fn;
    return 0;
}

extern void *g_path_to_disk(char *str);

void *blkdev_get_by_path(char *path)
{
   return g_path_to_disk(path);
}

int init_filter(struct module *module, int event, void *arg) 
{
    int ret = 0;

    switch(event)
    {
       case MOD_LOAD:
       {
          if((ret = change_request_fn()))
          {
            printf("Filter changing error ..\n");
          }
           break;
       }
       case MOD_UNLOAD:
       {

          restore_request_fn();
          printf("EXIT.\n");
          break;
       } 
       default:
           printf("default hanler");
    } 
    return 0;
}



/* The second argument of DECLARE_MODULE. */
moduledata_t moduleData = {
    "filter",    /* module name */
     init_filter,  /* event handler */
     NULL            /* extra data */
};
 
DECLARE_MODULE(filter, moduleData, SI_SUB_DRIVERS, SI_ORDER_MIDDLE); 
MODULE_DEPEND(filter, ad ,1,1,1);
