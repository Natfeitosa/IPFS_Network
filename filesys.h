#ifndef FILESYS_H
#define FILESYS_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h> //random
#include <time.h>
#include <utility> //pair
#include "request.h"
using namespace std;

class FileSys{
    public:
        FileSys(string File,int Block_S,int Size,string Port):
            SYSTEM_FILE_NAME_{File},BLOCK_SIZE_{Block_S},TOTAL_BLOCKS_{Size},PTR_LEN_L_{getPtrLen(Size)},EMPTY_BLOCK_{emptyBlock(Block_S)},CURR_NODE_PORT_{Port}{
            init_L();
        }
        // FileSys(string File):SYSTEM_FILE_NAME_{File},BLOCK_SIZE_{8},TOTAL_BLOCKS_{500},PTR_LEN_L_{getPtrLen(500)},EMPTY_BLOCK_{"~~~~~~~~"},CURR_NODE_PORT_{NodeID},NODE_ID_{fillNodeLen(NodeID)}{}
        // ~FileSys(){
        //     fs.close();
        // }
        void createFile(string fileName,string content){
            fstream fs;
            content+=EOF_;

            fs.open(SYSTEM_FILE_NAME_);
            string table;
            getline(fs,table);
            int s=table.find('~')+1;
            table=table.substr(s);
            //skip ports
            int pos=table.find('~',s)+1;
            pos=table.find('~',s)+1;
            //skip head+tail
            if (table.find(TABLE_FMT_CHAR_+fileName+TABLE_FMT_CHAR_,pos)!=string::npos){
                cout<<"Filename Already Exists"<<endl;
                fs.close();
                return;
            }
            if(fileName.find("/")!=string::npos||fileName.find("~")!=string::npos||fileName.find(" ")!=string::npos){
                cout<<"Filename Cannot Contain '/', '~', or ' '"<<endl;
                fs.close();
                return;
            }
            // int start=/*(MEM_ALLOC_TYPE_=="contiguous")?getStartBlock(table):getHead(table);
            int size=int(content.size()/BLOCK_SIZE_) + (content.size()%BLOCK_SIZE_!=0);
            vector<string> p_link=portLink(size);
            fs.close();
            // if(MEM_ALLOC_TYPE_=="contiguous"){
            //     writeFile(e.startBlock_,pos,content);
            // }else{
            vector<string> c=splitToBlocks(content);
            if(c.size()>getFreeBlocks(table)){
                cout<<"Not Enough Space"<<endl;
                return;
            }
            int start=writeLink(c,p_link);
            FATEntry e(fileName,p_link[0],start,size);
            // }
            writeFAT(e.toString(PTR_LEN_L_)+TABLE_END_);
            vector<string> ports=getPorts();
            Request r;
            for(int i=1;i<ports.size();i++){
                r.updateFAT(ports[i],e.toString(PTR_LEN_L_)+TABLE_END_);
            }
            return;
        }
        string getBlockAddr(int block) {
            return getBlockPtr(CURR_NODE_PORT_ + fillPtrLen(to_string(block)));
        }
        // void deleteFile(string fileName){
        //     fstream fs;
        //     fs.open(SYSTEM_FILE_NAME_);
        //     string table;
        //     getline(fs,table);
        //     bool dne=false;
        //     FATEntry f=getFATEntry(table,fileName,dne);
        //     if(dne){
        //         cout<<"File Does Not Exist"<<endl;
        //     }else{
        //         freeLink(table,f.startBlock_,f.size_);
        //         removeFATEntry(fileName);
        //     }
        //     return;
        // }
        string readFile(string name){
            bool dne=false;
            string f;
            // if(MEM_ALLOC_TYPE_=="contiguous"){
            //     f=getFile(name,dne);
            // }else{
            f=getLinkedFile(name,dne);
            // }

            if(dne){
                return "File Does Not Exist.\n";
            }else{
                return f.substr(0,f.find(EOF_))+'\n';
            }
            return "";
        }
        vector<string> ls_Files(){
            vector<string> files;
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string t;
            getline(fs,t);
            t=t.substr(t.find('~')+1);
            fs.close();
            vector<FATEntry> table=parseTable(t);
            for(int i=2;i<table.size();i++){
                files.push_back(table[i].name_);
            }
            return files;
        }
        void addPort(string port){
            writeNewPort(port);
            return;
        }
        string writeBlock_p(string content){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string table;
            getline(fs,table);
            int pos=fs.tellg();
            int s=table.find('~')+1;
            table=table.substr(s);
            int head=getHead(table);
            fs.close();
            writeBlock(content,head);
            updateHead(getNextHead(head,1),getFreeBlocks(table)-1);
            return CURR_NODE_PORT_+fillPtrLen(to_string(head));
        }
        void setNextAddr(int block,string nAddr){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string t;
            getline(fs,t);
            int pos=fs.tellg();
            fs.seekp(pos+(block*(BLOCK_SIZE_+PTR_LEN_L_+CURR_NODE_PORT_.length()))+BLOCK_SIZE_);
            fs.write(nAddr.c_str(),PTR_LEN_L_+CURR_NODE_PORT_.length());
            fs.close();
            return;
        }
        string getBlockContent(int block){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string table;
            getline(fs,table);
            int pos=fs.tellg();
            string outp="";
            fs.seekg(pos+block*(BLOCK_SIZE_+PTR_LEN_L_+CURR_NODE_PORT_.length()));
            char c;
            for (size_t i = 0; i < BLOCK_SIZE_; i++) {
                fs.get(c);
                outp+=c;
                if(c==EOF_){
                    fs.close();
                    return outp;
                }
            }
            fs.close();
            return outp;
        }
        void updateFAT(string entry){
            writeFAT(entry);
            return;
        }
    private:
        const string SYSTEM_FILE_NAME_;
        const string MEM_ALLOC_TYPE_="LINKED";
        const int BLOCK_SIZE_;
        const int TOTAL_BLOCKS_;
        const int PTR_LEN_L_;


