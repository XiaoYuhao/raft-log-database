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

#include "bitcask.h"

Bitcask::Bitcask(){
    outfile.open("data.db", std::ios::out|std::ios::app);
    infile.open("data.db", std::ios::in);
    old_seg_infile.open("old_c.db", std::ios::in);
    log_num = 0;
}

Bitcask* Bitcask::create(){
    if(_instance == NULL){
        _instance = new Bitcask();
    }
    return _instance;
}

Bitcask::~Bitcask(){
    infile.close();
    outfile.close();
    old_seg_infile.close();
}

void Bitcask::db_set(std::string key, std::string val){
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

std::string Bitcask::db_get(std::string key){
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

void Bitcask::compress_merge(){
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

Bitcask* Bitcask::_instance = NULL;
