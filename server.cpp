#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <fstream>
#include <iostream>
#include <conio.h>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <mutex>
#include <memory>
#include "server.h"
#include "header.h"

using std::fstream;
using std::ios;
using std::vector;
using std::string;
using std::find;

int FinishWork(const char* errorMessage, SOCKET& mySocket);

Server::Server()
{
	number_of_clients_ = 0;
}
// call of a new thread for the maintenance of client
int Server::WaitForClientsConnection()
{
	printf("Server is waiting for connection...\n");
    // extraction the messages from the queue
    SOCKET clientSocket; // socket for client
    sockaddr_in clientAddress; // address of client
    int client_addr_size = sizeof(clientAddress);
    // requests extract a connection from the queue
    std::thread* p_newThread;
    while( (clientSocket = accept(my_socket_, (sockaddr *)&clientAddress, &client_addr_size)) )
    {
        AddClient(); // increase the meter is connected clients
        HOSTENT *hst;
        hst = gethostbyaddr( (char*)&clientAddress.sin_addr.s_addr, 4, AF_INET);
		// output information about client
        printf("+%s [%s] new connect!\n",
               (hst)?hst->h_name : "",
               inet_ntoa(clientAddress.sin_addr));

        // call of a new thread for the maintenance of client
        try
        {
            p_newThread = new std::thread( &ServiceClient, this, &clientSocket );
            p_newThread->detach();
            delete p_newThread;
        }
        catch(std::bad_alloc)
        {
            send(clientSocket, (char*)&kOverloadedServerFlag, 1, 0);
        }
    }
    p_newThread = nullptr;
    return 0;
}

// Send piece of file or flag to client.
// If server send piece of file the function returns 0
// else the function returns this flag.
int Server::SendPieceOfFile(char* pBuffer, int& bufferLength, char* fileName, fstream& File, SOCKET& Socket)
{
	if (find(editing_files_.begin(), editing_files_.end(), fileName) != editing_files_.end())
	{
		send(Socket, (char*)&kUsingOtherClientFlag, 1, 0);
		File.close();
		return kUsingOtherClientFlag;
	}
	// if file is not exists
	if (!File.is_open())
	{
		send(Socket, (char*)&kNoFileFlag, 1, 0);
		File.close();
		return kNoFileFlag;
	}
	if ( !AddEditingFile(fileName) )
    {
        File.close();
        send(Socket, (char*)&kOverloadedServerFlag, 1, 0);
        return kOverloadedServerFlag;
    }

	File.seekg(0, ios::beg);

	// send file
	File.read(pBuffer, kPieceFileSize);

	if ((bufferLength = File.gcount())) // if file is not empty
	{
		send(Socket, pBuffer, bufferLength, 0);
	}
	else
	{
		send(Socket, (char*)&kEmptyFileFlag, 1, 0);
	}
	return 0;
}

// get piece of file from client and update file on server
void Server::UpdateFile(char* pBuffer, int& bufferLength, const char* fileName, fstream& File, SOCKET& Socket)
{
    File.open(fileName, ios::out | ios::in | ios::ate);
    File.seekp(0, ios::beg);
    bufferLength = recv(Socket, pBuffer, kPieceFileSize, 0); // get edited file
    if ( pBuffer[0] != kEmptyFileFlag ) // if file is not empty
    {
        File.write(pBuffer, bufferLength);
    }
    File.close();
    SubtractEditingFile(fileName);
}

