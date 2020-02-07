#include "Application.h"
#include <windows.h>
#include <locale>
#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <string>
#include "server.h"
#include "client.h"

HANDLE hConsole;

void Application::SettingFont()
{
    typedef struct _CONSOLE_FONT_INFOEX {
    ULONG cbSize;
    DWORD nFont;
    COORD dwFontSize;
    UINT  FontFamily;
    UINT  FontWeight;
    WCHAR FaceName[LF_FACESIZE];
    } CONSOLE_FONT_INFOEX, *PCONSOLE_FONT_INFOEX;

    typedef BOOL (WINAPI *SETCURRENTCONSOLEFONTEX)(HANDLE, BOOL, PCONSOLE_FONT_INFOEX);
    SETCURRENTCONSOLEFONTEX SetCurrentConsoleFontEx;

    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    setlocale(LC_ALL,"");

    HMODULE hmod = GetModuleHandle("KERNEL32.DLL");
    SetCurrentConsoleFontEx = (SETCURRENTCONSOLEFONTEX)GetProcAddress(hmod, "SetCurrentConsoleFontEx");
    if (!SetCurrentConsoleFontEx) {
        std::cout << "error" << std::endl;
        exit(1);
    }

    CONSOLE_FONT_INFOEX font;
    ZeroMemory(&font, sizeof(CONSOLE_FONT_INFOEX));
    font.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    wcscpy(font.FaceName, L"Lucida Console");
    font.dwFontSize.X = 10;
    font.dwFontSize.Y = 16;

    if (!SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), false, &font)) {
        std::cout << "error" << std::endl;
        exit(2);
    }


    if ( (hConsole = GetStdHandle(STD_OUTPUT_HANDLE)) == INVALID_HANDLE_VALUE )
    {
        getch();
    }
}

void Application::ShowMenu()
{
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
    GetConsoleScreenBufferInfo(hConsole, &csbiInfo); // read ussual console text attribute
    std::string Menu[] = {"Server", "Client"};
    unsigned curMenuItem = 0; // current menu item
    while (true)
    {
        system("cls");

        for ( unsigned i = 0; i < 2u; i++)
        {
            if ( i == curMenuItem )
            {
                SetConsoleTextAttribute(hConsole, ( 0xA | (0xE << 4)) ); //green on yellow background
            }
            else
            {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x0| (0xE << 4)); //black on yellow background
            }
            printf("%s\n", Menu[i].c_str());
        }
        SetConsoleTextAttribute(hConsole, csbiInfo.wAttributes); // set ussual console text attribute

        while ( true )
        {
            if ( GetAsyncKeyState(0x26) != 0 ) // if user press UP arrow key
            {
                if ( curMenuItem == 0 )
                    break;
                curMenuItem = curMenuItem - 1;
                break;
            }
            if ( GetAsyncKeyState(0x28) != 0 ) // if user press Down arrow key
            {
                if ( curMenuItem == 1 )
                    break;
                curMenuItem = curMenuItem + 1;
                break;
            }
             if ( GetAsyncKeyState(0x0D) != 0 ) // if user press Down arrow key
            {
                Server newServer;
                Client newClient;
                system("cls");
                switch ( curMenuItem )
                 {
                 case 0 :
                    exit( newServer.StartServer() );
                    break;
                 case 1 :
                    exit( newClient.ConnectToServer() );
                    break;
                 default :
                     ;
                 }
            }
        }
    }
}
