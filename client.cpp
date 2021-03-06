#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <assert.h>  
#include <stdio.h>  
#include <unistd.h>  
#include <errno.h>  
#include <string.h>  
#include <string>
#include <map>
#include <fcntl.h>  
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <random>
#include "package.h"  


int connect_to_server(){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(11234);
    server.sin_addr.s_addr = inet_addr("0.0.0.0");
    socklen_t len = sizeof(sockaddr_in);

    int ret = 0;
    ret = connect(sockfd, (sockaddr*)&server, len);
    if(ret<0){
        printf("connect server failed...\n");
        return -1;
    }
    return sockfd;
}



void db_set(const char *key, const char *val){
    int fd = connect_to_server();
    db_set_package dsp(key, val);
    int ret = 0;
    ret = send(fd, (void*)&dsp, ntohs(dsp.header.package_length), 0);
    //sleep(1);
    db_set_res_package dsrp;
    ret = recv(fd, (void*)&dsrp, sizeof(dsrp), MSG_WAITALL);
    close(fd);
    return ;
}

void db_get(const char *key){
    int fd = connect_to_server();
    db_get_package dgp(key);
    int ret = 0;
    ret = send(fd, (void*)&dgp, ntohs(dgp.header.package_length), 0);
    //sleep(1);
    db_get_res_package dgrp;
    package_header header;
    ret = recv(fd, (void*)&header, sizeof(header), MSG_PEEK);
    ret = recv(fd, (void*)&dgrp, ntohs(header.package_length), MSG_DONTWAIT);
    printf("%s\n", dgrp.buf);
    close(fd);
    return ; 
}

void create_data(){
    int num = 1000*100;
    char key[11];
    char val[11];
    key[10] = '\0';
    val[10] = '\0';
    srand(time(NULL));
    for(int i=0;i<num;i++){
        for(int i=0;i<10;i++){
            key[i] = rand()%26 + 'a';
            val[i] = rand()%10 + '0';
        }
        db_set(key, val);
    }
}

long getTimeUsec(){
    struct timeval t;
    gettimeofday(&t, 0);
    return (long)((long)t.tv_sec * 1000 * 1000 + t.tv_usec);
}
/*
int main(int argc, char *argv[]){
    if(argc<=1){
        printf("please input correct params!\n");
        return 0;
    }
    long start_time, end_time;
    if(!strcmp(argv[1], "set")&&argc==4){
        start_time = getTimeUsec();
        db_set(argv[2], argv[3]);
        end_time = getTimeUsec();
        printf("reponse time : %dms\n", (end_time-start_time)/1000);
        return 0;
    }
    else if(!strcmp(argv[1], "get")&&argc==3){
        start_time = getTimeUsec();
        db_get(argv[2]);
        end_time = getTimeUsec();
        printf("reponse time : %dms\n", (end_time-start_time)/1000);
        return 0;
    }
    else{
        printf("please input correct params!\n");
        return 0;
    }
    //create_data();
    return 0;
}*/

int main(){
    int n;
    char op[16];
    char key[64];
    char val[64];
    scanf("%d", &n);
    for(int i=0;i<n;i++){
        scanf("%s", op);
        if(!strcmp("set", op)){
            scanf("%s %s", key, val);
            db_set(key, val);
        }
        if(!strcmp("get", op)){
            scanf("%s", key);
            db_get(key);
        }
    }
    return 0;
}