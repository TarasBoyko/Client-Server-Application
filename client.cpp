#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <string.h>
#include <conio.h>
#include <locale>
#include <iostream>
#include <vector>
#include "client.h"
#include "header.h"

#define SERVERADDR "127.0.0.1"

int FinishWork(const char* errorMessage, SOCKET& mySocket);

// Initialization the sockets' library.
// If the function finished is successfully the function returns 0
// else returns -1.
int Client::Initialization()
{
    printf("This is a client\n");
    // Initialization the sockets' library
    if ( WSAStartup(0x202, (WSADATA *)buffer_) ) // if error
    {
        printf("Error number %d in WSAStart\n", WSAGetLastError());
        return -1;
    }

    my_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (my_socket_ < 0) // if error
    {
        return FinishWork("socket", my_socket_);
    }
    return 0;
}

// Connection setting. Indication the address and port of the server
// and convert IP-addresses from a character in a network format.
// If the setting is seccessfully the function returns 0
// else returns -1.
int Client::ConvertIPaddr()
{
    // indication the address and port of the server
    dest_addr_.sin_family = AF_INET;
    dest_addr_.sin_port   = htons(kMY_PORT);
    HOSTENT *hst;
    // convert IP-addresses from a character in a network format
    if (inet_addr(SERVERADDR) != INADDR_NONE) // if all OK
    {
        dest_addr_.sin_addr.s_addr = inet_addr(SERVERADDR);
    }
    else // attempt to obtain an IP address for the domain name server
    {
        if ( (hst = gethostbyname(SERVERADDR)) )
        {
            ((unsigned long *)&dest_addr_.sin_addr)[0] = ((unsigned long **)hst->h_addr_list)[0][0];
        }
        else
        {
            return FinishWork("gethostbyname", my_socket_);
        }
    }
    return 0;
}

// Connect to server.
// If connection is seccessfully the function returns 0
// else returns -1.
int Client::Connection()
{
    // server address is obtained. connect to server
    if ( connect(my_socket_, (sockaddr *)&dest_addr_, sizeof(dest_addr_)) ) // if error
    {
        return FinishWork("connect", my_socket_);
    }
    printf("Connection with %s installed\n", SERVERADDR);
    return 0;
}

// send edited piece of file to server
void Client::SendEditedPieceOfFile()
{
    copy(console_piece_file_editor_.text_.begin(), console_piece_file_editor_.text_.end(), buffer_);
    if ( console_piece_file_editor_.text_.size() )
    {
        send( my_socket_, buffer_, console_piece_file_editor_.text_.size(), 0 );
    }
    else
    {
        buffer_[0] = kEmptyFileFlag;
        send( my_socket_, buffer_, 1, 0 );
    }
    console_piece_file_editor_.absolute_position_ = 0;
    console_piece_file_editor_.text_.clear();
}

// Get piece of file or flag from server.
// If client get piece of file the function returns 0.
// If client get flag the function returns this flag.
// If the connection is unexpectedly terminated the function returns -1.
int Client::GetPieceOfFile()
{
    if ( (buffer_length_ = recv(my_socket_, buffer_, kPieceFileSize, 0)) == SOCKET_ERROR )
    {
        printf("The connection is unexpectedly terminated.\n"
               "Press any key");
        CLEAR_BUFFER
        getch();
        return -1;
    }
    console_piece_file_editor_.absolute_position_ = 0;
    if ( buffer_[0] == kOverloadedServerFlag )
    {
        printf("Server is overloaded now.\n"
               "Press any key");
        CLEAR_BUFFER
        getch();
        return kOverloadedServerFlag;
    }
    if ( buffer_[0] == kUsingOtherClientFlag ) // if file is using by other client
    {
        printf("This file is using by other client. Please, try again later.\n");
        return kUsingOtherClientFlag;
    }
    // if file is not exists
    if ( buffer_[0] == kNoFileFlag )
    {
        printf("This file is not exists.\n");
        return kNoFileFlag;
    }
    // if get empty file
    if ( buffer_[0] == kEmptyFileFlag )
    {
        buffer_[0] = '\0';
        console_piece_file_editor_.absolute_position_ = 0;
        console_piece_file_editor_.text_.clear();
        return kEmptyFileFlag;
    }
    // if get not empty file
    console_piece_file_editor_.assign(buffer_, buffer_ + buffer_length_);
    //bufferForEdit.assign(buffer, buffer + bufferLength);
    buffer_[buffer_length_] = '\0';
    console_piece_file_editor_.absolute_position_ = buffer_length_;
    return 0;
}

// User enter file name, get piece of this file from server, edit it
// and send edited piece of file to server.
int Client::EditServersFiles()
{
    printf("write \"quit\" for quit\n\n");
    while( EnterFileName() != -1 )
    {
        // send file name
        send(my_socket_, buffer_, buffer_length_, 0);
        char flagFromServer;
        flagFromServer = GetPieceOfFile();
        // if the connection is unexpectedly terminated
        if ( flagFromServer == -1 || flagFromServer == kOverloadedServerFlag )
        {
            return flagFromServer;
        }
        if ( flagFromServer == kNoFileFlag || flagFromServer == kUsingOtherClientFlag )
        {
            continue;
        }
        // output geted piece of file
        printf("%s", buffer_);
        EditPieceOfFile();
        printf("\n");
        SendEditedPieceOfFile();
    }
    return 0;
}

// editing piece of file at conlole mode
void Client::EditPieceOfFile()
{
    char c;
    while ( (c = getch()) != kEOKB )
    {
        // if bufferForEdit is overloaded client gets an audio message
        if ( console_piece_file_editor_.text_.size() == console_piece_file_editor_.text_.max_size() )
        {
            printf("\a");
            continue;
        }
        // if user press left arrow key
        if ( GetAsyncKeyState(VK_LEFT) != 0 )
        {
            console_piece_file_editor_.MoveBack(1, true);
            CLEAR_BUFFER
            continue;
        }
        // if user press right arrow key
        if ( GetAsyncKeyState(VK_RIGHT) != 0 )
        {
            console_piece_file_editor_.MoveForward(1, true);
            CLEAR_BUFFER
            continue;
        }
        switch ( c )
        {
        case 8:
            console_piece_file_editor_.DeletePreviousSymbol();
            break;
        case 13 : // if user press Enter
            console_piece_file_editor_.PrintNewLineSymbol();
            break;
        default:
            console_piece_file_editor_.PrintGraficSymbol(c);
        }
    }
}

// Get file name from user and put it into buffer(field of Client-class).
// If user print "quit" the function returns -1
// else the function returns 0.
int Client::EnterFileName()
{
    printf("Enter file name ");
    CLEAR_BUFFER
    gets(buffer_);
    if ( strcmp(buffer_, "quit") == 0 ) // if client wrote "quit"
    {
        return -1;
    }
    ;
    while ( (buffer_length_ = strlen(buffer_)) < 1 || buffer_length_ > kMaxFileNameLen ) // if File name length is out of range
    {
        printf("File name length is out of range.\n"
               "Enter file name(min length - 1 symbol, max length - 256 symbols) ");
        gets(buffer_);
    }
    buffer_[buffer_length_] = '\0';
    return 0;
}

// prepare client to work and start edit server's files
int Client::ConnectToServer()
{
    if (Initialization() == -1)
    {
        return -1;
    }
    if (ConvertIPaddr() == -1)
    {
        return -1;
    }
    if (Connection() == -1)
    {
        return -1;
    }
    return EditServersFiles();
}

template <class T1, class T2>
void F()
{
	T1 t;
	T2 n;
}

template <class T2>
void F<int, T2>()
{

}