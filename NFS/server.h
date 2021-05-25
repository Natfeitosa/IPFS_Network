#ifndef SERVER
#define SERVER
#include <iostream>
#include <stdio.h>

#include "filesys.h"
#include <vector>
#include "header.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iphlpapi.h>
#define DEFAULT_PORT "27015"
#define BUFFER_SIZE 1024

#pragma comment(lib,"Ws2_32.lib")
/*
IGNORE IGNORE IGNORE IGNORE IGNORE IGNORE IGNORE IGNORE IGNORE IGNORE IGNORE IGNORE
When using the class you must Bind and set the server to listen, before requesting to receive info
public:
  void Create_server()-
    void Listen()
    void Bind()
  char *Receive_Msg()
  Proposed chages:
    stadby()- this is where we might have select() and allow for multiple connections, then create a table
    where you can determine the reason for connection, if its to be on standby or if its to send message etc.
    FD will keep track of the clients since i am not using threads.
    List of addresses will keep track of where the server is located to send data.
    For recieving data, just listen to the port and wait for someone to connect.
    type of messages that will be recieved:
      -UPDATE
      -NEW FILE
*/

/*
THINGS NEED TO BE DONE:
  CREATE RESEND FILE IF CONNECTION FAILS
  CREATE TIMEOUT FOR DDOS ATTACKS OR FAILED CONNECTIONS
*/
class Server
{
public:
  Server();
  char *Receive_Msg();    //Function to practice sending messages
  std::string Stand_by(); //To find what type of operation the client_socket wants the server to peform
  //std::string Read_file();// Recieves the name of the file the client_socket wants
  void Send_file(std::string &file); // Sends the file the client_socket wants
  std::string Get_filename(); //Gets the name of the File it is meant to create
  std::string Get_Content(); //Gets the Content of the file
  void Close_connection();
  bool Get_handshake();
  void Server_init();
private:
  fd_set readio;
  vector<SOCKET> client_list;//This will hold onto all the clients it connected to
  SOCKET server_socket;
  SOCKET client_socket;
  bool handshake_;
  SOCKET active_client;
  int iResult;
  char buffer[BUFFER_SIZE]={0};
  int buffer_size=BUFFER_SIZE;
  struct addrinfo *result=NULL,*ptr=NULL,hints;

  void Clear_buffer();
  void Bind();            //Binds the server to a PORT
  void Listen();          //Listens for incoming sockets
  void Create_Server();   //Creates the server socket
  void Remove_socket(SOCKET peer);
  void Handshake()
  {
    printf("staring Handshake...\n");
    Header clienth;
    Header serverh;
    iResult = recv(client_socket, (char*)&clienth, sizeof(clienth), 0);
    if (clienth.syn == 1 )
    {
        serverh.ack = 1;
        serverh.syn = 1;
        iResult = send(client_socket, (char*)&serverh, sizeof(serverh), 0);
        iResult = recv(client_socket, (char*)&clienth, sizeof(clienth), 0);
        if (clienth.ack == 1)
        {
          printf("it worked!\n");
          handshake_=true;
        }
        else {
            printf("it Failed!\n");
            handshake_ = false;
        }
    }
    else {
        printf("it Failed!\n");
        handshake_ = false;
    }
  }


};

