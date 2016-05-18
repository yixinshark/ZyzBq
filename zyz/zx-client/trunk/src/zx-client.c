/*************************************************************************
> File Name: zx-client.c
> Author: 
> Mail: 
> Created Time: 2015年02月06日 星期五 10时11分04秒
************************************************************************/

#include <poll.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/sockios.h>
#include "copt_connect.c"
#include "zxapp.h"

static session_t *session_table = NULL;
static int session_num;

static tea_result_t init(void)
{
    int32_t r;

    r = query_inst_num("zxapp",&session_num);
    CHECK_RESULT(r);

    session_table = calloc(session_num, sizeof(session_t));
    CHECK_POINTER(session_table);

    return 0;

}

static tea_result_t fini(void)
{
    SAFE_FREE(session_table);

    return 0;
}

static tea_result_t create(struct N_node* nn)
{
    xT_update_int_2(nn, "state", 1);
    return 0;
}
 
 /* send a specified length(size_t len) packet_data */
static int send_data(unsigned char buf[], size_t len, session_t *session)
{
    size_t nleft = len;
    ssize_t nsend;

    while (nleft > 0)
    {
        if ((nsend = send(session->clientfd, buf, nleft,0)) <= 0 )
        {
            if(errno == EINTR)
                continue;
            return -1;
        }
        buf += nsend;
        nleft -= nsend;
    }

    return 0;
}
/**recv a specified length(int count) packet_data */
static int recv_data(const void * buffer, int count, session_t *session)
{
    size_t nleft = count;
    ssize_t nrecv;
    unsigned char *buf=(unsigned char *)buffer;
    int r;

    while(nleft > 0)
    {
        // wait for events on the sockets, 10 second timeout
        r = poll(&session->pfd, 1, 10000);
        if(r>0)
        {
            if((nrecv = recv(session->clientfd, buf, nleft, 0)) <= 0)
            {
                if(errno == EINTR)
                    continue;
                return -1;
            }

            buf += nrecv;
            nleft -= nrecv;
        }
        else
        {
            return -1;
        }
    }

    return 0;
}

/**handle packet_data recved from server */
static int handle_data(unsigned char buf[], int len, struct N_node* nn)
{
    session_t *session = session_table + NN_inst_id(nn) - 1;
    int res = 0;
    int udp_port = 0;
    unsigned int audioAlgo = 0; //int i = 0;

    if(len == (0x1f) && buf[17] == 0x03) //recv_data  firstly recved 4-bytes
    {
        //udp_port = 3000 + buf[18]*2; // 赵应振：buf[18] 是分机的编号。
        udp_port = *(uint16_t *)(&buf[21]);
        xT_set_int_2(nn, "server/port/udp", udp_port); //write udp_port to file xml.m/port/udp 
        if(buf[26] == 0x01)
        {
            audioAlgo = 7;
            xT_set_int_2(nn, "server/audioAlgo", audioAlgo); //write audioAlgo to file xml.m 
        }
        else if(buf[26] == 0x0)
        {
            audioAlgo = 5;
            xT_set_int_2(nn, "server/audioAlgo", audioAlgo); //write audioAlgo to file xml.m
        }
        xT_update_int_2(nn, "state", 3);
        Debug("udp_port=%d\n",udp_port);
    }
    else if(len == (0x16))
    {
        /***
        * send_ctrl1:
        *   when firstly recv a packet_data, which length is (0x16+4),
        *   we need send  a data .if recv a same length packet_data again,
        *   we need do nothing.
        *   the same as send_ctrl2!
        *   ***/
        if(session->send_ctrl1 == 0)                     
        {
            res = send_data(peer0_1, 26, session);
            if(res == -1)
            {
                return -1;
            }
          //  session-> send_ctrl1 = 1;
       // }
      //  else if(buf[len-1] == 0x89)
       // {
            res = send_data(peer0_2, 26, session);
            if(res == -1)
            {
                return -1;
            }
            usleep(40000);// 0.04s = 40000us
            res = send_data(peer0_3, 26, session);
            if(res == -1)
            {
                return -1;
            }
    
            session-> send_ctrl1 = 1;
        }
    }
    else if(len == (0x15))
    {
        /***
        * Heartbeat Response of the packet_data sent by server,
        * need Response every time when recved the length(0x15+4) packet_data
        * ***/
        res = send_data(peer0_4, 25, session);
        if(res == -1)
        {
            return -1;
        }
    }
    else if(len == (0x1b) && buf[21-4] == 0x02) // suggest by xiaofeng.wang
    {
       res = send_data(peer_connect0_29, 26, session);
       if(res == -1)
       {
           return -1;
       }
        /*
       //printf("HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHi\n");
       for(i = 0; i < 26; i++)
       {        
           printf("%02x",peer_connect0_29[i]);
           printf(":");
       }*/

    }
    else if( len == (0x1c) || len == 0x1b )
    {
        if(session->send_ctrl2 == 0)
        {
            res = send_data(peer0_5, 26, session);
            if(res == -1)
            {
                return -1;
            }
            session-> send_ctrl2 = 1;
        }
    }

    return 0;
}

