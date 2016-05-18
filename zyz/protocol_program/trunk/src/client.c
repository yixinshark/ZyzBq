#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "packet1.c"
#define N 1024

int clientfd=0;
int flag=0,flag1=0;
int udp_port=0;

void send_data(unsigned char buf[],int len);
void eq_len(unsigned char buf[],int len);
int gt_len(unsigned char buf[],int nbytes,int len);

void send_data(unsigned char buf[],int len)
{
    if(-1 == send(clientfd,buf,len,0))
    {
        perror("send\n");
        exit(-1);
    }
}

void eq_len(unsigned char buf[],int len)
{
    if(len == (0x1f+4) && buf[21] == 0x03)
    {
        udp_port = 3000+buf[22]*2;
        printf("udp_port=%d\n",udp_port);
    }
    else if(len == (0x16+4))
    {
        if(flag == 0)
        {
            send_data(peer0_1, 26);
        //    flag = 1;
            usleep(0.04);
        }
      //  else if(buf[len-1] == 0x89)
        {
            send_data(peer0_2, 26);
            usleep(0.04);
            send_data(peer0_3, 26);
            flag = 1;
        }
    }
    else if(len == (0x15+4))
    {
        send_data(peer0_4,25);
    }
    else if(len == (0x1b+4))
    {
        if(flag1 == 0)
        {
            send_data(peer0_5,26);
            flag1 = 1;
        }
    }
}

int gt_len(unsigned char buf[],int nbytes,int len)//len 包中起始数据的长度
{
    int sec_len = *(buf+len)+4;
    int n = nbytes-len;
  //  int res=0;
 //   printf("sec_len=%d,",sec_len);
 //   printf("n=%d\n",n);
    eq_len(buf,len);

    if(n < sec_len)
    {
    //    res = n;
        return n;
    }
    else if(n == sec_len)
    {
        eq_len(buf+len,n);
        return 0;
    }
    else if(n > sec_len)
    {
        gt_len(buf+len,n,sec_len);
    }
  //  return res;
}

int main(int argc,char *argv[])
{
    int i=0,k=0,n=0;
    int username=0;
    int username_fir,username_sec,username_thi,username_for;
    int nbytes=0 ,len=0;
    int gt_length=0;
	unsigned char buf[N] = {0};
    unsigned char buffer[N]={0};

	struct sockaddr_in s_addr;
	if(3 != argc)
    {
		puts("input argc error!");
		exit(-1);
	}

    printf("Please input usrname:");
    scanf("%d",&username);
    getchar();
    username_fir = username/1000;
    peer0_0[22] = username_fir + 0x30;
    username_sec = (username%1000)/100;
    peer0_0[23] = username_sec + 0x30;
    username_thi = ((username%1000)%100)/10;
    peer0_0[24] = username_thi + 0x30;
    username_for = ((username%1000)%100)%10;
    peer0_0[25] = username_for + 0x30;
#if 0
    for(i=22;i<26;i++)
    {
        printf("%#x",peer0_0[i]);
        printf(",");
    }
    putchar(10);
#endif
	bzero(&s_addr,sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(atoi(argv[2]));
	s_addr.sin_addr.s_addr = inet_addr(argv[1]);

    //socket
	if (-1 == (clientfd = socket(AF_INET,SOCK_STREAM,0)))
	{
		perror("socket");
		exit(-1);
	}

    //connect
	if(-1 == connect(clientfd,(struct sockaddr *)&s_addr,sizeof(s_addr)))
	{
		perror("connect");
		exit(-1);
	}

    send_data(peer0_0,56);
#if 1
    while(1)
    {
        /*  client :recv  */
        if((nbytes = recv(clientfd,buffer,N,0))==-1)
        {
            perror("recv\n");
            exit(-1);
        } 
/*
        printf("--------------------------------------\n");
        printf("nbytes=%d\n",nbytes);
        for(i=0;i<nbytes;i++)
        {
            printf("%02x",buffer[i]);
            printf(":");
        }
        putchar(10);
        printf("--------------------------------------\n");
*/
        nbytes = nbytes+n;
        k = 0;
        for(i=n;i<nbytes;i++)      /** buf=buffer*/
        {
            buf[i] = buffer[k];
        //  printf("%02x:",buffer[k]);
        //  printf(":");
            k++;
        }
       

        len = buf[0]+4; //length of a stream data    

      //  nbytes = nbytes+n;

        /* printf  */
        for(i=0;i<nbytes;i++)
        {
            printf("%02x",buf[i]);
            printf(":");
        }
        putchar(10);
        printf("n=%d\n",n);

        if(nbytes <= 4 || nbytes < len)
        {    // strncpy((buf+n),buffer,nbytes-n);
            n=nbytes;
        }
        else if(nbytes == len)
        {
            eq_len(buf,len);
        }
        else if(nbytes > len)
        {
            gt_length = gt_len(buf,nbytes,len);
         //   strncpy(buf, (buf+nbytes)-gt_length ,gt_length);
            for(i=0;i<gt_length;i++)
            {
                buf[i] = buf[(nbytes-gt_length)+i];
           //     printf("%02x,",buf[i]);
            }
         //   putchar(10);
            n = gt_length;
        }
    }
#endif
	close(clientfd);

	return 0;
}

