#include<iostream>
#include<cstring>
#include<string>
#include<fstream>
using namespace std;

void db_set(string key, string val){
    ofstream outfile;
    outfile.open("data.db", ios::out|ios::app);
    outfile<<key<<" "<<val<<endl;
    outfile.close();
    return;
}

string db_get(string key){
    ifstream infile;
    infile.open("data.db", ios::in);
    string k, v, res;
    while(!infile.eof()){
        infile>>k>>v;
        if(k==key) res=v;
    }
    infile.close();
    return res;
}

int main(int argc, char *argv[]){
    if(argc<=1){
        cout<<"please input correct params!"<<endl;
        return 0;
    }
    if(!strcmp(argv[1], "set")&&argc==4){
        db_set(string(argv[2]), string(argv[3]));
        return 0;
    }
    else if(!strcmp(argv[1], "get")&&argc==3){
        string val = db_get(string(argv[2]));
        cout<<val<<endl;
        return 0;
    }
    else{
        cout<<"please input correct params!"<<endl;
        return 0;
    }
}