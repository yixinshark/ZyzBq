/*************************************************************************
	> File Name: smp8756_pca9535.c
	> Author: 
	> Mail: 
	> Created Time: 2015年04月30日 星期四 10时09分17秒
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include "smp8756_pca9535.h"
#include "smp8756_i2c.h"

unsigned int chip_num = 0;
//void *handle = NULL;
unsigned char dev_addr = 0x40;
//unsigned char output_data[2]; //UNDONE

static int i2c_write(void *handle, unsigned char reg, unsigned char value)
{
    unsigned char output_data[2];
    output_data[0] = reg;
    output_data[1] = value;

    smp8756_i2c_op(handle, 1, dev_addr, output_data, 2);

    return 0;
}

static int i2c_read(void *handle, unsigned char reg, unsigned char *value)//UNDONE
{
    unsigned char output_data[2];
    #if 0
    output_data[0] = reg;
    smp8756_i2c_op(handle, 0, dev_addr, output_data, 2);
    *value = output_data[1];
    #endif
    output_data[0] = reg;
    smp8756_i2c_op(handle, 1, dev_addr, output_data, 1); //call write
    smp8756_i2c_op(handle, 0, dev_addr, output_data+1, 1);//call read
    *value = output_data[1];
    return 0;
}

static int pca9535_write_output_data(void *handle, int index, unsigned char data)
{

    if (0 == index)
    {
        i2c_write(handle, OUTPUT_PORT0_DATA, data);
    }
    else if (1 == index) 
    {
        i2c_write(handle, OUTPUT_PORT1_DATA, data);
    }

    return 0;
}

static int smp8756_pca9535_config(void *handle)
{
    unsigned char config;

    {   //port 0 [0-7] output
        i2c_read(handle, IOPORT0_CTL, &config);
        config &= ~(0xff); //config = 0x0
        config = 0x0;
        i2c_write(handle, IOPORT0_CTL, config);
    }

    {   //port 1 [10-11] input,[15-17] output
        i2c_read(handle, IOPORT1_CTL, &config);
       // config &= ~(0x03);
       // config |= 0x0f;
        config = 0x03;
        i2c_write(handle, IOPORT1_CTL, config);
    }
    
    return 0;
}


int main(void)
{
    void * handle;
    handle = smp8756_i2c_open(chip_num);
    printf("handle------------------------1\n");
    
    smp8756_pca9535_config(handle);
    pca9535_write_output_data(handle, 0, 0x0);
    pca9535_write_output_data(handle, 1, 0xe0);//p16 = 1;
    printf("handle------------------------2\n");

    sleep(1);
    pca9535_write_output_data(handle, 1, 0x0);
    sleep(1);
    printf("handle------------------------3\n");

    while(1);
    smp8756_i2c_close(handle);
    return 0;
}



