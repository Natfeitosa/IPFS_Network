#ifndef FILESYS_H
#define FILESYS_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h> //random
#include <time.h>
#include <utility> //pair
using namespace std;

class FileSys{
    public:
        FileSys(string File,int Block_S,int Size,string NodeID):
            SYSTEM_FILE_NAME_{File},BLOCK_SIZE_{Block_S},TOTAL_BLOCKS_{Size},PTR_LEN_L_{getPtrLen(Size)},EMPTY_BLOCK_{emptyBlock(Block_S)},CURR_NODE_{stoi(NodeID)},NODE_ID_{fillNodeLen(NodeID)}{
            init_L();
        }
        // FileSys(string File):SYSTEM_FILE_NAME_{File},BLOCK_SIZE_{8},TOTAL_BLOCKS_{500},PTR_LEN_L_{getPtrLen(500)},EMPTY_BLOCK_{"~~~~~~~~"},CURR_NODE_{NodeID},NODE_ID_{fillNodeLen(NodeID)}{}
        // ~FileSys(){
        //     fs.close();
        // }
        void createFile(string fileName,string content){
            fstream fs;
            content+=EOF_;
            
            fs.open(SYSTEM_FILE_NAME_);
            string table;
            getline(fs,table);

            if (table.find(fileName+TABLE_FMT_CHAR_)!=string::npos){
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
            int start=getHead(table);
            int size=int(content.size()/BLOCK_SIZE_) + (content.size()%BLOCK_SIZE_!=0);
            FATEntry e(fileName,NODE_ID_,start,size);
            int pos=fs.tellg();
            fs.close();
            // if(MEM_ALLOC_TYPE_=="contiguous"){
            //     writeFile(e.startBlock_,pos,content);
            // }else{
                vector<string> c=splitToBlocks(content);
                if(c.size()>getFreeBlocks(table)){
                    cout<<"Not Enough Space"<<endl;
                    return;
                }
                vector<pair<string,int>> link=getPtrLink(e.startNode_,e.startBlock_,e.size_);
                writeLink(c,link);
                int nHead= getNextHead(e.startBlock_,e.size_);
                int nFreeB=getFreeBlocks(table)-e.size_;
                updateHead(nHead,nFreeB);
            // }
            writeFAT(e.toString()+TABLE_END_);
            return;
        }
        void deleteFile(string fileName){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string table;
            getline(fs,table);
            bool dne=false;
            FATEntry f=getFATEntry(table,fileName,dne);
            if(dne){
                cout<<"File Does Not Exist"<<endl;
            }else{
                freeLink(table,f.startBlock_,f.size_);
                removeFATEntry(fileName);
            }
            return;
        }
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
            return "file not found";
        }
        vector<string> ls_Files(){
            vector<string> files;
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string t;
            getline(fs,t);
            fs.close();
            for(auto e:parseTable(t)){
                files.push_back(e.name_);
            }
            return files;
        }
    private:
        const string SYSTEM_FILE_NAME_;
        const string MEM_ALLOC_TYPE_="LINKED";
        const int BLOCK_SIZE_;
        const int TOTAL_BLOCKS_;
        const int PTR_LEN_L_;


        const int MAX_NODES_=100;
        // static int TOTAL_NODES_;
        const int CURR_NODE_;
        const string NODE_ID_;

        const string TABLE_FMT_CHAR_="/";
        const string TABLE_ENTRY_SEP_="~";
        const string TABLE_END_="\n";
        const string EMPTY_BLOCK_;
        const char EOF_='\^';



        struct FATEntry{
            const string TABLE_FMT_CHAR_="/";
            const string TABLE_ENTRY_SEP_="~";