        // const int MAX_NODES_=100;
        const string CURR_NODE_PORT_;
        // int NODE_COUNT_=0;

        const string TABLE_FMT_CHAR_="/";
        const string TABLE_ENTRY_SEP_="~";
        const string TABLE_END_="\n";
        const string EMPTY_BLOCK_;
        const char EOF_='\e';



        struct FATEntry{
            const string TABLE_FMT_CHAR_="/";
            const string TABLE_ENTRY_SEP_="~";

            string name_;
            string startNodePort_;
            int startBlock_;
            int size_;
            // FATEntry(File f,int startBlock):name_{f.name_},startBlock_{startBlock},size_{f.memBlocks_}{}
            //Format: /<name_>/<startNode_><startBlock_>/<size_>~
            FATEntry(string name,string startNodePort,int startBlock,int size):name_{name},startNodePort_{startNodePort},startBlock_{startBlock},size_{size}{}
            string toString(int ptr_l){
                return TABLE_FMT_CHAR_+name_+TABLE_FMT_CHAR_+startNodePort_+format(ptr_l,to_string(startBlock_))+TABLE_FMT_CHAR_+to_string(size_)+TABLE_ENTRY_SEP_;
            }
            string format(int len,string ptr){
                string p=ptr;
                while(p.length()<len){
                    p="0"+p;
                }
                return p;
            }
        };

