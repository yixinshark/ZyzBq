/*************************************************************************
	> File Name: smp8756_i2c.h
	> Author: 
	> Mail: 
	> Created Time: 2015年04月21日 星期二 16时52分18秒
 ************************************************************************/

#ifndef _SMP8756_I2C_H
#define _SMP8756_I2C_H

#define DEV_ADDR 0x0
#if 0
#define ALLOW_OS_CODE 1

#include "rua/include/rua.h"
#include "rmcore/include/rmcore.h"
#include "rmlibcw/include/rmlibcw.h"
#include "rmi2c/include/ruai2c.h"
#include "rmi2c/include/rmi2c.h"

#ifdef RMBUILD_USE_HWLIB_V2
#define EMHWLIB_CATEGORY(mod) EMhwlib ## mod
#else // RMBUILD_USE_HWLIB_V2
#define EMHWLIB_CATEGORY(mod) mod
#endif // RMBUILD_USE_HWLIB_V2
typedef struct RUA  i2c_devhandle ;

#endif


typedef struct  ruai2c_device  i2c_devhandle;

//void prepare_i2c(struct rmi2c_device *rmi2c, struct ruai2c_device *ruai2c);
void * smp8756_i2c_open(unsigned int chip_num);
int smp8756_i2c_op(void * handle, int rw_flag, unsigned char dev_addr, unsigned char *pdata,unsigned int count);
int smp8756_i2c_close(void *handle);

#endif