            string name_;
            string startNode_;
            int startBlock_;
            int size_;
            // FATEntry(File f,int startBlock):name_{f.name_},startBlock_{startBlock},size_{f.memBlocks_}{}
            //Format: /<name_>/<startNode_><startBlock_>/<size_>~
            FATEntry(string name,string startNode,int startBlock,int size):name_{name},startNode_{startNode},startBlock_{startBlock},size_{size}{}
            string toString(){
                return TABLE_FMT_CHAR_+name_+TABLE_FMT_CHAR_+startNode_+to_string(startBlock_)+TABLE_FMT_CHAR_+to_string(size_)+TABLE_ENTRY_SEP_;
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

            fs.seekp(pos-2);
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
                entry=table.substr(pos_t,table.find("~",pos_t)-1);
                pos_e=entry.find("/",2);
                name=entry.substr(1,pos_e-1);
                pos_e+=1;
                start=entry.substr(pos_e,entry.find("/",pos_e)-pos_e);
                startNode=start.substr(0,NODE_ID_.length());
                startBlock=stoi(start.substr(NODE_ID_.length()));
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
                fs<<EMPTY_BLOCK_<<NODE_ID_+ptr;
                b.erase(b.begin()+x);
            }
            // int tb=TOTAL_BLOCKS_;
            ptr = TABLE_FMT_CHAR_ + "head" + TABLE_FMT_CHAR_ + NODE_ID_ + fillPtrLen("0") + TABLE_FMT_CHAR_ + to_string(TOTAL_BLOCKS_) + TABLE_ENTRY_SEP_ + TABLE_FMT_CHAR_ + "tail" + TABLE_FMT_CHAR_ + NODE_ID_ + fillPtrLen(tail) + TABLE_FMT_CHAR_ + "0" + TABLE_ENTRY_SEP_ + TABLE_END_;
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
        string fillNodeLen(string id){
            string p=id;
            while(p.length()<to_string(MAX_NODES_).length()){
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
            int pos=table.find("/",1)+1;
            string beg=table.substr(0,pos);
            pos=table.find("/",pos)+1;
            pos=table.find("/",pos)-1;
            string end=table.substr(pos);
            string nTable=beg+NODE_ID_+to_string(nHead)+"/"+to_string(nSize)+end+TABLE_END_;

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
            string nTable=beg+NODE_ID_+to_string(nTail)+end+TABLE_END_;

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
            fs.seekp(pos+(block*(BLOCK_SIZE_+PTR_LEN_L_+NODE_ID_.length()))+BLOCK_SIZE_);
            string ptr=nodeID+fillPtrLen(to_string(nPtr));
            fs.write(ptr.c_str(),PTR_LEN_L_+NODE_ID_.length());
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
            vector<int> l=getPtrLink(cHead,used+1);
            return l.back();
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

        void writeLink(vector<string> str,vector<pair<string,int>> link){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string t;
            getline(fs,t);
            int pos=fs.tellg();
            fs.close();
            for (size_t i = 0; i < link.size(); i++) {
                writeBlock(link[i].first,link[i].second*(BLOCK_SIZE_+PTR_LEN_L_+NODE_ID_.length())+pos,str[i]);
            }
            return;
        }

        void writeBlock(string nodeID,int pos,string word){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            fs.seekp(pos);
            for(auto c:word)
                fs.put(c);
            fs.close();
            return;
        }
        vector<int> getPtrLink(int head,int size){
            vector<int> link;
            link.push_back(head);
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string t;
            getline(fs,t);
            int start=fs.tellg();
            int block=head;
            string ptr="";
            char c;
            for (size_t i = 0; i < size; i++) {
                fs.seekg(start+block*(BLOCK_SIZE_+PTR_LEN_L_+NODE_ID_.length())+BLOCK_SIZE_+NODE_ID_.length());
                for (size_t j = 0; j < PTR_LEN_L_; j++) {
                    fs.get(c);
                    ptr+=c;
                }
                link.push_back(stoi(ptr));
                block=stoi(ptr);
                ptr="";
            }
            fs.close();
            return link;
        }
        vector<pair<string,int>> getPtrLink(string headNode,int head,int size){
            vector<pair<string,int>> link;
            link.push_back(make_pair(headNode,head));
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string t;
            getline(fs,t);
            int start=fs.tellg();
            int block=head;
            string ptr="";
            string node="";
            char c;
            for (size_t i = 0; i < size; i++) {
                fs.seekg(start+block*(BLOCK_SIZE_+PTR_LEN_L_+NODE_ID_.length())+BLOCK_SIZE_);
                for(size_t k = 0; k < NODE_ID_.length(); k++){
                    fs.get(c);
                    node+=c;
                }
                for (size_t j = 0; j < PTR_LEN_L_; j++) {
                    fs.get(c);
                    ptr+=c;
                }
                link.push_back(make_pair(node,stoi(ptr)));
                block=stoi(ptr);
                node="";
                ptr="";
            }
            fs.close();
            return link;
        }
        int getLinkTail(int head,int size){
            vector<int> link=getPtrLink(head,size);
            return link.back();
        }
        string readBlock(int pos,int ptr){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string outp="";
            fs.seekg(pos+ptr*(BLOCK_SIZE_+PTR_LEN_L_+NODE_ID_.length()));
            char c;
            for (size_t i = 0; i < BLOCK_SIZE_; i++) {
                fs.get(c);
                outp+=c;
                if(c==EOF_){
                    fs.close();
                    return outp;
                }
            }
            return outp;
        }
        string getLinkedFile(string fname,bool & dne){
            fstream fs;
            fs.open(SYSTEM_FILE_NAME_);
            string table;
            getline(fs,table);
            int pos=fs.tellg();
            vector<FATEntry> t=parseTable(table);
            string file="";
            fs.close();

            for (size_t i = 2; i < t.size(); i++) {

                if(t[i].name_==fname){
                    vector<pair<string,int>> l=getPtrLink("0",t[i].startBlock_,t[i].size_);
                    
                    for(auto j:l){
                        string block_part = readBlock(pos, j.second);
                        file+= block_part;
                    }
                    return file;
                }
            }
            dne=true;
            return file;
        }
        void freeLink(string table,int lhead,int lsize){
            int lTail=getTail(table);
            int nTail=getLinkTail(lhead,lsize);
            updateBlockPtr(lTail,NODE_ID_,TOTAL_BLOCKS_);
            updateBlockPtr(nTail,NODE_ID_,lhead);
            updateTail(lTail);
            updateHead(getHead(table),getFreeBlocks(table)+lsize);
            return;
        }
};
#endif
