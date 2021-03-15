#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <assert.h>  
#include <stdio.h>  
#include <unistd.h>  
#include <errno.h>  
#include <string.h>  
#include <fcntl.h>  
#include <stdlib.h>  
#include <sys/epoll.h>  
#include <signal.h>  
#include <sys/wait.h>  
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>

#include "common/network.h"
#include "common/package.h"

#include "Bitcask/bitcask.h"
#include "SSTable/sstable.h"

long getTimeUsec(){
    struct timeval t;
    gettimeofday(&t, 0);
    return (long)((long)t.tv_sec * 1000 * 1000 + t.tv_usec);
}

void work(){
    int listen_sock = startup(11234);
    struct sockaddr_in remote;
    socklen_t len = sizeof(struct sockaddr_in);

    int epfd = epoll_create(1024);
    struct epoll_event events[1024];

    addfd(epfd, listen_sock);

    int ret = 0;

    //Bitcask *db = Bitcask::create();
    SSTable *db = SSTable::create();

    for(;;){
        int nfds = epoll_wait(epfd, events, 1024, -1);
        if((nfds<0)&&(errno!=EINTR)){
            printf("epoll failure.\n");
            break;
        }
        for(int i=0;i<nfds;i++){
            int sockfd = events[i].data.fd;
            if(sockfd == listen_sock){
                int client_sock = accept(listen_sock, (struct sockaddr*)&remote, &len);
                addfd(epfd, client_sock);
            }
            else if(events[i].events&EPOLLIN){
                package_header header;
                ret = recv(sockfd, (void*)&header, sizeof(header), MSG_PEEK);
                if((ret<0)&&(errno!=EINTR)){
                    continue;
                }
                if(ret<=0){
                    //printf("Client has off line.\n");
                    continue;
                }
                if(header.package_type == DB_GET){
                    db_get_package dgp;
                    ret = recv(sockfd, (void*)&dgp, ntohs(header.package_length), MSG_DONTWAIT);
                    //printf("Database Server recv Get request. Key = %s\n", dgp.buf);
                    long start_time = getTimeUsec();
                    std::string val = db->db_get(dgp.buf);
                    long end_time = getTimeUsec();
                    db_get_res_package dgrp(val.c_str());
                    //printf("Val = %s Cost time : %dms\n", dgrp.buf, (end_time-start_time)/1000);
                    ret = send(sockfd, (void*)&dgrp, ntohs(dgrp.header.package_length), 0);
                    removefd(epfd, sockfd);
                    close(sockfd);
                }
                else if(header.package_type == DB_SET){
                    db_set_package dsp;
                    ret = recv(sockfd, (void*)&dsp, ntohs(header.package_length), MSG_DONTWAIT);
                    //printf("Database Server recv Set request. Key = %s Val = %s\n", dsp.buf, dsp.buf+dsp.key_len);
                    long start_time = getTimeUsec();
                    db->db_set(dsp.buf, dsp.buf+ntohl(dsp.key_len));
                    long end_time = getTimeUsec();
                    //printf("Cost time : %dms\n", (end_time-start_time)/1000);
                    db_set_res_package dsrp(1);
                    ret = send(sockfd, (void*)&dsrp, ntohs(dsrp.header.package_length), 0);
                    removefd(epfd, sockfd);
                    close(sockfd);
                }
            }
        }
    }

}

int main(){
    work();
    return 0;
}