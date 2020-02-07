#include <stdio.h>
//#include <winsock2.h>
//#include <windows.h>
#include <fstream>
#include <iostream>
#include <conio.h>
#include "client.h"
#include "server.h"
//#include "header.h"
#include "application.h"

extern HANDLE hConsole;

using std::fstream;
using std::ios;
using std::cout;
using std::string;

// print error message and finish work fo client or server
int FinishWork(const char* errorMessage, SOCKET& mySocket)
{
    printf("Error number %d in %s\n", WSAGetLastError(), errorMessage);
    WSACleanup(); // uninitialization the Winsock library
    return -1;
}

int main(int argc, char* argv[])
{
    Application a;
    a.SettingFont();
    a.ShowMenu();
    return 0;
}
