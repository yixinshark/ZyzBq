/*************************************************************************
	> File Name: smp8756_pca9535.h
	> Author: 
	> Mail: 
	> Created Time: 2015年04月30日 星期四 10时06分33秒
 ************************************************************************/

#ifndef _SMP8756_PCA9535_H
#define _SMP8756_PCA9535_H
#define PCA9535BUS  "/dev/i2c-0"
#define IOPORT0_CTL 0x6
#define IOPORT1_CTL 0x7
#define INPUT_PORT0_DATA 0x0
#define INPUT_PORT1_DATA 0x1
#define OUTPUT_PORT0_DATA 0x2
#define OUTPUT_PORT1_DATA 0x3
#define PORT0_POLARITY 0x4
#define PORT1_POLARITY 0x5

static int i2c_write(void *handle, unsigned char reg, unsigned char value);
static int i2c_read(void *handle, unsigned char reg, unsigned char *value);

static int pca9535_write_output_data(void *handle, int index, unsigned char data);

static int smp8756_pca9535_config(void *handle);


#endif