// send needed for client piece of file, update it on server
int Server::ServiceClient(void* clientSocket)
{
    char* fileName = nullptr; // if the pointer will be deleted nothing will happen
    char *buffer;
    int bufferLength;
    SOCKET socketForCurrentClient;
	socketForCurrentClient = *(SOCKET*)clientSocket;

    // getting file name from client
    bool isOverloadedServer = false;

    while ( !number_of_clients_mutex_.try_lock() )
        {
            ;
        }
    try
    {
        if ( number_of_clients_ == maxNumberOfClient )
        {
            number_of_clients_mutex_.unlock();
            throw std::bad_alloc();
        }
        number_of_clients_++;
        number_of_clients_mutex_.unlock();
        fileName = new char[kMaxFileNameLen + 1]; // +1 for '\0'
        buffer = new char[kPieceFileSize];
    }
    catch(std::bad_alloc)
    {
        delete[] fileName; // if enough space for fileName then delete filename
        send(socketForCurrentClient, (char*)&kOverloadedServerFlag, 1, 0);
        isOverloadedServer = true;
    }

    if ( isOverloadedServer )
    {
        return -1;
    }

    int fileNameLen;
	while ((fileNameLen = recv(socketForCurrentClient, fileName, kMaxFileNameLen, 0)) && (fileNameLen != SOCKET_ERROR))
    {
        fileName[fileNameLen] = '\0';
        printf("%s\n",fileName);
        fstream File;
        File.open(fileName, ios::out | ios::in | ios::ate | ios::binary); // file, that needed for client
        // if file is using by other client
        int flagForClient = SendPieceOfFile(buffer, bufferLength, fileName, File, socketForCurrentClient);
        if ( flagForClient == kOverloadedServerFlag )
        {
            break;
        }
		if ( flagForClient == kNoFileFlag || flagForClient == kUsingOtherClientFlag )
        {
            File.close();
            continue;
        }
        File.close();
		UpdateFile(buffer, bufferLength, fileName, File, socketForCurrentClient);
    }

    // decrement the counter of active clients
    SubtractClient();
    delete[] fileName;
    delete[] buffer;
	closesocket(socketForCurrentClient);
    printf("-disconnect\n");
    fileName = nullptr;
    buffer = nullptr;
    return 0;
}

// Initialization the sockets' library and creation of socket with TCP protocol.
// if the function finished is successfully the function returns 0
// else returns -1.
int Server::Initialization()
{
    char buffer[1024];
	// Initialization the sockets' library
	if (WSAStartup(0x0202, (WSADATA *)buffer)) // if error
	{
		printf("Error number %d in WSAStartup\n", WSAGetLastError());
		return -1;
	}

	// Creation of socket with TCP protocol
	if ((my_socket_ = socket(AF_INET, SOCK_STREAM, 0)) < 0) // if error
	{
		return FinishWork("socket", my_socket_);
	}
	return 0;
}

// Binding the socket to a local address.
// If the function finished is successfully the function returns 0
// else returns -1.
int Server::Binding()
{
	local_address_.sin_family = AF_INET;
	local_address_.sin_port = htons(kMY_PORT);
	local_address_.sin_addr.s_addr = 0;
	// if error
	if (bind(my_socket_, (sockaddr *)&local_address_, sizeof(local_address_)))
	{
		return FinishWork("bind", my_socket_);
	}
	return 0;
}

// Waiting for connection.
// If the function finished is successfully the function returns 0
// else returns -1.
int Server::Listening()
{
	// Waiting for connection, queue size - 512
	const int queueSize = 512;
	if (listen(my_socket_, queueSize)) // if error
	{
		return FinishWork("listen", my_socket_);
	}
	return 0;
}

// prepare server to work
int Server::StartServer()
{
    printf("This is a server\n");
	if (Initialization() == -1)
	{
		return -1;
	}
	if (Binding() == -1)
	{
		return -1;
	}
	if (Listening() == -1)
	{
		return -1;
	}
    WaitForClientsConnection();
    return 0;
}

// safe add count of clients
void Server::AddClient()
{
    while( !number_of_clients_mutex_.try_lock() )
    {
        ;
    }
    number_of_clients_++;
    number_of_clients_mutex_.unlock();
}

// safe subtract count of clients
void Server::SubtractClient()
{
    while( !number_of_clients_mutex_.try_lock() )
    {
        ;
    }
    number_of_clients_--;
    number_of_clients_mutex_.unlock();
}

// safe add file to editingFiles
bool Server::AddEditingFile(const string& newEditingFile)
{
    while( !number_of_clients_mutex_.try_lock() )
    {
        ;
    }
    if ( editing_files_.size() == editing_files_.max_size() )
    {
        editing_files_mutex_.unlock();
        return false;
    }
    editing_files_.push_back(newEditingFile);
    editing_files_mutex_.unlock();
    return true;
}

// safe delete file from editing files
void Server::SubtractEditingFile(const string& editingFile)
{
    while( !editing_files_mutex_.try_lock() )
    {
        ;
    }
    editing_files_.erase( find(editing_files_.begin(), editing_files_.end(), editingFile)); // delete this file from editing files
    editing_files_mutex_.unlock();
}
