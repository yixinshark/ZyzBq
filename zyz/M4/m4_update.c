/*************************************************************************
	> File Name: m4_update.c
	> Author: 
	> Mail: 
	> Created Time: 2015年04月02日 星期四 16时30分45秒
 ************************************************************************/

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "m4_update.h"

#define Bit(x,y)  ((x & (0xff << y)) >> y)

static void SetforI2C(void)
{
    //init I2C0

}

static void GPIOInit(void)
{
    // init GPIO pins(used to Rest m4 and force to update)
}
static void Command_Init(void)
{
    //the format of the COMMAND_PING
    command_ping[0] = COMMAND_PING;

    //the format of the COMMAND_GET_STATUS
    command_get_status[0] = COMMAND_GET_STATUS;
    
    // the format of the COMMAND_DOWNLOAD
    command_download[0] = COMMAND_DOWNLOAD;
    command_download[1] = Bit(Program_Address, 24);
    command_download[2] = Bit(Program_Address, 16);
    command_download[3] = Bit(Program_Address, 8);
    command_download[4] = Bit(Program_Address, 0);
    command_download[5] = Bit(Program_Size, 24);
    command_download[6] = Bit(Program_Size, 16);
    command_download[7] = Bit(Program_Size, 8);
    command_download[8] = Bit(Program_Size, 0);

    //the format of the COMMAND_SEND_DATA
    command_send_data[0] = COMMAND_SEND_DATA;

}

// get the size of file which given by path
static unsigned int Get_file_size(const char *path)
{
    unsigned long FileSize = 0;
    struct stat statbuff;
    if(stat(path, &statbuff) < 0)
    {
        return  -1;
    }
    else 
    {
        FileSize = statbuff.st_size;
    }

    return FileSize;
}

static unsigned int CheckSum(const char *DataBuffer, unsigned int Size)
{
    unsigned int CheckRes;
    //Initialize the checkres to zero
    CheckRes = 0;

    //Add up all the bytes, don't do anything for an overflow
    while(Size--)
    {
        CheckRes += *DataBuffer++;
    }

    //  Return the caculated check sum

    return (CheckRes & 0xff);
}

static int Send_Packet(unsigned char * DataBuffer, unsigned int Size)
{
    unsigned int Temp;

    //caculated the checksum to be sent out with the data
    Temp = CheckSum(DataBuffer, Size);
    
    //Need to include the size (1 byte) and checksum (1 byte) in the packet
    Size += 2;

    // Send out the size , checkres followed by the Data

    SendData((unsigned char *)&Size, 1);
    SendData((unsigned char *)&Temp, 1);
    SendData(DataBuffer, Size - 2);

    //wait for a non-zero byte 
    Temp = 0;
    while(Temp == 0)
    {
        ReceiveData((unsigned char *)&Temp, 1);
    }

    // Check if the byte was a valid ACK and Return -1 if it is not ACK

    if(Temp != COMMAND_ACK)
    {
        return -1;
    }

    //This packet is sent out and received successfully by the M4 bootloader
    return 0;

}

static int Send_Command(unsigned char *DataBuffer, unsigned int Size)
{
    int Temp;

    Temp = -1;
    while(Temp == -1)
    {
        Temp = Send_Packet(DataBuffer, Size);
    }

    Temp = -1;
    while(Temp == -1)
    {
        Temp = Send_Packet(command_get_status, 1);
    }

    //Temp = 0;
    ReceiveData((unsigned char *)&Temp, 1);

    if(Temp != COMMAND_RET_SUCCESS)
    {
        return -1;
    }

    // This command is executed successfully
    return 0;
}
int main(void)
{
    int Temp_Che;
    int File_fd;
    unsigned int n;
    unsigned char *Buf;
    SetforI2C();
    GPIOInit();
    //PB3 = 0; force M4 to Update.when done ,it need to change
    //PB4 = 0; Rest M4;
    //PB4 = 1; before this step,need think whether there is a delay(usleep)

    Program_Size = Get_file_size(File_Path); 
    if(Program_Size == -1)
    {
        printf("Get_file_size failed\n");
        //need other handle
    }

    Buf = (unsigned char *)malloc(Program_Size);

    Command_Init();
    
    if((File_fd = open(File_Path, O_RDONLY)) < 0 )
    {
        printf("file open failed\n");
    }
    if(read(File_fd, Buf, Program_Size) < 0)
    {
        printf("file read failed\n");
    }

    Temp_Che = -1;
    while(Temp_Che == -1 )
    {
        Temp_Che = Send_Command(command_ping, 1);
    }

    Temp_Che = -1;
    while(Temp_Che == -1)
    {
        Temp_Che = Send_Command(command_download, 9);
    }

    n = Program_Size / 100;
    while(n--)
    {
        strncpy(command_send_data + 1, Buf, 100);
        Temp_Che = -1;
        while(Temp_Che == -1)
        {
            Temp_Che = Send_Command(command_send_data, 101);
        }

        Buf = Buf + 100;
    }
    
    Temp_Che = -1;
    strncpy(command_send_data + 1, Buf + n * 100, Program_Size - n * 100);
    while(Temp_Che == -1)
    {
        Temp_Che = Send_Command(command_send_data, (Program_Size - n * 100) + 1);
    }


    free(Buf);
    close(File_fd);
    return 0;
}




