/**
 * $Id$
 *
 * @brief Red Pitaya Nginx module - Bazaar.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *         Ales Bardorfer <ales.bardorfer@redpitaya.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>

#include "rp_bazaar_cmd.h"
#include "rp_bazaar_app.h"

const char *c_rp_app_init_str     = "rp_app_init";
const char *c_rp_app_exit_str     = "rp_app_exit";
const char *c_rp_app_desc_str     = "rp_app_desc";
const char *c_rp_params_desc_str  = "rp_params_desc";
const char *c_rp_signals_desc_str = "rp_signals_desc";
const char *c_rp_set_params_str   = "rp_set_params";
const char *c_rp_get_params_str   = "rp_get_params";
const char *c_rp_set_signals_str  = "rp_set_signals";
const char *c_rp_get_signals_str  = "rp_get_signals";


/** Get MAC address of a specific NIC via sysfs */
int rp_bazaar_get_mac(const char* nic, char *mac)
{
    FILE *fp;
    ssize_t read;
    const size_t len = 17;

    fp = fopen(nic, "r");
    if(fp == NULL) {
        return -1;
    }

    read = fread(mac, len, 1, fp);
    if(read != 1) {
        fclose(fp);
        return -2;
    }
    mac[len] = '\0';

    fclose(fp);

    return 0;
}


