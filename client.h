#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include <iostream>
#include <vector>
#include <winsock2.h>
#include <windows.h>

#include "header.h"
#include "consoleeditor.h"

using std::vector;

class Client
{
public:
    int ConnectToServer();

private:
    // Socket with TCP protocol
    SOCKET my_socket_;
    sockaddr_in dest_addr_;
    consoleEditor console_piece_file_editor_;

    char buffer_[kPieceFileSize + 1]; // Buffer for different needs. +1 for '\0'
    int buffer_length_;

    int Initialization();
    int ConvertIPaddr();
    int Connection();
    int EditServersFiles();
    int EnterFileName();
    int GetPieceOfFile();
    void EditPieceOfFile();
    void SendEditedPieceOfFile();
};

#endif // CLIENT_H_INCLUDED
