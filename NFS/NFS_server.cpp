#include "filesys.h"
#include "server.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main(int argc, char const* argv[]) {
    FileSys fs("FileSystem.txt", 8, 500, "0");// Move this to the server.h
    Server s;
    s.Server_init();
    //Initiate a client inside of Server.h by doing this you can send the wanted files to all nodes
    //Allowing you to be connected to all of them.
    //Requirement for client to send data= PORT in which we will send the data.
    //Have a premade list of known port.
    //Have another program that acts as a new node asking for information
    //Since client sockets are just used to send data, i could close them once i was done.
    //Leave the server socket open
    //How would client socket share data with server socket?
    //Client socket would be initiated inside the server and the server would feed the client the strings
    //It is meant to send. This way we can have 1 main.cpp running the whole program and no need for a client/server.
    //How would user interact with each node if server always running?
    //Threads? have both server up recieving data and sending?
    //Server is always on a constant state of listening, having users interacting with the server sockets
    //Won't allow server to await for connecting sockets.
    //If we wont have user interaction why have server node with client in it. Create a main with thread for both client and server at the same time
    //Thread in main.cpp can maybe fix this, but im sure of how the interaction would look like
    //Reason to have client and server together
    //WHEN CLIENT WILL BE USED:
    //-request blocks of data
    //-Write new file into the FS
    //
    //
    //WHEN SERVER WILL BE USED:
    //-Wait for a Connection
    //-Send data requested by client
    //-Add something new to the FS
    //
    //
    //Reason to have them together is that each node is allowed to update the FS if needed
    //They may add files, which will cause the client node to send information to everyone to update.
    //The client node will have to update itself too if client and server are seperate threads
    //If they are sharing the same folder and memory space they should have access to the same file
    //Filesystem does not allow for an empty creation
    while (1)//Turn this into a function for the class---Maybe add this to Stand_by
    {
        string cmd = s.Stand_by();
        if (cmd == "read") {
            string fname = s.Get_filename();
            cout << "Looking for content\n";
            string content = fs.readFile(fname);
            s.Send_file(content);
        }
        else if (cmd == "write") {
            string fname = s.Get_filename();
            string content = s.Get_Content();
            fs.createFile(fname, content);
        }
        else if (cmd == "list") {
            vector<string> files=fs.ls_Files();
            string all_files;
            for (string file : files) {
                all_files += file;
            }
            s.Send_file(all_files);
        }
        else {
            //cout << "Invalid Command\n";
            //s.Close_connection();
            //break;
            }
    }
    // while(1){
    //     string cmd;
    //     getline(cin,cmd);
    //     vector<string> options;
    //
    // }
    return 0;
}
