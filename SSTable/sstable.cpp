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

#include "sstable.h"

SSTable::SSTable(){
    infile_old.open("old_c.db", std::ios::in);
}

SSTable::~SSTable(){
    infile_old.close();
}

SSTable* SSTable::_instance = NULL;

SSTable* SSTable::create(){
    if(_instance==NULL){
        _instance = new SSTable();
    }
    return _instance;
}


void SSTable::db_set(std::string key, std::string val){
    if(sstable.size()>=MAX_NUM){
        compress_merge();
    }
    sstable[key] = val;
}

std::string SSTable::db_get(std::string key){
    iter = sstable.find(key);
    if(iter!=sstable.end()){
        return iter->second;
    }
    else{
        long p;
        std::string k, v;
        std::map<std::string, long>::iterator it;
        it = old_sstable.lower_bound(key);
        if(it->first==key){
            p = it->second;
        }
        else if(it==old_sstable.begin()){
            return "None";
        }
        else{
            it--;
            p = it->second;
        }
        //std::cout<<"Index : "<<it->first<<" Peek : "<<p<<std::endl;
        infile_old.clear();
        infile_old.seekg(p);
        while(!infile_old.eof()){
            infile_old>>k>>v;
            if(k==key) return v;
        }
        return "None";
    }
}


void SSTable::compress_merge(){
    //int _a;
    //scanf("%d", &_a);
    infile_old.close();
    std::ifstream infile;
    infile.open("old_c.db", std::ios::in);
    std::ofstream outfile;
    outfile.open("data.db", std::ios::out|std::ios::app);
    old_sstable.clear();
    
    std::string key, val;
    int count = 0;
    long p = 0;
    iter = sstable.begin();
    infile.peek();
    if(!infile.eof()) infile>>key>>val;
    while(1){
        /*if(key=="zzyfypkjre"){
            std::cout<<"old"<<std::endl;
        }
        if(iter->first=="zzyfypkjre"){
            std::cout<<"new"<<std::endl;
        }*/
        if(key<iter->first){
            //std::cout<<key<<" "<<val<<std::endl;
            outfile<<key<<" "<<val<<std::endl;
            if(count%SPARSE_INTERVAL==0){
                old_sstable[key] = p;
            }
            infile>>key>>val;
        }
        else{
            //std::cout<<iter->first<<" "<<iter->second<<std::endl;
            outfile<<iter->first<<" "<<iter->second<<std::endl;
            if(count%SPARSE_INTERVAL==0){
                old_sstable[iter->first] = p;
            }
            if(key==iter->first){
                //if(infile.eof())break;
                infile>>key>>val;
            }
            iter++;
        }
        count++;

        if(infile.eof()) break;
        if(iter==sstable.end()) break;
    }
    
    while(!infile.eof()){
        p = outfile.tellp();
        //std::cout<<key<<" "<<val<<std::endl;
        outfile<<key<<" "<<val<<std::endl;
        if(count%SPARSE_INTERVAL==0){
            old_sstable[key] = p;
        }
        count++;
        infile>>key>>val;
    }

    while(iter!=sstable.end()){
        p = outfile.tellp();
        //std::cout<<iter->first<<" "<<iter->second<<std::endl;
        outfile<<iter->first<<" "<<iter->second<<std::endl;
        if(count%SPARSE_INTERVAL==0){
            old_sstable[iter->first] = p;
        }
        count++;
        iter++;
    }

    infile.close();
    outfile.close();
    sstable.clear();
    remove("old_c.db"); 
    rename("data.db", "old_c.db");
    infile_old.open("old_c.db", std::ios::in);
}