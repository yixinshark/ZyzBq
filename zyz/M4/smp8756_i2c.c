#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include <fcntl.h>

#define ALLOW_OS_CODE 1

// RUA / EMhwlib include
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

// RUA selection
RMuint32 chip_num = 0;

// I2C ModuleID index
RMuint32 i2c_bus = 1;
// GPIO of the I2C data line (software I2C only)
enum GPIOId_type i2c_clock = GPIOId_Sys_0;
// GPIO of the I2C clock line (software I2C only)
enum GPIOId_type i2c_data = GPIOId_Sys_1;
// I2C device address (write uses DevAddr, read uses DevAddr + 1)
RMuint8 i2c_device = 0x00;
// I2C delay, in uSec
RMuint32 i2c_delay = 0;
// frequency of the I2C bit transfer, in kHz (e.g. 100 or 400)
RMuint32 i2c_speed = 100;



typedef   RUA  i2c_devhandle;


//Set up rmi2c callback pointers for I2C access through ruai2c
static void prepare_i2c(struct rmi2c_device *rmi2c, struct ruai2c_device *ruai2c)
{
	rmi2c->pContext = (void *)ruai2c;
	rmi2c->device = 0x00;
	rmi2c->check_burst = (RMstatus(*)(void *))&ruai2c_check_burst;
	rmi2c->segment = (RMstatus(*)(void *, RMuint8, RMuint8))&ruai2c_segment;
	rmi2c->write = (RMstatus(*)(void *, RMint32, RMuint8, RMuint8))&ruai2c_write;
	rmi2c->read = (RMstatus(*)(void *, RMint32, RMuint8, RMuint8 *))&ruai2c_read;
	rmi2c->write_data = (RMstatus(*)(void *, RMint32, RMuint8, RMuint8 *, RMuint32))&ruai2c_write_data;
	rmi2c->read_data = (RMstatus(*)(void *, RMint32, RMuint8, RMuint8 *, RMuint32))&ruai2c_read_data;
	rmi2c->write_nosub = (RMstatus(*)(void *, RMint32, RMuint8 *, RMuint32))&ruai2c_write_nosub;
	rmi2c->read_nosub = (RMstatus(*)(void *, RMint32, RMuint8 *, RMuint32))&ruai2c_read_nosub;
	rmi2c->write_large_data = (RMstatus(*)(void *, RMint32, RMbool, RMuint8, RMuint8 *, RMuint32))&ruai2c_write_large_data;
	rmi2c->read_large_data = (RMstatus(*)(void *, RMint32, RMbool, RMuint8, RMuint8 *, RMuint32))&ruai2c_read_large_data;
};

int smp8756_i2c_open(struct i2c_devhandle * handle ,unsigned int chip_num)
{
   	RMstatus err;
	int ret = 0;
    
    struct ruai2c_device ruai2c;
    struct rmi2c_device rmi2c;

   	// Open and init EMhwlib (as RUA)
	if (RMFAILED(err = RUACreateInstance(&handle, chip_num))) {
		RMNOTIFY((NULL, err, "Error creating RUA instance!\n"));
		return -1;
	}
  
	// Connect RUA to ruai2c
	ruai2c.pRUA = handle;
	
	// Pass local I2C bus options to ruai2c
	ruai2c.I2CModuleID = EMHWLIB_MODULE(EMHWLIB_CATEGORY(I2C), i2c_bus);
	ruai2c.I2CDevice.APIVersion = 1;
	ruai2c.I2CDevice.Clock = i2c_clock;
	ruai2c.I2CDevice.Data = i2c_data;
	ruai2c.I2CDevice.DevAddr = i2c_device;
	ruai2c.I2CDevice.Delay = i2c_delay;
	ruai2c.I2CDevice.Speed = i2c_speed;
	
	// Connect ruai2c to rmi2c instance
	prepare_i2c(&(rmi2c), &(ruai2c));
	
	// Detect and circumvent faulty burst write (EM8622L, SMP8634 Rev.A/B)
	rmi2c_check_burst(&(rmi2c));

    return 0;
}
int smp8756_i2c_op(i2c_dev_handle *handle, int rw_flag, unsigned char dev_addr,  unsigned char *pdata, int count)
{
    RMstatus err;
    if(rw_flag == 0) //rw_flag = 0, read
    {
        err = ruai2c_read_data(handle, 0, dev_addir, pdata, count);
    }
    else
    {
        err = ruai2c_write_data(handle, 0, dev_addr, pdata, count);
    }
    
    return 0;
}


int smp8756_i2c_close(struct i2c_devhandle * handle)
{
    // Close EMhwlib
	RUADestroyInstance(handle);

    return 0;
}