        // void init(){
        //     fstream fs;
        //     fs.open(SYSTEM_FILE_NAME_);
        //     fs<<TABLE_END_;
        //     for (size_t i = 0; i < TOTAL_BLOCKS_; i++) {
        //         fs<<EMPTY_BLOCK_;
        //     }
        //     fs.close();
        // }
        // int getStartBlock(string table){
        //     vector<FATEntry> t=parseTable(table);
        //     if(!t.size()){
        //         return 0;
        //     }
        //     auto last=t.back();
        //     return last.startBlock_+last.size_;
        // }
        // string getFile(string fileName,bool & dne){
        //     string outp;
        //     char c;
        //     fstream fs;
        //     fs.open(SYSTEM_FILE_NAME_);
        //     string table;
        //     getline(fs,table);
        //     int pos=fs.tellg();
        //     for(auto e:parseTable(table)){
        //         if(e.name_==fileName){
        //             fs.seekg(pos+e.startBlock_*BLOCK_SIZE_);
        //             for (size_t i = e.startBlock_*BLOCK_SIZE_; i < (e.startBlock_+e.size_)*BLOCK_SIZE_; i++) {
        //                 fs.get(c);
        //                 outp+=c;
        //             }
        //             return outp;
        //         }
        //     }
        //     fs.close();
        //     dne=true;
        //     return "";
        // }
        // void writeFile(int startBlock,int pos,string content){
        //     fstream fs;
        //     fs.open(SYSTEM_FILE_NAME_);
        //     fs.seekp(pos + (startBlock*BLOCK_SIZE_));
        //     const char* c=content.c_str();
        //     fs.write(c,content.size());
        //     // for(char c:content){
        //     //     fs.put(c);
        //     // }
        //     fs.close();
        // }
        void writeNewPort(string port){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string table;
            getline(fs,table);
            long block_s=fs.tellg();
            int pos=table.find("~");
            string beg=table.substr(0,pos);
            string end = table.substr(pos) + '\n';
            string nPort=beg+'|'+port+end;

            fs.seekg (0,fs.end);
            long size = fs.tellg();
            fs.seekg (block_s);
            char* buffer = new char[size-block_s];
            fs.read (buffer,size-block_s);
            const char* e_=nPort.c_str();
            fs.seekp(0,fs.beg);
            fs.write(e_,nPort.length());
            fs.write(buffer,size-block_s);
            fs.close();
            return;
        }
        vector<string> getPorts(){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string table;
            getline(fs,table);
            fs.close();
            vector<string> ports;
            table=table.substr(0,table.find('~'));
            int pos=1;
            string tmp;
            while(table.find('|',pos)!=string::npos){
                tmp=table.substr(pos,CURR_NODE_PORT_.length());
                ports.push_back(tmp);
                pos=table.find('|',pos)+1;
            }
            ports.push_back(table.substr(pos,CURR_NODE_PORT_.length()));
            return ports;
        }
        void writeFAT(string entry){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string table;
            getline(fs,table);
            int pos=fs.tellg();

            fs.seekg (0,fs.end);
            long size = fs.tellg();
            fs.seekg (pos);

            char* buffer = new char[size-pos];
            fs.read (buffer,size-pos);

            fs.seekp(pos-1);
            const char* e_=entry.c_str();
            fs.write(e_,entry.size());
            fs.write(buffer,size-pos);

            delete[] buffer;
            fs.close();
        }
        vector<FATEntry> parseTable(string table){
            vector<FATEntry> outp;
            size_t pos_t=0;
            size_t pos_e=0;
            string entry;

            string name;
            string startNode;
            string start;
            int startBlock;
            int size;
            while(table.find("~",pos_t)!=string::npos){
                entry=table.substr(pos_t,table.find("~",pos_t));
                entry = entry.substr(1);
                pos_e=entry.find("/",1);
                name=entry.substr(1,pos_e-1);
                pos_e+=1;
                start=entry.substr(pos_e,entry.find("/",pos_e)-pos_e);
                startNode=start.substr(0,CURR_NODE_PORT_.length());

                startBlock=stoi(start.substr(CURR_NODE_PORT_.length()));
                pos_e=entry.find("/",pos_e)+1;

                size=stoi(entry.substr(pos_e,entry.size()-1));
                FATEntry f(name,startNode,startBlock,size);
                outp.push_back(f);
                pos_t=table.find("~",pos_t)+1;
            }
            return outp;
        }
        FATEntry getFATEntry(string table,string fileName,bool & dne){
            vector<FATEntry> f=parseTable(table);
            for (size_t i = 2; i < f.size(); i++) {
                if(f[i].name_==fileName){
                    return f[i];
                }
            }
            dne=true;
            FATEntry e("","",0,0);
            return e;
        }
        void removeFATEntry(string fileName){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string t;
            getline(fs,t);
            long block_s=fs.tellg();
            int pos=t.find(TABLE_ENTRY_SEP_)+1;
            pos=t.find(TABLE_ENTRY_SEP_,pos)+1;//after tail
            pos=t.find(fileName)-1;
            string beg=t.substr(0,pos);
            pos=t.find(TABLE_ENTRY_SEP_,pos)+1;
            string nTable=beg+t.substr(pos)+TABLE_END_;

            fs.seekg (0,fs.end);
            long size = fs.tellg();
            fs.seekg (block_s);
            char* buffer = new char[size-block_s];
            fs.read (buffer,size-block_s);
            const char* e_=nTable.c_str();
            fs.seekp(0,fs.beg);
            fs.write(e_,nTable.length());
            fs.write(buffer,size-block_s);
            fs.close();
            return;
        }
        void init_L(){
            ofstream f(SYSTEM_FILE_NAME_);
            f.close();
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            vector<int> b;

            for(size_t i = 1; i <= TOTAL_BLOCKS_; i++) {
                b.push_back(i);
            }

            if (RAND_MAX<TOTAL_BLOCKS_) {
                cout<<"INIT_ERROR_RXGTB";
            }
            string ptr;
            string tail;
            int x;
            srand (time(NULL));
            for (size_t i = 0; i < TOTAL_BLOCKS_; i++) {
                x=rand()%b.size();
                ptr=fillPtrLen(to_string(b[x]));
                if (b[x]==TOTAL_BLOCKS_) {
                    tail=to_string(i);
                }
                fs<<EMPTY_BLOCK_<<CURR_NODE_PORT_+ptr;
                b.erase(b.begin()+x);
            }
            // int tb=TOTAL_BLOCKS_;
            ptr='|'+CURR_NODE_PORT_+'~'+TABLE_FMT_CHAR_+"head"+TABLE_FMT_CHAR_+CURR_NODE_PORT_+fillPtrLen("0")+TABLE_FMT_CHAR_+to_string(TOTAL_BLOCKS_)+TABLE_ENTRY_SEP_+TABLE_FMT_CHAR_+"tail"+TABLE_FMT_CHAR_+CURR_NODE_PORT_+fillPtrLen(tail)+TABLE_FMT_CHAR_+"0"+TABLE_ENTRY_SEP_+TABLE_END_;
            // /head/<NodeID>0/<free>/tail/<NodeID><ptr>/0~
            fs.seekg (0,fs.end);
            long size = fs.tellg();
            fs.seekg (0,fs.beg);

            char* buffer = new char[size];
            fs.read (buffer,size);
            const char* e_=ptr.c_str();
            fs.seekp(0);
            fs.write(e_,ptr.length());
            fs.write(buffer,size);
            // TOTAL_NODES_++;
            fs.close();
        }
        int getPtrLen(int s){
            int i=0;
            while(s>0){
                s/=10;
                i++;
            }
            return i;
        }
        string emptyBlock(int b_size){
            string s="";
            for (size_t i = 0; i < b_size; i++) {
                s+='~';
            }
            return s;
        }
        string fillPtrLen(string ptr){
            string p=ptr;
            while(p.length()<PTR_LEN_L_){
                p="0"+p;
            }
            return p;
        }


