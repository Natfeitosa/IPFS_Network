// NFS_client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "client.h"
#include "header.h"

int main()
{
	Sender client;
	client.Connect_client();
	std::cout << "Client was created!\n";
	//std::string filename = "filesystem";
	client.WriteFile("systemfile", "First message");
	std::cout << "File was written\n";
	std::string filename = "systemfile";
	std::string content = client.Read(filename);
	std::cout << content << std::endl;
	std::cout << "entering wait stage...\n";
	while (1) {
		std::cout << "What other operation you would like to do?\n";
		std::string command;
		std::cin >> command;
		if (command == "read") {
			std::cout << "what is the name of the file?\n";
			std::cin >> filename;
			content = client.Read(filename);
		}
		else if (command == "write") {
			std::cout << "what is the name of the file?\n";
			std::cin >> filename;
			std::cout << "What is the content of the file?\n";
			std::cin >> content;
			client.WriteFile(filename, content);
		}
		else {
			std::string list = client.list();
			std::cout << "List of available files:\t" << list << std::endl;
		}
	}

	
	
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
