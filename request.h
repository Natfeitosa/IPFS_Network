#ifndef REQUEST
#define REQUEST


#include <iostream>
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iphlpapi.h>
#define BUFFER_SIZE 1024
#include "header.h"
#include <string>
#pragma comment(lib,"Ws2_32.lib")

class Request{
public:
  Request();
  std::string Request_block_ptr(std::string port_num,int block_num);
  std::string Request_block_content(std::string port_num,int block);
  std::string write_block(std::string port_num,std::string content);
  void set_next_addr(std::string port, int prevBlock,std::string new_addr);
  void updateFAT(std::string port_num, std::string entry);
private:
  void Create_socket(std::string port);
  void Connect_client(std::string port);
  SOCKET client;
  int iResult;
  char buffer[BUFFER_SIZE]={0};
  int buffer_size=BUFFER_SIZE;
  struct addrinfo *result=NULL,*ptr=NULL,hints;

  bool Handshake() //Change so the client and server know the size of each others buffer;
  {
   Header clienth;
   Header serverh;
   clienth.syn = 1;
   bool confirm = false;
   printf("Starting Handshake\n");
   iResult = send(client, (char*)&clienth, sizeof(clienth), 0);
   iResult = recv(client, (char*)&serverh, sizeof(serverh), 0);
   if (serverh.ack == 1 && serverh.syn==1)
     {
       clienth.ack = 1;
       clienth.syn = 0;
       iResult = send(client, (char*)&clienth, sizeof(clienth), 0);
       printf("It worked!\n");
       return true;
      }
   else {
       printf("It failed\n");
       return false;
   }
  }

};

Request::Request()
{
  WSADATA wsaData;
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0)
  {
    printf("WSASstartup failed: %d\n", iResult);
  }
}
void Request::Create_socket(std::string port){
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  iResult = getaddrinfo("127.0.0.2", port.c_str(), &hints, &result);
  if (iResult != 0)
  {
   printf("getaddinfo failed: %d\n", iResult);
   WSACleanup();
   //return EXIT_FAILURE;
  }
  ptr=result;
  client=INVALID_SOCKET;
  client = socket(ptr->ai_family,ptr->ai_socktype,ptr->ai_protocol);//creates the socket
  if (client == INVALID_SOCKET)
  {
    printf("error creating socket: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    WSACleanup();
    //return false;
  }
}
void Request::Connect_client(std::string port){
  Create_socket(port);
  iResult = connect(client, ptr->ai_addr, (int)ptr->ai_addrlen);
  if(iResult == SOCKET_ERROR)
  {
    closesocket(client);
    client = INVALID_SOCKET;
  }
  //freeaddrinfo(result);
  if (client == INVALID_SOCKET) {
    printf("Failed to connect\n");
    WSACleanup();

  }
  while (!Handshake()) {}



}
std:: string Request::Request_block_ptr(std::string port_num,int block_num){
  Connect_client(port_num);
  Header server1;
  char buff[BUFFER_SIZE]={0};
  //SEND COMMAND
  const char* cmd="block_ptr";
  iResult=send(client,cmd,(int)strlen(cmd),0);
  iResult = recv(client, (char*)&server1, sizeof(server1), 0);
  //SEND SEND BLOCK#
  std::string block;
  block = std::to_string(block_num);
  iResult=send(client,block.c_str(),block.size(),0);
  //RECIEVE BLOCK_PTR
  iResult=recv(client,buff,BUFFER_SIZE,0);
  std::string block_ptr(buff);
  return block_ptr;
}


std:: string Request::Request_block_content(std::string port_num,int block){
  Connect_client(port_num);
  char buff[BUFFER_SIZE]={0};
  //SEND COMMAND
  const char* cmd="block_content";
  iResult=send(client,cmd,(int)strlen(cmd),0);
  Header server1;
  iResult = recv(client, (char*)&server1, sizeof(server1), 0);
  //SEND BLOCK_NUM
  std::string block_num;
  block_num = std::to_string(block);
  iResult=send(client,block_num.c_str(),block_num.size(),0);
  //RECIEVE CONTENT
  iResult=recv(client,buff,BUFFER_SIZE,0);
  std::string block_content(buff);
  return block_content;
}



std::string Request::write_block(std::string port_num,std::string content){
  Connect_client(port_num);
  Header serverack;
  //SEND COMMAND
  const char* cmd="write_block";
  iResult=send(client,cmd,(int)strlen(cmd),0);
  iResult = recv(client, (char*)&serverack, sizeof(serverack), 0);
  //SEND CONTENT
  iResult=send(client,content.c_str(),content.size(),0);
  //Expect the address back
  
  iResult = recv(client, buffer, buffer_size, 0);
 
  /*for (char i : buffer) {
      std::cout << "\""<<i<<"\"";
  }*/
  std::cout << std::endl;
  std::string addr_(buffer);
  
  for (int i = 0; i < BUFFER_SIZE; i++) {
      buffer[i] = 0;
    }
  return addr_;
}
void Request::set_next_addr(std::string port, int prevBlock, std::string new_addr) {
    Connect_client(port);
    const char* commad = "set_next_addr";
    iResult = send(client, commad, (int)strlen(commad), 0);
    Header server1;
    iResult = recv(client, (char*)&server1, sizeof(server1), 0);
    std::string prevblock;
    prevblock = std::to_string(prevBlock);
    iResult = send(client, prevblock.c_str(), prevblock.size(), 0);

    iResult = send(client, new_addr.c_str(), new_addr.size(), 0);

}
void Request::updateFAT(std::string port_num, std::string entry) {
    Connect_client(port_num);
    const char* commad = "updateFAT";
    iResult = send(client, commad, (int)strlen(commad), 0);
    Header server1;
    iResult = recv(client, (char*)&server1, sizeof(server1), 0);
    iResult = send(client, entry.c_str(), entry.size(), 0);
}

#endif
