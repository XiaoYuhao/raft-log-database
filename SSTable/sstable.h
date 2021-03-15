#include <map>
#include <string>
#include <fstream>

class SSTable{
    SSTable();
    static SSTable *_instance;
    std::map<std::string, std::string> sstable;
    std::map<std::string, std::string>::iterator iter; 
    std::map<std::string, long> old_sstable;    //保存旧数据段文件的稀疏索引
    std::ifstream infile_old;
    const static int MAX_NUM = 1000;              //排序字串表最大容量
    const static int SPARSE_INTERVAL = 100;    //稀疏哈希索引的间隔
    void compress_merge();

public:
    static SSTable* create();
    void db_set(std::string key, std::string val);
    std::string db_get(std::string key);
    ~SSTable();
};