        void updateHead(int nHead,int nSize){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string table;
            getline(fs,table);
            long block_s=fs.tellg();
            int pos=table.find("/")+6;

            string beg=table.substr(0,pos);
            pos=table.find("/",pos)+1;
            pos=table.find("/",pos)-1;
            string end=table.substr(pos);
            string nTable=beg+CURR_NODE_PORT_+fillPtrLen(to_string(nHead))+"/"+to_string(nSize)+end+TABLE_END_;
            fs.seekg (0,fs.end);
            long size = fs.tellg();
            fs.seekg (block_s);
            char* buffer = new char[size-block_s];
            fs.read (buffer,size-block_s);
            const char* e_=nTable.c_str();
            fs.seekp(0,fs.beg);
            fs.write(e_,nTable.length());
            fs.write(buffer,size-block_s);
            fs.close();
            return;
        }
        void updateTail(int nTail){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string table;
            getline(fs,table);
            long block_s=fs.tellg();
            int pos=table.find("tail")+5;
            string beg=table.substr(0,pos);
            pos=table.find("/",pos);
            string end=table.substr(pos);
            string nTable=beg+CURR_NODE_PORT_+fillPtrLen(to_string(nTail))+end+TABLE_END_;

            fs.seekg (0,fs.end);
            long size = fs.tellg();
            fs.seekg (block_s);
            char* buffer = new char[size-block_s];
            fs.read (buffer,size-block_s);
            const char* e_=nTable.c_str();
            fs.seekp(0,fs.beg);
            fs.write(e_,nTable.length());
            fs.write(buffer,size-block_s);
            fs.close();
            return;
        }
        void updateBlockPtr(int block,string nodeID,int nPtr){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string t;
            getline(fs,t);
            int pos=fs.tellg();
            fs.seekp(pos+(block*(BLOCK_SIZE_+PTR_LEN_L_+CURR_NODE_PORT_.length()))+BLOCK_SIZE_);
            string ptr=nodeID+fillPtrLen(to_string(nPtr));
            fs.write(ptr.c_str(),PTR_LEN_L_+CURR_NODE_PORT_.length());
            fs.close();
            return;
        }
        int getHead(string table){
            vector<FATEntry> t=parseTable(table);
            return t[0].startBlock_;
        }
        int getTail(string table){
            vector<FATEntry> t=parseTable(table);
            return t[1].startBlock_;
        }
        int getFreeBlocks(string table){
            vector<FATEntry> t=parseTable(table);
            return t[0].size_;
        }
        int getNextHead(int cHead, int used){
            string nh=getBlockPtr(CURR_NODE_PORT_+to_string(cHead));
            for(int i=1;i<used;i++){
                nh=getBlockPtr(nh);
            }
            return stoi(nh.substr(CURR_NODE_PORT_.length()));
        }
        vector<string> splitToBlocks(string str){
            vector<string> outp;
            int pos=0;
            while(pos<str.length()){
                outp.push_back(str.substr(pos,BLOCK_SIZE_));
                pos+=BLOCK_SIZE_;
            }
            outp.push_back(str.substr(pos-BLOCK_SIZE_));
            return outp;
        }
        string getBlockPtr(string nAddr){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string t;
            getline(fs,t);
            int start=fs.tellg();
            int block=stoi(nAddr.substr(CURR_NODE_PORT_.length()));
            string addr;
            char c;

            fs.seekg(start+block*(BLOCK_SIZE_+PTR_LEN_L_+CURR_NODE_PORT_.length())+BLOCK_SIZE_);
            for(size_t i = 0; i < CURR_NODE_PORT_.length()+PTR_LEN_L_; i++){
                fs.get(c);
                addr+=c;
            }
            fs.close();
            return addr;
        }
        vector<string> portLink(int size){
            vector<string> nodes=getPorts();
            vector<string> link;
            // for(int i=0;i<NODE_COUNT_;i++){
            //     nodes.push_back(to_string(i));
            // }
            string tmp;
            int j;
            if(nodes.size()>1){
                for(int i=0;link.size()<size;i++){
                    j=rand()%(nodes.size()-i);
                    link.push_back(nodes[j]);
                    tmp=nodes[j];
                    nodes[j]=nodes[nodes.size()-i-1];
                    nodes[nodes.size()-i-1]=tmp;
                    i=(i==nodes.size()-1)?-1:i;
                }
            }else{
                for(int i=0;i<size;i++){
                    link.push_back(nodes[0]);
                }
            }
            return link;
        }
        int writeLink(vector<string> str,vector<string> ports){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string table;
            getline(fs,table);
            table=table.substr(table.find('~')+1);
            int head;
            fs.close();
            string addr;
            int start=-1;
            int prevBlockNum;
            Request r;
            for (size_t i = 0; i < ports.size(); i++) {
                if(ports[i]==CURR_NODE_PORT_){
                    head=getHead(table);
                    writeBlock(str[i],head);
                    updateHead(getNextHead(head,1),getFreeBlocks(table)-1);
                    if(i==0){
                        start=head;
                    }else{
                        if(ports[i-1]==CURR_NODE_PORT_){
                            updateBlockPtr(prevBlockNum,ports[i],stoi(addr.substr(CURR_NODE_PORT_.length())));
                        }else{
                            r.set_next_addr(ports[i-1],prevBlockNum,CURR_NODE_PORT_+to_string(head));
                        }
                    }
                    prevBlockNum=head;
                }else{
                    addr=r.write_block(ports[i],str[i]);
                    //send writeBlock_p to port: ports[i]

                    if(i==0){
                        start=stoi(addr.substr(CURR_NODE_PORT_.length()));
                    }else{
                        if(ports[i-1]==CURR_NODE_PORT_){
                            updateBlockPtr(prevBlockNum,ports[i],stoi(addr.substr(CURR_NODE_PORT_.length())));
                        }else{
                            r.set_next_addr(ports[i-1],prevBlockNum,addr);
                        }
                    }
                    prevBlockNum=stoi(addr.substr(CURR_NODE_PORT_.length()));
                }
                fs.open(SYSTEM_FILE_NAME_);
                getline(fs,table);
                table=table.substr(table.find('~')+1);
                fs.close();
            }
            return start;
        }

