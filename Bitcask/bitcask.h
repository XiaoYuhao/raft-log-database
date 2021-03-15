#ifndef _BITCASK_H
#define _BITCASK_H

#include <unordered_map>
#include <fstream>
#include <string>

class Bitcask{
    //std::fstream file;
    static Bitcask* _instance;
    std::ifstream infile;
    std::ofstream outfile;
    std::ifstream old_seg_infile;
    Bitcask();

    std::unordered_map<std::string, long> htable;
    std::unordered_map<std::string, long> old_htable;

    void compress_merge();

    static const int MAX_LOG_NUM = 500;
    int log_num;

public:
    static Bitcask* create();
    void db_set(std::string key, std::string val);
    std::string db_get(std::string key);
    ~Bitcask();
};

#endif