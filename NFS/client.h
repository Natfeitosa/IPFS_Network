#ifndef CLIENT
#define CLIENT


#include <iostream>
#include <stdio.h>

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iphlpapi.h>
#define PORT "27015"
#define BUFFER_SIZE 1024
#include "header.h"
#include <string>
#pragma comment(lib,"Ws2_32.lib")
/*
  THIS IS MEANT TO SEND DATA TO OTHER NODES IN THE NETWORK
  MUST KNOW THEIR PORT ADDRESS WHERE THEY ARE LOCATED
  TODO:
    Add a time out for recv and send sockets


*/

class Sender
{
  public:
    Sender();
    void Connect_client();
    bool Send_data(const char* message);
    std::string Read(std::string &filename);
    void WriteFile(std::string filename,std::string content);
    std::string list();
  private:
    void Create_Socket();
    SOCKET client;
    int valread;
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


Sender::Sender()
{
  WSADATA wsaData;
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0)
  {
    printf("WSASstartup failed: %d\n", iResult);

  }
}
bool Sender::Send_data(const char* message)
{

  iResult = send(client, message, (int)strlen(message), 0);
  if (iResult == SOCKET_ERROR)
  {
    printf("Had problem sending message %d\n", WSAGetLastError());
    closesocket(client);
    WSACleanup();
    return false;
  }
  iResult = shutdown(client, SD_SEND);
  if (iResult == SOCKET_ERROR) {
    printf("Had problem shutting down send service: %d\n", WSAGetLastError());
    closesocket(client);
    WSACleanup();
    return EXIT_FAILURE;
  }
  do {
      iResult = recv(client, buffer, buffer_size, 0);
      if (iResult > 0) {
        printf("Bytes revieced: %d\n", iResult);
      }
      else if(iResult==0){
        printf("Connection finished\n");
      }
      else{
        printf("recv failed: %d\n", WSAGetLastError());
      }

  } while (iResult > 0);
  printf("sever said: %s\n", buffer);
  closesocket(client);
  return true;
  }

void Sender::Connect_client()
{
    Create_Socket();
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
    if(!Handshake()){
      closesocket(client);
    }



  }

void Sender::Create_Socket()
{
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    iResult = getaddrinfo("127.0.0.1", PORT, &hints, &result);
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
    //return clientz;
  }

std::string Sender::Read(std::string &filename)
{
  Header serverheader;
  Header clientheader;
  const char *read="read";
  const char *name=filename.c_str();
  //while(1)// will loop until it recieves the file
  //{
    //SENDS OPERATION TO THE SERVER
    iResult= send(client,read,(int)strlen(read),0);
    if(iResult==SOCKET_ERROR)
    {
      printf("Had problem sending message: %d\n", WSAGetLastError());
    }
    iResult = recv(client,(char*)&serverheader,sizeof(serverheader),0);//This waits for the confirmation that the server recieved the read request
    if(iResult==SOCKET_ERROR)
    {
      printf("Had problem recv the message: %d\n", WSAGetLastError());
    }
    if(serverheader.ack==1)
    {//SEND FILENAME TO SERVER
      serverheader.ack=0;
      iResult = send(client,name,(int)strlen(name),0);//Send the name of the file that client wants to read
      iResult = recv(client, (char*)&serverheader, sizeof(serverheader), 0);//confirms it recieved the names
      if(iResult==SOCKET_ERROR){
        printf("Error recv things to client_socket: %d\n", WSAGetLastError());
      }
      if(serverheader.ack == 1)
      {
        printf("ACK for Filename recv\n");
      /* This is where the client waits for the server to send the file.*/

        printf("Recving file\n");

        iResult= recv(client,buffer,buffer_size,0);//Might have to loop multiple times depending on the size of the file
        printf("This is the buffer: %s\n",buffer);
        clientheader.ack=1;
        iResult= send(client,(char*)&clientheader,sizeof(clientheader),0);
        std::string file(buffer);
        return file;
      }
      else
      {
        return "Was not able to get file\n";
      }
    }
    else
      printf("did not recieve ack from Server");
  //}
}

void Sender::WriteFile(std::string filename,std::string content)
{
  Header serverh;
  Header clienth;
  printf("Sending operation\n");
  const char* write= "write";
  iResult=send(client,write,(int)strlen(write),0);
  printf("waiting for ack\n");
  iResult=recv(client,(char*)&clienth,sizeof(clienth),0);
  if(clienth.ack==1)
  { clienth.ack=0;
    printf("sending file name\n");
    iResult=send(client,filename.c_str(),filename.size(),0);
    printf("waiting for ack\n");
    iResult=recv(client,(char*)&clienth,sizeof(clienth),0);
    if(clienth.ack==1)
    {
      clienth.ack=0;
      printf("sending content\n");
      iResult=send(client,content.c_str(),content.size(),0);
      printf("waiting for ack\n");
      iResult=recv(client,(char*)&clienth,sizeof(clienth),0);
      if(clienth.ack==1)
      {
        printf("File was created!\n");
      }
    }
  }
}
std::string Sender::list() {
    char buff[BUFFER_SIZE];
    Header serverheader;
    Header clientheader;
    const char* list = "list";
    iResult = send(client, list, (int)strlen(list), 0);
    iResult = recv(client, (char*)&serverheader, sizeof(serverheader), 0);//This waits for the confirmation that the server recieved the read request
    if (iResult == SOCKET_ERROR)
    {
        printf("Had problem recv the message: %d\n", WSAGetLastError());
    }
    if (serverheader.ack == 1)
    {//SEND FILENAME TO SERVER
        serverheader.ack = 0;


        printf("ACK for Filename recv\n");
        /* This is where the client waits for the server to send the file.*/

        printf("Recving file\n");

        iResult = recv(client, buff, buffer_size, 0);//Might have to loop multiple times depending on the size of the file
        printf("This is the buffer: %s\n", buff);
        clientheader.ack = 1;
        iResult = send(client, (char*)&clientheader, sizeof(clientheader), 0);
        std::string file(buff);
        return file;
    }
}
#endif