        void writeBlock(string word,int pos){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string t;
            getline(fs,t);
            int offset=fs.tellg();
            fs.seekp(pos*(BLOCK_SIZE_+PTR_LEN_L_+CURR_NODE_PORT_.length())+offset);
            for(auto c:word)
                fs.put(c);
            fs.close();
            return;
        }
        vector<pair<string,int>> getPtrLink(string port,int block,int size){
            vector<pair<string,int>> link;
            link.push_back(make_pair(port,block));
            string addr;
            Request r;
            for(int i=1;i<size;i++){
                if(port==CURR_NODE_PORT_){
                    addr=getBlockPtr(port+fillPtrLen(to_string(block)));
                    port=addr.substr(0,CURR_NODE_PORT_.length());
                    block=stoi(addr.substr(CURR_NODE_PORT_.length()));
                    link.push_back(make_pair(port,block));
                }else{
                    addr=r.Request_block_ptr(port,block);
                    port=addr.substr(0,CURR_NODE_PORT_.length());
                    block=stoi(addr.substr(CURR_NODE_PORT_.length()));
                    link.push_back(make_pair(port,block));
                }
            }
            return link;
            // fstream fs;
            // fs.open(SYSTEM_FILE_NAME_);
            // string t;
            // getline(fs,t);
            // int start=fs.tellg();
            // int block=head;
            // string ptr="";
            // string node="";
            // char c;
            // for (size_t i = 0; i < size; i++) {
            //     fs.seekg(start+block*(BLOCK_SIZE_+PTR_LEN_L_+CURR_NODE_PORT_.length())+BLOCK_SIZE_);
            //     for(size_t k = 0; k < CURR_NODE_PORT_.length(); k++){
            //         fs.get(c);
            //         node+=c;
            //     }
            //     for (size_t j = 0; j < PTR_LEN_L_; j++) {
            //         fs.get(c);
            //         ptr+=c;
            //     }
            //     link.push_back(make_pair(node,stoi(ptr)));
            //     block=stoi(ptr);
            //     node="";
            //     ptr="";
            // }
            // fs.close();
            // return link;
        }
        // int getLinkTail(int head,int size){
        //     vector<int> link=getPtrLink(head,size);
        //     return link.back();
        // }
        string readBlock(int pos,int ptr){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string outp="";
            fs.seekg(pos+ptr*(BLOCK_SIZE_+PTR_LEN_L_+CURR_NODE_PORT_.length()));
            char c;
            for (size_t i = 0; i < BLOCK_SIZE_; i++) {
                fs.get(c);
                outp+=c;
                if(c==EOF_){
                    fs.close();
                    return outp;
                }
            }
            fs.close();
            return outp;
        }
        string getLinkedFile(string fname,bool & dne){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string table;
            getline(fs,table);
            int start=table.find('~')+1;
            table=table.substr(start);
            int pos=fs.tellg();
            fs.close();
            vector<FATEntry> t=parseTable(table);
            string file="";
            vector<pair<string,int>> l;
            Request r;
            for (size_t i = 2; i < t.size(); i++) {

                if(t[i].name_==fname){
                    l=getPtrLink(t[i].startNodePort_,t[i].startBlock_,t[i].size_);
                    for(auto j:l){
                        if(j.first==CURR_NODE_PORT_){
                            file+=readBlock(pos,j.second);
                        }else{
                            file+=r.Request_block_content(j.first,j.second);
                        }
                    }
                    return file;
                }
            }
            dne=true;
            return file;
        }
        // void freeLink(string table,int lhead,int lsize){
        //     int lTail=getTail(table);
        //     int nTail=getLinkTail(lhead,lsize);
        //     updateBlockPtr(lTail,NODE_ID_,TOTAL_BLOCKS_);
        //     updateBlockPtr(nTail,NODE_ID_,lhead);
        //     updateTail(lTail);
        //     updateHead(getHead(table),getFreeBlocks(table)+lsize);
        //     return;
        // }
};
#endif
