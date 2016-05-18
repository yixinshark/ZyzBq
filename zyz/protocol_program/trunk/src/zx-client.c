#include <tea/tea.h>
#include "packet1.c"
#define N 1024

int clientfd=0;
int flag=0,flag1=0;
int udp_port=0;

void send_data(unsigned char buf[], int len)
{
    if(-1 == send(clientfd,buf,len,0))
    {
        perror("send\n");
        exit(-1);
    }
}

void equal_len(unsigned char buf[], int len, struct N_node* nn)
{

    if(len == (0x1f+4) && buf[21] == 0x03)
    {
        udp_port == 3000 + buf[22]*2;
        xT_set_2( nn, "udp_port", &udp_port);
    }
    else if(len == (0x16+4))
    {
        if(flag == 0)
        {
            send_data(peer0_1, 26);
            flag = 1;
        }
        else if(buf[len-1] == 0x89)
        {
            send_data(peer0_2, 26);
            sleep(0.04);
            send_data(peer0_3, 26);
        }
    }
    else if(len == (0x15+4))
    {
        send_data(peer0_4, 25);
    }
    else if(len == (0x1b+4))
    {
        if(flag1 == 0)
        {
            send_data(peer0_5, 26);
            flag1 = 1;
        }
    }
}

int great_than_len(unsigned char buf[], int nbytes, int len, struct N_node* nn_inst)
{
    int sec_len = *(buf+len) + 4;
    int n = nbytes - len;
    equal_len(buf, len, nn_inst);

    if(n < sec_len)
    {
        return n;
    }
    else if(n == sec_len)
    {
        equal_len(buf+len, n, nn_inst);
        return 0;
    }
    else if(n > sec_len)
    {
        great_than_len(buf+len, n, sec_len,nn_inst);
    }
}

static tea_result_t zxapp_init(void)
{
    return 0;
}

static tea_result_t zxapp_fini(void)
{
    return 0;
}
static tea_result_t tsk_init(worker_t* worker)
{
    int i=0,k=0,n=0;
    int username_fir,username_sec,username_thi,username_for;
    int nbytes=0 ,len=0;
    int gt_length=0;
    unsigned char buf[N] = {0};
    unsigned char buffer[N]={0};
    char *ip;
    int tcp_port=0;
    int username=0;
    struct sockaddr_in s_addr;

    r = xT_read_nolock_2(worker->nn_inst, "ip", &ip);
    CHECK_RESULT(r);
    NN_LOAD_INT_FAR_Q(worker->nn_inst, tcp_port);
    NN_LOAD_INT_FAR_Q(worker->nn_inst, username);

    username_fir = username / 1000;
    peer0_0[22] = username_fir + 0x30;
    username_sec = (username % 1000)/100;
    peer0_0[23] = username_sec + 0x30;
    username_thi = ((username % 1000) % 100) / 10;
    peer0_0[24] = username_thi + 0x30;
    username_for = ((username % 1000) % 100) % 10;
    peer0_0[25] = username_for + 0x30;

    bzero(&s_addr,sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(tcp_port);
    s_addr.sin_addr.s_addr = inet_addr(ip);

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

    return 0;
}

static tea_result_t tsk_repeat(worker_t* worker)
{
    /*  client :recv  */
    if((nbytes = recv(clientfd,buffer,N,0))==-1)
    {
        perror("recv\n");
        exit(-1);
    } 
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
    /**
    *  printf  
    for(i=0;i<nbytes;i++)
    {
    printf("%02x",buf[i]);
    printf(":");
    }
    putchar(10);
    printf("n=%d\n",n);
    */
    if(nbytes <= 4 || nbytes < len)
    {    // strncpy((buf+n),buffer,nbytes-n);
         n=nbytes;
    }
    else if(nbytes == len)
    {
        equal_len(buf,len, worker->nn_inst);
    }
    else if(nbytes > len)
    {
        gt_length = great_than_len(buf,nbytes,len, worker->nn_inst);
        //   strncpy(buf, (buf+nbytes)-gt_length ,gt_length);
        for(i=0;i<gt_length;i++)
        {
            buf[i] = buf[(nbytes-gt_length)+i];
            //     printf("%02x,",buf[i]);
        }
        //   putchar(10);
        n = gt_length;
    }

    return 0;
｝
static tea_result_t tsk_cleanup(worker_t* worker)
{
    close(clientfd);
    return 0;
}

static task_func_t repeat_table[] = {tsk_repeat, NULL};

/**
* 声明任务接口。
*/
static struct task_logic logic =
{
init:
    tsk_init,
repeat:
    tsk_repeat,
cleanup:
    tsk_cleanup
};

/**
* 声明应用的接口。
*/
tea_app_t zxapp =
{
    version:TEA_VERSION(0,5),
    init:zxapp_init,
    fini:zxapp_fini,
    task:&logic
};
