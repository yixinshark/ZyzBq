/*************************************************************************
	> File Name: m4_update.h
	> Author: 
	> Mail: 
	> Created Time: 2015年04月02日 星期四 16时30分32秒
 ************************************************************************/

#ifndef _M4_UPDATE_H
#define _M4_UPDATE_H

//app program starting address and the size of the program
unsigned int Program_Address = 0x00002800;
unsigned int Program_Size    ;
unsigned char File_Path[] = "/home/backup/program.bin";

#define  SEND_DATA_SIZE          120;

//The following commands are used by the custom protocal on the I2C0
#define  COMMAND_PING            0x20
#define  COMMAND_DOWNLOAD        0X21
#define  COMMAND_RUN             0x22
#define  COMMAND_GET_STATUS      0X23
#define  COMMAND_SEND_DATA       0X24
#define  COMMAND_RESET           0X25

//This following are the definitions for the possible status 
//values that can be retruned form m4 bootloader when COMMAND_GET_STATUS
//is sent to the m4 bootloader
#define COMMAND_RET_SUCCESS          0x40
#define COMMAND_RET_UNKNOWN_CMD      0x41
#define COMMAND_RET_INVALID_CMD      0x42
#define COMMAND_RET_INVALID_ADR      0x43
#define COMMAND_RET_FLASH_FAIL       0x44
//#define COMMAND_RET_CRC_FAIL       0x45

//DEFINE ACK AND NAK
#define COMMAND_ACK                  0XCC
#define COMMAND_NAK                  0X33

//the statement of the COMMAND_PING:
unsigned char command_ping[1];

//the statement  of the COMMAND_DOWNLOAD:
unsigned char command_download[9];

//the statement of the COMMAND_RUN
unsigned char command_run[5]

//the statement of the COMMAND_GET_STATUS
unsigned char command_get_status[1];

//the statement of the COMMAND_SEND_DATA:
unsigned char command_send_date[SEND_DATA_SIZE] ;
//command_send_date[0] = COMMAND_SEND_DATA;

//the statement of the COMMAND_RESET
unsigned char command_rest[1] ;


__BEGIN_DECLS

static void Command_Init(void);
static unsigned int Get_file_size(const char * path);
static unsigned int CheckSum(const char *DataBuffer, unsigned int Size);
static int Send_Packet(unsigned char *DataBuffer, unsigned int Size);
static int Send_Command(unsigned char * DataBuffer, unsigned int Size);

__END_DECLS


#endif