/** Get DNA number */
int rp_bazaar_get_dna(unsigned long long *dna)
{
    void *page_ptr;
    long page_addr, page_size = sysconf(_SC_PAGESIZE);
    const long c_dna_fpga_base_addr = 0x40000000;
    const long c_dna_fpga_base_size = 0x20;
    int fd = -1;

    fd = open("/dev/mem", O_RDONLY | O_SYNC);
    if(fd < 0) {
        fprintf(stderr, "open(/dev/mem) failed: %s\n", strerror(errno));
        return -1;
    }

    page_addr = c_dna_fpga_base_addr & (~(page_size-1));

    page_ptr = mmap(NULL, c_dna_fpga_base_size, PROT_READ,
                          MAP_SHARED, fd, page_addr);

    if((void *)page_ptr == MAP_FAILED) {
        fprintf(stderr, "mmap() failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    hk_fpga_reg_mem_t *hk = page_ptr;
    *dna = hk->dna_hi & 0x00ffffff;
    *dna = *dna << 32 | hk->dna_lo;

    if(munmap(page_ptr, c_dna_fpga_base_size) < 0) {
        fprintf(stderr, "munmap() failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    close (fd);

    return 0;
}

/* Returns 1 if file is readable within the "app_id" 
 * application directory.
 * Returns 0 otherwise.
 */
inline int is_readable(const char *dir,
                       const char *app_id,
                       const char *fname)
{
    char *file = NULL;
    int file_len = strlen(dir) + strlen(app_id) + strlen(fname) + 3;
    struct stat stat_buf;
    
    file = (char *)malloc(file_len);
    if(file == NULL) {
        fprintf(stderr, "Can not allocate memory: %s", strerror(errno));
        return 0;
    }

    sprintf(file, "%s/%s/%s", dir, app_id, fname);

    if(stat((const char *)file, &stat_buf) < 0) {
        /* File does not exist */
        free(file);
        return 0;
    }
    if(!(stat_buf.st_mode & S_IRUSR)) {
        /* Permissions wrong */
        fprintf(stderr, "%s exists but is not readable.\n", file);
        free(file);
        return 0;
    }
    
    free(file);
    
    return 1;
}


/* Returns 1 if app info is found within the "app_id" 
 * application directory.
 * Returns 0 otherwise.
 *
 * If successful, info is parsed from info/desc.json.
 */
int get_info(cJSON **info, const char *dir, const char *app_id, ngx_pool_t *pool)
{
    char *file = NULL;
    char *data = NULL;
    cJSON *json = NULL;
    FILE *fp = NULL;
    size_t len, read;
    int ret = 1;

    /* Read description JSON file */
    const char *fname = "info/info.json";
    int file_len = strlen(dir) + strlen(app_id) + strlen(fname) + 3;

    file = (char *)malloc(file_len);
    if(file == NULL) {
        fprintf(stderr, "Can not allocate memory: %s", strerror(errno));
        ret = 0;
        goto out;
    }

    sprintf(file, "%s/%s/%s", dir, app_id, fname);
    
    fp = fopen(file, "r");
    if(fp == NULL) {
        fprintf(stderr, "Cannot open %s.\n", file);
        ret = 0;
        goto out;
    }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0 ,SEEK_SET);

    data = (char *)malloc(len+1);
    if(data == NULL) {
        fprintf(stderr, "Can not allocate memory: %s", strerror(errno));
        ret = 0;
        goto out;
    }

    read = fread(data, len, 1, fp);
    if(read != 1) {
        fprintf(stderr, "Cannot read from %s.\n", file);
        ret = 0;
        goto out;
    }

    /* Get rid of comments */
    cJSON_Minify(data);

    /* Parse relevant JSON content */
    json = cJSON_Parse(data, pool);
    if(json == NULL) {
        fprintf(stderr, "Error parsing JSON before [%s].\n", cJSON_GetErrorPtr());
        ret = 0;
        goto out;
    }

    *info = json;

 out:
    /* Do not delete json: 
     * If it is added to some json tree, it will be deleted as a branch.
     * If not, the caller is responsible to delete it.
     */
    if (data)  free(data);
    if (file)  free(file);
    if (fp)    fclose(fp);
    
    return ret;
}


/* Returns 1 if application controller within the "app_id" 
 * application directory is OK.
 * Returns 0 otherwise.
 */
inline int is_controller_ok(const char *dir,
                            const char *app_id,
                            const char *fname)
{
    char *file = NULL;
    int file_len = strlen(dir) + strlen(app_id) + strlen(fname) + 3;
    struct stat stat_buf;
    const mode_t perms = S_IRUSR | S_IXUSR;
    
    file = (char *)malloc(file_len);
    if(file == NULL) {
        fprintf(stderr, "Can not allocate memory: %s", strerror(errno));
        return 0;
    }

    sprintf(file, "%s/%s/%s", dir, app_id, fname);

    if(stat((const char *)file, &stat_buf) < 0) {
        /* File does not exist */
        free(file);
        return 0;
    }
    if((stat_buf.st_mode & perms) != perms) {
        /* Permissions wrong */
        fprintf(stderr, "%s exists but has wrong permissions.\n", file);
        free(file);
        return 0;
    }
    
    rp_bazaar_app_t app;
    ngx_memset(&app, 0, sizeof(rp_bazaar_app_t));

    /* Open and check init, exit and desc symbols - if they exist,
     * controller is OK.
     */

    if(rp_bazaar_app_load_module(file, &app) < 0) {
        fprintf(stderr, "Problem loading app: %s\n", dlerror());
        rp_bazaar_app_unload_module(&app);
        free(file);
        return 0;
    }

    rp_bazaar_app_unload_module(&app);

    free(file);
    
    return 1;
}


int rp_bazaar_app_get_local_list(const char *dir, cJSON **json_root,
                                 ngx_pool_t *pool, int verbose)
{
    DIR *dp;
    struct dirent *ep;

    if((dp = opendir(dir)) == NULL)
        return rp_module_cmd_error(json_root, "Can not open apps directory", 
                                   strerror(errno), pool);

    while((ep = readdir (dp))) {
        const char *app_id = ep->d_name;
        cJSON *info = NULL;

        /* check if structure is correct, we need: 
         *  <app_id>/info/info.json
         *  <app_id>/info/icon.png
         *  <app_id>/controller.so
         *  <app_id>/fpga.bit
         * And we must be able to load the application and test mandatory
         * functions.
         */
        
        if (!is_readable(dir, app_id, "info/icon.png"))
            continue;
        if (!is_readable(dir, app_id, "fpga.bit"))
            continue;
        if (!is_controller_ok(dir, app_id, "controller.so"))
            continue;
        if (!get_info(&info, dir, app_id, pool))
            continue;
        if (info == NULL)
            continue;

        /* We have an application */

        if (verbose) {
            /* Attach whole info JSON */
            cJSON_AddItemToObject(*json_root, app_id, info, pool);
        } else {
            /* Include version only */
            cJSON *j_ver = cJSON_GetObjectItem(info, "version");
            if(j_ver == NULL) {
                fprintf(stderr, "Cannot get version from info JSON.\n");
                cJSON_Delete(info, pool);
                continue;
            }
            
            cJSON_AddItemToObject(*json_root, app_id,
                                  cJSON_CreateString(j_ver->valuestring, pool),
                                  pool);
            cJSON_Delete(j_ver, pool);
            cJSON_Delete(info, pool);
        }
    }

    closedir(dp);

    return 0;
}

int rp_bazaar_app_load_module(const char *app_file, rp_bazaar_app_t *app)
{
    if(app->handle != NULL) {
        rp_bazaar_app_unload_module(app);
    }

    app->handle = dlopen(app_file, RTLD_LAZY);
    if(!app->handle)
        return -1;

    dlerror(); /* clear error */
    app->init_func = dlsym(app->handle, c_rp_app_init_str);
    if(!app->init_func)
        return -1;

    dlerror(); /* clear error */
    app->exit_func = dlsym(app->handle, c_rp_app_exit_str);
    if(!app->exit_func)
        return -1;

    dlerror(); /* clear error */
    app->desc_func = dlsym(app->handle, c_rp_app_desc_str);
    if(!app->desc_func)
        return -1;

    app->set_params_func  = dlsym(app->handle, c_rp_set_params_str);
    if(!app->set_params_func)
        return -1;

    app->get_params_func = dlsym(app->handle, c_rp_get_params_str);
    if(!app->get_params_func)
        return -1;

    app->get_signals_func = dlsym(app->handle, c_rp_get_signals_str);
    if(!app->get_signals_func)
        return -1;

    app->file_name = (char *)malloc(strlen(app_file)+1);
    if(app->file_name == NULL)
        return -1;
    
    strncpy(app->file_name, app_file, strlen(app_file));
    app->file_name[strlen(app_file)] = '\0';

    return 0;
}

int rp_bazaar_app_unload_module(rp_bazaar_app_t *app)
{
    if(app->handle) {
        if(app->initialized && app->exit_func) {
            app->exit_func();
        }
        if(app->file_name) {
            free(app->file_name);
        }
        if (app->id) {
            free(app->id);
        }
        dlclose(app->handle);


        ngx_memset(app, 0, sizeof(rp_bazaar_app_t));
    }
    return 0;
}

/* Use xdevcfg to load the data - using 32k buffers */
#define RP_FPGA_CONF_BUF_LEN (32*1024)
int rp_bazaar_app_load_fpga(const char *fpga_file)
{
    unsigned char buff[RP_FPGA_CONF_BUF_LEN];
    int fo, fi;
    int ret_val = 0;

    fo = open("/dev/xdevcfg", O_WRONLY);
    if(fo < 0) {
        fprintf(stderr, "rp_bazaar_app_load_fpga() failed to open xdevcfg: %s\n",
                strerror(errno));
        return -1;
    }

    fi = open(fpga_file, O_RDONLY);
    if(fi < 0) {
        fprintf(stderr, "rp_bazaar_app_load_fpga() failed to open FPGA file: %s\n",
                strerror(errno));
        return -1;
    }

    while(1) {
        int ret_w;
        int ret_r = read(fi, &buff[0], RP_FPGA_CONF_BUF_LEN);
        if(ret_r < 0) {
            fprintf(stderr, "rp_bazaar_app_load_fpga() read failed: %s\n", 
                    strerror(errno));
            ret_val = -1;
            break;
        }
        if(ret_r == 0) {
            /* finished loading */
            break;
        }
        
        ret_w = write(fo, &buff[0], ret_r);
        if(ret_w < 0) {
            fprintf(stderr, "rp_bazaar_app_load_fpga() write failed: %s\n",
                    strerror(errno));
            ret_val = -1;
            break;
        }
    }

    close(fo);
    close(fi);

    return ret_val;
}