Server::Server()
{
  WSADATA wsaData;
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0)
  {
    printf("WSASstartup failed: %d\n", iResult);
  }
}
void Server::Create_Server()
{
    printf("Creating Server\n");
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
  if (iResult !=0 )
  {
      printf("getaddinfo failed: %d\n", iResult);
      WSACleanup();
      return;
  }
  server_socket = INVALID_SOCKET;//this is the int for the socket
  server_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if(server_socket==INVALID_SOCKET){
    printf("failed to create socket\n");
  }
  printf("Created Server\n");
}
void Server::Bind()
{
  iResult = bind(server_socket, result->ai_addr, (int)result->ai_addrlen);
  if(iResult == SOCKET_ERROR)
  {
      printf("failed to bind the socket: %d\n", WSAGetLastError());
      freeaddrinfo(result);
      closesocket(server_socket);
      WSACleanup();
      return;

  }
  printf("Binded Server\n");
  //freeaddrinfo(result);
}
void Server::Listen()
{
  printf("Listening for connections...\n");
  if(listen(server_socket,SOMAXCONN) == SOCKET_ERROR)
    {
      printf("Error with listening : %d\n", WSAGetLastError());
      closesocket(server_socket);
      WSACleanup();
      //return EXIT_FAILURE;
    }

}
 char* Server::Receive_Msg()
{
    int Sendresults;

      iResult = recv(client_socket, buffer, buffer_size, 0);
      if (iResult > 0) {
          Sendresults = send(client_socket, buffer, buffer_size, 0);
          if (Sendresults == SOCKET_ERROR) {
              printf("Error sending things to client_socket: %d\n", WSAGetLastError());
              closesocket(client_socket);
              WSACleanup();
              //return EXIT_FAILURE;
          }
      }
      else if (iResult == 0)
          printf("Connection over\n");
      else {
          printf("Failed to recieve message from client_socket: %d\n", WSAGetLastError());
          closesocket(client_socket);
          WSACleanup();
          return buffer;
      }

  printf("Message from server is: %s\n", buffer);
  return buffer;
}

 std::string Server::Stand_by()
 { // This is meant to recieve commands from the client_socket about operations
     while (TRUE)
     {
         FD_ZERO(&readio);
         FD_SET(server_socket, &readio);
         SOCKET max_sd = server_socket;
         //std::cout<<"client size is: "<<client_list.size()<<std::endl;
         for (int i = 0; i < client_list.size(); i++)
         {
             SOCKET sd = client_list[i];
             if (sd > 0)
             {
                 FD_SET(sd, &readio);
                 if (sd > max_sd)
                 {
                     max_sd = sd;
                 }
             }
         }

         Header serverh;
         Header clienth;

         int activity = select(max_sd + 1, &readio, NULL, NULL, NULL);
         if (FD_ISSET(server_socket, &readio)) {//Connects new clients since FD_SET is listening for connections

             client_socket = accept(server_socket, NULL, NULL);//Connects the client, that is now on the client list
             std::cout << "Connecting socket:\t" << client_socket << std::endl;
             if (client_socket == SOCKET_ERROR)
             {
                 printf("Error accepting client: %d\n", WSAGetLastError());
                 closesocket(client_socket);
                 WSACleanup();
                 return "";
             }
             Handshake();
             if (handshake_ == false) {std::cout<<"handshake failed\n"; Close_connection(); }
             handshake_=false;

             client_list.push_back(client_socket);
             
         }
         if (client_socket == INVALID_SOCKET) { std::cout << "Socket is invalid\n"; }
         else
         {
         for (int i=0;i<client_list.size();i++)
         {
             //std::cout << i << std::endl;
             char buff[BUFFER_SIZE] = { 0 };
             SOCKET peer=client_list[i];
             if (FD_ISSET(peer,&readio))
             {
                 iResult = recv(peer, buff, buffer_size, 0); // recieves the operation READ/Write
                 if (iResult == SOCKET_ERROR) { 
                     std::cout << "Connection closed, disconnection socket: " << peer << std::endl;
                     Remove_socket(peer);
                     closesocket(peer);
                     break;
                 }
                 std::cout << "Recieved command from\t" << peer << std::endl;
                 printf("Message is: %s\n", buff);
                 /*if(iResult==SOCKET_ERROR){
                   printf("Error sending things to client_socket: %d\n", WSAGetLastError());
                 }*/
                 serverh.ack = 1;
                 iResult = send(peer, (char*)&serverh, sizeof(serverh), 0);//Sends ack that recieved operation

                 if (!strcmp(buff, "read"))//Informs the main.cpp that client wants read command
                 {
                     client_socket=peer;
                     Clear_buffer();
                     return "read";
                 }
                 else if (!strcmp(buff, "write"))
                 {
                     client_socket=peer;
                     Clear_buffer();
                     return "write";
                 }
                 else if (!strcmp(buff, "list")) {
                     client_socket=peer;
                     return "list";
                 }
             }

         }
     }
   }
 }

 void Server::Remove_socket(SOCKET peer){
     vector<SOCKET> temp;
     for (SOCKET i : client_list) {
         if (i != peer) {
             temp.push_back(i);
         }
     }
     client_list.clear();
     client_list = temp;
 }


/*
std::string Server::Read_file() //This is meant to be used to recieve the name of the file the client_socket wants.
{
  Header serverh;
  Clear_buffer();
  //wait for files name
  iResult=recv(client_socket,buffer,buffer_size,0);//Name of the file client wants to read
  if(iResult==0)
  {
    printf("failed to recieve filesname\n");
    std::string empty="";
    Clear_buffer();
    return empty;
  }
  serverh.ack=1;
  iResult=send(client_socket,(char*)&serverh,sizeof(serverh),0);//Sends the client ack that recieved the name of the file
  std::string filename(buffer);
  Clear_buffer();
  return filename;

}*/
void Server::Clear_buffer(){
  for(int i=0;i<buffer_size;i++){
    buffer[i]=0;
  }
}
void Server::Send_file(std::string &file){ //Send the file to the client_socket once it finds it
  Header serverh;
  Header clienth;
  std::cout<<file<<std::endl;
  const char* message = file.c_str();
  iResult=send(client_socket,message,(int)strlen(message),0);//Might have to loop to send it all
  /*if(iResult==SOCKET_ERROR){
    printf("Error sending things to client_socket: %d\n", WSAGetLastError());
  }*/
  iResult=recv(client_socket,(char*)&clienth,sizeof(clienth),0);//Waits for ack from client that it recieved the file
  /*if(iResult==SOCKET_ERROR){
    printf("Error recv things to client_socket: %d\n", WSAGetLastError());
  }*/
  if(clienth.ack==1){
    printf("Client recieved data!\n");
  }
  else
  {
    printf("Client failed to recieve file\n");
  }
}
std::string Server::Get_filename()
{
  Header serverh;
  Clear_buffer();
  iResult=recv(client_socket,buffer,buffer_size,0);//Recives the Name of the File it will create
  serverh.ack=1;
  iResult=send(client_socket,(char*)&serverh,sizeof(serverh),0);//sends client ack that recieved the name of the file
  std::string filename(buffer);
  return filename;
}
std::string Server::Get_Content(){//Create a protocol to see check the size of content, in case content is bigger than BUFFER_SIZE
  Header serverh;
  Clear_buffer();
  iResult=recv(client_socket,buffer,buffer_size,0);//Receives the content it is meant to put into the file. This might have to be looped if contet is to big
  serverh.ack=1;
  iResult=send(client_socket,(char*)&serverh,sizeof(serverh),0);//Sends client ACK that recieved all the contents of the files
  std::string content(buffer);
  return content;
}
void Server::Close_connection(){
  closesocket(client_socket);
  printf("Connection closed\n");
}
bool Server::Get_handshake() {
    return handshake_;
}
void Server::Server_init(){
    Create_Server();
    Bind();
    Listen();
}
#endif
