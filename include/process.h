#ifndef PROCESSPOOL_H
#define PROCESSPOOL_H

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


static int sig_pipefd[2];

static int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

static void addfd(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

static void removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

static void sig_handler(int signum)
{
    int save_errno = errno;
    int msg = signum;
    if (send(sig_pipefd[1], (char *)&msg, 1, 0) != 1)
        err_msg("in sig_handler send! errno=[%d]", errno);
    errno = save_errno;
}

static void addsig(int signum, void(hanlder)(int), bool restart = true)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = hanlder;
    if (restart == true)
    {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask); //????
    if (sigaction(signum, &sa, NULL) < 0)
    {
        err_sys("in addsig sigaction err errno=[%d]", errno);
    }
}

// class sub process
class process
{
  public:
    process() : m_pid(-1) {}

  public:
    pid_t m_pid;     // subprocess pid
    int m_pipefd[2]; // pipe
};


// processpool
template <typename T>
class processpool
{
  private:
    processpool(int listenfd, int process_number = 8);

  public:
    static processpool<T> *GetInstance(int listenfd, int process_num)
    {
        if (!m_instance)
        {
            m_instance = new processpool<T>(listenfd, process_num);
        }
        return m_instance;
    }
    ~processpool()
    {
        delete m_instance;
        delete[] m_sub_process;
    }
    void run();

  private:
    void set_sig_pipe();
    void run_parent();
    void run_child();

  private:
    static const int MAX_PROCESS_NUMBER = 16;
    static const int USER_PER_PROCESS = 65536;
    static const int MAX_EVENT_NUMBER = 10000;
    int m_process_number; // process total number in pool
    int m_idx;            // serial numer in pool
    int m_epollfd;
    int m_listenfd;
    bool m_stop;
    process *m_sub_process;
    static processpool<T> *m_instance;
};

// private constructor of processpool
template <typename T>
processpool<T> ::processpool(int listenfd, int proc_num)
    : m_listenfd(listenfd), m_process_number(proc_num), m_idx(-1),
      m_stop(false)
{
    assert((m_process_number > 0) && (m_process_number < MAX_PROCESS_NUMBER));

    m_sub_process = new process[m_process_number];
    assert(m_sub_process != NULL);

    for (int i = 0; i < m_process_number; ++i)
    {        
        int ret = socketpair(PF_UNIX,SOCK_STREAM,0,m_sub_process[i].m_pipefd);
        if(ret<0)
            err_sys("constructor processpool::socketpair() err");
        m_sub_process[i].m_pid = fork();
        if(m_sub_process[i].m_pid < 0)
            err_sys("constructor processpool::fork() err");

        if( m_sub_process[i].m_pid > 0 )//parent
        {
            close(m_sub_process[i].m_pipefd[1]);
            continue;
        }
        else
        {
            close(m_sub_process[i].m_pipefd[0]);
            m_idx = i;
            break;
        }       
                
    }
}


template <typename T>
void processpool<T>::set_sig_pipe()
{
    
    m_epollfd = epoll_create(5);
    if( m_epollfd < 0  )
        err_sys("in set_sig_pipe::epoll_create() err");
    int ret = socketpair(PF_UNIX,SOCK_STREAM , 0 , sig_pipefd);
    if(  ret < 0 )
        err_sys("in set_sig_pipe::sockertpair() err");
    setnonblocking(sig_pipefd[1]);
    addfd( m_epollfd , sig_pipefd[0] );

    addsig( SIGCHLD  , sig_handler);
    addsig( SIGTERM  , sig_handler);
    addsig( SIGINT  , sig_handler);
    addsig( SIGPIPE  , SIG_IGN);
}


template<typename T>
void processpool<T>::run()
{
    if(m_idx >=0 )
    {
        run_child();
        return;
    }
    run_parent();
}


template<typename T>
void processpool<T>::run_parent()
{
    set_sig_pipe();
    addfd( m_epollfd , m_listenfd );

    epoll_event events[MAX_EVENT_NUMBER];
    int sub_process_counter = 0;
    char new_conn = 1;
    int number = 0;
    int ret = -1;

    while(!m_stop)
    {
        number = epoll_wait(m_epollfd,events,MAX_EVENT_NUMBER,-1); //timeout -1  block forever
        if( (number<0) && (errno!=EINTR)   )
        {
            err_sys("in processpool epoll_wait err" );                
        }

        for(int i=0;i<number;i++)
        {
            int sockfd = events[i].data.fd;

            if( sockfd == m_listenfd )
            {
                int i = sub_process_counter;
                do{
                    if(m_sub_process[i].pid != -1)
                        break;
                    i = (i+1)%m_process_number;
                }
                while( i!= sub_process_counter);

                if( m_sub_process[i].m_pid == -1)
                {
                    m_stop = true;
                    break;
                }
                sub_process_counter = (i+1)%m_process_number;
                //sub_process_counter = i;
                send( m_sub_process[i].m_pipefd[0] , &new_conn ,sizeof(new_conn) );
                

            }
        }

    }


}


#endif