static int done_first(session_t *session)
{
    struct sockaddr_in s_addr;
    int res;
    //peer0_0[22] ~peer0_0[26]
    sprintf((char *)peer0_0+22, "%4d%s", session->username, (char *)peer0_0+26);
    bzero(&s_addr,sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(session->tcp_port);
    s_addr.sin_addr.s_addr = inet_addr(session->ip);

    #if 1
    char *name = "eth0";
    int localip;
    struct ifreq ifr;
    struct sockaddr_in *adr;

    strcpy(ifr.ifr_name, name);
    ioctl(session->clientfd, SIOCGIFADDR, &ifr);
    adr = (struct sockaddr_in *)&(ifr.ifr_addr);
    localip= (adr->sin_addr.s_addr);
    *(int *)(peer0_0 + 42) = localip;

    *(int *)(peer0_0 + 46) = s_addr.sin_addr.s_addr;

    #endif

    res = connect(session->clientfd, (struct sockaddr *)&s_addr, sizeof(s_addr));
    if(res == 0)
    {
        res = send_data(peer0_0, 56, session);
        if(res == -1)
        {
            return res; // failed to send
        }
    }
    else
    {
        return -1;
    } 

    return 0;
}

static tea_result_t tsk_init(worker_t* worker) 
{
//  init_____
    session_t *session = session_table + NN_inst_id(worker->nn_inst) - 1;
    int r;
    session->clientfd = 0;
    session->send_ctrl1 = 0;
    session->send_ctrl2 = 0;
    session->connect_ctrl =0;
    session->tcp_port = 0;
    session->username = 0;
    bzero(&(session->ip), 16);

    r = xT_read_copy_2(worker->nn_inst, "server/addr",session->ip, 16);
    TEST_RESULT(r);
    
    r = xT_read_uint_2(worker->nn_inst, "server/port/tcp", &(session->tcp_port));
    TEST_RESULT(r);

    r = xT_read_uint_2(worker->nn_inst, "client/username", &(session->username));
    TEST_RESULT(r);

    //create socket
    session->clientfd = socket (AF_INET, SOCK_STREAM, 0);
    ASSERT(session->clientfd >= 0);

    session->pfd.fd = session->clientfd;
    session->pfd.events |= POLLIN;

    tea_task_setopt(worker, task_opt_enable_kill, (void*) TRUE);

    return 0;

}

static tea_result_t tsk_repeat(worker_t* worker)
{
    session_t *session = session_table + NN_inst_id(worker->nn_inst) - 1;
    int i, n;
    int ret = 0;
    struct packet{
        int len;
        unsigned char buf[N];    
    };
    struct packet recvbuf;

    if(session->connect_ctrl == 0)
    {
        xT_update_int_2(worker->nn_inst, "state", 2);
        ret = done_first(session); //_____do connect

        if(ret == -1)
            return 0;

        session->connect_ctrl = 1;
    }

    /*
     * *recv 4-bytes data to buf firstly,
    * * thus we can get the length of packet_data ,which need to be received
    * */
    ret = recv_data(&recvbuf.len, 4, session);
    if(ret == -1)
    {
        perror("recv\n");   //____ no stream_data or server has closed down
        return TEA_RSLT_RESTART_TASK; 
    }

    n = recvbuf.len; //the length of packet_data to be received
    //printf("n=%d\n",n);

    //______debug________if(n >= 1024)______
    if(n >= 1024)
    {
        ASSERT(0);
        return 0;
    }

    ret = recv_data(recvbuf.buf, n, session);
    if(ret == -1)
    {
        perror("recv\n");
        return TEA_RSLT_RESTART_TASK;
    }
    /** printf content of recv_buffer*/
    //printf("%x:00:00:00:",n);
    for(i = 0; i < n; i++)
    {
        //printf("%02x",recvbuf.buf[i]);
        //printf(":");
    }
    //putchar(10);// "\n"

    ret = handle_data(recvbuf.buf, n, worker->nn_inst);
    if(ret == -1)
    {
        session->connect_ctrl = 0;  
        //send failed ______do function of done_first (connet) again
    }

    return 0;
}

static tea_result_t tsk_cleanup(worker_t* worker)
{
    session_t *session = session_table + NN_inst_id(worker->nn_inst) - 1;
    
    close(session->clientfd);

    xT_update_int_2(worker->nn_inst, "state", 1);

    SLEEP(1,0);

    return 0;
}

static tea_result_t tsk_repeat0(worker_t* worker)
{
    return  TEA_RSLT_COMPLETE;
}
static task_func_t repeat_table[] = {tsk_repeat0,tsk_repeat, NULL};

static struct task_logic logic =
{
    init:
    tsk_init,
    repeat:
    repeat_table,
    cleanup:
    tsk_cleanup
};

tea_app_t zxapp =
{
    version:TEA_VERSION(0,5),
    init:init,
    fini:fini,
    create:create,
    task:&logic
};
