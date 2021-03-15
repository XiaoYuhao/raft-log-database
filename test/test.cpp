#include<fstream>
#include<iostream>
#include<unordered_map>
#include<time.h>
#include<random>
#include<string>
using namespace std;

const int NUM = 100000;

void create_data_in(){
    vector<string> kvec;
    char key[11];
    char val[11];
    key[10] = '\0';
    val[10] = '\0';
    ofstream out;
    out.open("in4", ios::out);
    out<<NUM<<endl;
    for(int i=0;i<NUM;i++){
        int op = rand()%10;
        if(op==0){
            if(kvec.size()==0){
                i--;
                continue;
            }
            int index = rand()%kvec.size();
            out<<"get "<<kvec[index]<<endl;
        }
        else if(op<8){
            for(int i=0;i<10;i++){
                key[i] = rand()%26 + 'a';
                val[i] = rand()%10 + '0';
            }
            out<<"set "<<key<<" "<<val<<endl;
            kvec.push_back(string(key));
        }
        else{
            if(kvec.size()==0){
                i--;
                continue;
            }
            for(int i=0;i<10;i++){
                val[i] = rand()%26 + 'A';
            }
            int index = rand()%kvec.size();
            out<<"set "<<kvec[index]<<" "<<val<<endl;
        }
    }
    out.close();
}

void create_data_out(){
    unordered_map<string, string> htable;
    ifstream in;
    in.open("in4", ios::in);
    ofstream out;
    out.open("out4", ios::out);
    string op, key, val;
    while(!in.eof()){
        in>>op;
        if(op=="set"){
            in>>key>>val;
            htable[key] = val;
        }
        else if(op=="get"){
            in>>key;
            out<<htable[key]<<endl;
        }
    }
    in.close();
    out.close();
}

int main(){
    /*
    unordered_map<string, string> htable;
    vector<string> kvec;
    char key[11];
    char val[11];
    key[10] = '\0';
    val[10] = '\0';
    ofstream out;
    out.open("test_in", ios::out);
    for(int i=0;i<1000;i++){
        int op = rand()%10;
        if(op==0){
            if(kvec.size()==0){
                i--;
                continue;
            }
            int index = rand()%kvec.size();
            out<<"get "<<kvec[index]<<endl;
        }
        else{
            for(int i=0;i<10;i++){
                key[i] = rand()%26 + 'a';
                val[i] = rand()%10 + '0';
            }
            out<<"set "<<key<<" "<<val<<endl;
            htable[string(key)] = string(val);
            kvec.push_back(string(key));
        }
    }
    */
   create_data_in();
   create_data_out();
    return 0;
}