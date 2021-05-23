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
THINGS LEFT TO DO, ADD ACK AFTER SENDING AND RECIEVING EACH MESSAGE
*/

/*
THINGS NEED TO BE DONE:
  CREATE RESEND FILE IF CONNECTION FAILS
  CREATE TIMEOUT FOR DDOS ATTACKS OR FAILED CONNECTIONS
*/
class Server
{
public:
  Server(std::string port);
  char *Receive_Msg();    //Function to practice sending messages
  std::string Stand_by(); //To find what type of operation the client_socket wants the server to peform
  //std::string Read_file();// Recieves the name of the file the client_socket wants
  void Send_file(std::string &file); // Sends the file the client_socket wants
  std::string Get_filename(); //Gets the name of the File it is meant to create
  std::string Get_Content(); //Gets the Content of the file
  void Close_connection();
  bool Get_handshake();
  void Server_init();
  void Start_server();
private:
  FileSys fs;
  std::string port_;
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
  std::string Get_write_block();
  void Send_addr(std::string addr);
  int Get_previous_block();
  string Get_new_addr();
  int Get_block_number();
  void Send_block_content(string block_content);
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

Server::Server(std::string port): port_{port},fs("FileSystem.txt", 8, 500, port)
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

  iResult = getaddrinfo(NULL, port_.c_str(), &hints, &result);
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
                 else if(!strcmp(buff,"write_block")){
                     client_socket = peer;
                         return "write_block";
                 }
                 else if(!strcmp(buff, "set_next_addr")){
                     client_socket = peer;
                     return "set_next_addr";
                 }
                 else if (!strcmp(buff, "block_content")) 
                 {
                     client_socket = peer;
                         return "block_content";
                 }
                 else if (!strcmp(buff, "block_ptr"))
                 {
                     client_socket = peer;
                     return "block_ptr";
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
  std::string filename_(buffer);
  return filename_;
}
std::string Server::Get_Content(){//Create a protocol to see check the size of content, in case content is bigger than BUFFER_SIZE
  Header serverh;
  Clear_buffer();
  iResult=recv(client_socket,buffer,buffer_size,0);//Receives the content it is meant to put into the file. This might have to be looped if contet is to big
  serverh.ack=1;
  iResult=send(client_socket,(char*)&serverh,sizeof(serverh),0);//Sends client ACK that recieved all the contents of the files
  std::string content_(buffer);
  return content_;
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

void Server::Start_server(){
    Server_init();
    
    std::vector<string> ports = {"8080","8081"};
    for (string i : ports) {
        if (i != port_) {
            fs.addPort(i);
        }
    }
  while (1)//Turn this into a function for the class---Maybe add this to Stand_by
  {

      string cmd = Stand_by();
      if (cmd == "read") {
          string fname = Get_filename();
          cout << "Looking for content\n";
          string content_ = fs.readFile(fname);
          Send_file(content_);
      }
      else if (cmd == "write") {
          string fname = Get_filename();
          string content_ = Get_Content();
          fs.createFile(fname, content_);
      }
      else if (cmd == "list") {
          vector<string> files=fs.ls_Files();
          string all_files;
          for (string file : files) {
              all_files += file;
          }
          Send_file(all_files);
      }
      else if(cmd=="write_block"){
          string file_content=Get_write_block();
          string addr=fs.writeBlock_p(file_content);
          std::cout << "This is the server, this is the addr: " << addr << std::endl;
          Send_block_content(addr);
          }
      else if (cmd == "set_next_addr") {
          
          int Prev_block = Get_previous_block();
          string new_addr = Get_new_addr();
          fs.setNextAddr(Prev_block, new_addr);
      }
      else if (cmd == "block_content") {
          int block1 = Get_block_number();
          string content_block = fs.getBlockContent(block1);
          Send_block_content(content_block);
      }

      else if (cmd == "block_ptr") {
          int block_num = Get_block_number();
          string block_ptr_ = fs.getBlockAddr(block_num);
          Send_block_content(block_ptr_);
      }

      else{}
  }

  return;
}
void Server::Send_block_content(string block_content) {
    iResult = send(client_socket, block_content.c_str(), block_content.size(), 0);
}

int Server::Get_block_number() {
    char buff_[BUFFER_SIZE] = { 0 };
    iResult = recv(client_socket, buff_, BUFFER_SIZE, 0);
    string blockstr(buff_);
    int block_num = stoi(blockstr);
    return block_num;
}
std::string Server::Get_write_block(){
  char buff_[BUFFER_SIZE]={0};
  iResult=recv(client_socket,buff_,BUFFER_SIZE,0);
  std::string block_content(buff_);
  return block_content;
}
void Server::Send_addr(std::string addr){

}
int Server::Get_previous_block() {
    char buff_[BUFFER_SIZE] = { 0 };
    iResult=recv(client_socket, buff_, BUFFER_SIZE, 0);
    std::string block_(buff_);
    int prev_block = stoi(block_);
    return prev_block;
}
string Server::Get_new_addr() {
    char buff[BUFFER_SIZE] = { 0 };
    iResult = recv(client_socket, buff, BUFFER_SIZE, 0);
    string new_addr(buff);
    return new_addr;
}
#endif
