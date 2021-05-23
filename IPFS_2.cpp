// IPFS_2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include <iostream>
#include <thread>
#include "client.h"
#include "server.h"
#include <Windows.h>
#include "request.h"
void Start_server(std::string port) {
   
    Server server(port);
    server.Start_server();
}
void Start_client(std::string port) {
    Sleep(5000);
    Sender client(port);
    client.Client_start();
}
int main()
{
    std::cout << "Enter port of node:\t";
    std::string port;
    std::cin >> port;
    std::thread server(Start_server, port);
    std::thread client(Start_client,port);
    server.join();
    client.join();
    return 0;
}