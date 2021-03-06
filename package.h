#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>

const u_int8_t DB_GET = 0x01;
const u_int8_t DB_SET = 0x02;

const u_int8_t DB_GET_RES = 0x11;
const u_int8_t DB_SET_RES = 0x12;

const u_int8_t DB_SET_SUCCESS = 0x01;
const u_int8_t DB_SET_FAIL = 0x00;

struct package_header{
    u_int8_t package_type;
    u_int8_t package_seq;
    u_int16_t package_length;
};

struct db_get_package{
    package_header header;
    u_int32_t key_len;
    char buf[1024];
    db_get_package(){}
    db_get_package(const char *key){
        header.package_type = DB_GET;
        key_len = strlen(key) + 1;
        strcpy(buf, key);
        header.package_length = htons(sizeof(package_header)+4+key_len);
    }
};

struct db_get_res_package{
    package_header header;
    u_int32_t val_len;
    char buf[4096];
    db_get_res_package(){}
    db_get_res_package(const char *val){
        header.package_type = DB_GET_RES;
        val_len = strlen(val) + 1;
        strcpy(buf, val);
        header.package_length = htons(sizeof(header)+8+val_len);
    }
};

struct db_set_package{
    package_header header;
    u_int32_t key_len;
    u_int32_t val_len;
    char buf[4096];
    db_set_package(){}
    db_set_package(const char *key, const char *val){
        header.package_type = DB_SET;
        key_len = strlen(key) + 1;
        val_len = strlen(val) + 1;
        strcpy(buf, key);
        strcpy(buf+key_len, val);
        header.package_length = htons(sizeof(package_header)+8+key_len+val_len);
    }
};

struct db_set_res_package{
    package_header header;
    u_int8_t status;
    db_set_res_package(){}
    db_set_res_package(u_int8_t st){
        header.package_type = DB_SET_RES;
        header.package_length = htons(sizeof(db_set_res_package));
        status = st;
    }
};