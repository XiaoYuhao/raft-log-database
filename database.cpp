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
#include <sys/epoll.h>  
#include <signal.h>  
#include <sys/wait.h>  
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <unordered_map>

#include"network.h"
#include"package.h"

class DataBase{
    //std::fstream file;
    static DataBase* _instance;
    std::ifstream infile;
    std::ofstream outfile;
    std::ifstream old_seg_infile;
    DataBase();

    std::unordered_map<std::string, long> htable;
    std::unordered_map<std::string, long> old_htable;

    void compress_merge();

    static const int MAX_LOG_NUM = 500;
    int log_num;

public:
    static DataBase* create();
    void db_set(std::string key, std::string val);
    std::string db_get(std::string key);
    ~DataBase();
};

DataBase::DataBase(){
    outfile.open("data.db", std::ios::out|std::ios::app);
    infile.open("data.db", std::ios::in);
    old_seg_infile.open("old_c.db", std::ios::in);
    log_num = 0;
}

DataBase* DataBase::create(){
    if(_instance == NULL){
        _instance = new DataBase();
    }
    return _instance;
}

DataBase::~DataBase(){
    infile.close();
    outfile.close();
    old_seg_infile.close();
}

void DataBase::db_set(std::string key, std::string val){
    htable[key] = outfile.tellp();
    outfile<<key<<" "<<val<<std::endl;
    log_num++;
    if(log_num >= MAX_LOG_NUM){
        infile.close();
        old_seg_infile.close();
        outfile.close(); //关闭原文件
        rename("data.db", "old.db"); //修改源文件名
        compress_merge(); //压缩与合并数据文件
        outfile.open("data.db", std::ios::out|std::ios::app); //重新创建数据文件
        infile.open("data.db", std::ios::in);
        old_seg_infile.open("old_c.db", std::ios::in);
        log_num = 0;
        htable.clear();
    }
}

std::string DataBase::db_get(std::string key){
    std::string k, v, res;
    res = "None";
    if(htable.count(key)==0){
        if(old_htable.count(key)==0) return res;
        printf("Get in old data seg. Peek = %d\n", old_htable[key]);
        old_seg_infile.clear();
        old_seg_infile.seekg(old_htable[key]);
        old_seg_infile>>k>>v;
        return v;
    }
    else{
        printf("Get in new data seg. Peek = %d\n", htable[key]);
        infile.clear();
        infile.seekg(htable[key]);
        infile>>k>>v;
        return v;
    }
    /*
    infile.clear();
    infile.seekg(0);
    while(!infile.eof()){
        infile>>k>>v;
        if(k==key) res=v;
    } 
    return res;
    */
}

void DataBase::compress_merge(){
    std::ifstream infile_old;
    infile_old.open("old.db", std::ios::in);
    std::ifstream infile_old_c;
    infile_old_c.open("old_c.db", std::ios::in);
    std::ofstream outfile_old;
    outfile_old.open("tmp.db", std::ios::out|std::ios::app);

    //std::unordered_map<std::string, long> oldc_htable;
    //std::unordered_map<std::string, long> tmp_htable;
    std::unordered_map<std::string, long> oldc_htable;

    std::string k, v;
    /*while(!infile_old_c.eof()){
        long p = infile_old_c.tellg();
        infile_old_c>>k>>v;
        oldc_htable[k] = p;
    }*/
    /*while(!infile_old.eof()){
        long p = infile_old.tellg();
        infile_old>>k>>v;
        tmp_htable[k] = p;
    }
    for(auto t : tmp_htable){
        std::cout<<t.first<<" "<<t.second<<std::endl;
        infile_old.clear();
        infile_old.seekg(t.second);
        infile_old>>k>>v;
        std::cout<<k<<" "<<v<<std::endl;
    }*/
    std::cout<<std::endl;
    for(auto t : old_htable){
        if(htable.count(t.first)) continue;
        infile_old_c.clear();
        infile_old_c.seekg(t.second);
        infile_old_c>>k>>v;
        oldc_htable[k] = outfile_old.tellp();
        outfile_old<<k<<" "<<v<<std::endl;
    }
    for(auto t : htable){
        infile_old.clear();
        infile_old.seekg(t.second);
        std::cout<<t.first<<" "<<t.second<<std::endl;
        infile_old>>k>>v;
        std::cout<<k<<" "<<v<<std::endl;
        oldc_htable[k] = outfile_old.tellp();
        outfile_old<<k<<" "<<v<<std::endl;
    }

    std::cout<<std::endl;
    for(auto t : oldc_htable){
        std::cout<<t.first<<" "<<t.second<<std::endl;
    }

    infile_old.close();
    infile_old_c.close();
    outfile_old.close();
    old_htable = oldc_htable;
    remove("old.db");
    remove("old_c.db");
    rename("tmp.db", "old_c.db");


}

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

    DataBase *db = DataBase::create();

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
                    printf("Database Server recv Get request. Key = %s\n", dgp.buf);
                    long start_time = getTimeUsec();
                    std::string val = db->db_get(dgp.buf);
                    long end_time = getTimeUsec();
                    db_get_res_package dgrp(val.c_str());
                    printf("Val = %s Cost time : %dms\n", dgrp.buf, (end_time-start_time)/1000);
                    ret = send(sockfd, (void*)&dgrp, ntohs(dgrp.header.package_length), 0);
                    removefd(epfd, sockfd);
                    close(sockfd);
                }
                else if(header.package_type == DB_SET){
                    db_set_package dsp;
                    ret = recv(sockfd, (void*)&dsp, ntohs(header.package_length), MSG_DONTWAIT);
                    printf("Database Server recv Set request. Key = %s Val = %s\n", dsp.buf, dsp.buf+dsp.key_len);
                    long start_time = getTimeUsec();
                    db->db_set(dsp.buf, dsp.buf+dsp.key_len);
                    long end_time = getTimeUsec();
                    printf("Cost time : %dms\n", (end_time-start_time)/1000);
                    db_set_res_package dsrp(1);
                    ret = send(sockfd, (void*)&dsrp, ntohs(dsrp.header.package_length), 0);
                    removefd(epfd, sockfd);
                    close(sockfd);
                }
            }
        }
    }

}

DataBase* DataBase::_instance = NULL;


void create_data(){
    DataBase *db = DataBase::create();
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
        db->db_set(key, val);
    }
}

int main(){
    //create_data();
    work();
}