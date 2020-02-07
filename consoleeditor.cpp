#include <stdio.h>
#include <windows.h>
#include <algorithm>
#include "consoleeditor.h"
#include "header.h"
#include "application.h"

extern HANDLE hConsole;





consoleEditor::consoleEditor()
{
    absolute_position_ = 0;
    verticalPosition = 0;
}
void consoleEditor::assign(char* begArray, char* afterEndArray)
{
    absolute_position_ = afterEndArray - begArray;
    text_.assign(begArray, afterEndArray);
}

// Move cursor position back n times.
// The functions returns number moves.
int consoleEditor::MoveBack(const int& n, const bool& soundFlag)
{
    int i;
    for ( i = 0; i < n; i++ )
    {
        if ( absolute_position_ == 0 )
        {
            if ( soundFlag )
            {
                printf("\a");
            }
            return i;
        }
        // if cursor position is not at begin of row
        if ( IsCursorePosAtbeginOfRow() )
        {
            if ( text_[absolute_position_ - 1] == '\n' )
            {
                MoveCursorePosition(Find_X_CursorePosAtUpperRow(), -1);
                absolute_position_ = absolute_position_ - 2;
            }
            else
            {
                absolute_position_--;
                MoveCursorePosition(79, -1);
            }
        }
        else
        {
            printf("\b");
            absolute_position_--;
        }
    }
    return n;
}

int consoleEditor::MoveForward(const int& n, const bool& soundFlag)
{
    int i;
    for( i = 0; i < n; i++ )
    {
        if ( absolute_position_ == text_.size() )
        {
            if ( soundFlag )
            {
                printf("\a");
            }
            return i;
        }
        if ( text_[absolute_position_] == '\r' )
        {
            printf("\n");
            absolute_position_++;
        }
        else
        {
            printf("%c", text_[absolute_position_]);
        }
        absolute_position_++;
    }
    return i;
}

void consoleEditor::PrintGraficSymbol(char ch)
{
    // if console pointer is in end
    unsigned newTextSize = text_.size() + 1;
    if ( absolute_position_ == newTextSize - 1)
    {
        text_.insert(text_.end(), ch);
        printf("%c", ch);
        absolute_position_++;
    }
    else
    {
        text_.insert(text_.begin() + absolute_position_, ch);
        printf("%c", ch);
        absolute_position_++;
        PrintSymbolsAfterCursore();
    }
}
int consoleEditor::FindOutVerticalPosition()
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(hConsole, &info);

    printf("\nx = %d, y = %d\n", info.dwCursorPosition.X, info.dwCursorPosition.Y);
    return 0;
}

void consoleEditor::MoveCursorePosition(short dx, short dy)
{
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    COORD newCOORD;
    newCOORD.X = consoleInfo.dwCursorPosition.X + dx;
    newCOORD.Y = consoleInfo.dwCursorPosition.Y + dy;
    SetConsoleCursorPosition( hConsole, newCOORD );
}

short consoleEditor::Find_X_CursorePosAtUpperRow()
{
    short shift = 0;
    int i = absolute_position_ - 2;

    while ( i != 0 && text_[i - 1] != '\n' )
    {
        shift++;
        i--;
    }
    return shift;
}

bool consoleEditor::IsCursorePosAtbeginOfRow()
{
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    return !consoleInfo.dwCursorPosition.X;
}

void consoleEditor::PrintSymbolsAfterCursore()
{
    unsigned i;
    unsigned numberPrintedSymbols = text_.size() - absolute_position_;
    unsigned runningI = absolute_position_;
    for ( i = 0; i < numberPrintedSymbols; i++)
    {
        printf("%c", text_[runningI + i]);
        /*if ( text[i] == '\r' )
        {
            absolutePosition++;
        }*/
        absolute_position_++;
    }
    for ( i = 0; i < numberPrintedSymbols; i++)
    {
        MoveBack(1, false);
    }
}

void consoleEditor::PrintNewLineSymbol()
{
    text_.insert(text_.begin() + absolute_position_, '\n');
    if ( !IsCursorePosAtbeginOfRow() )
    {
        text_.insert(text_.begin() + absolute_position_, '\r');
        absolute_position_++;
    }
    ClearSymbolsAfterCursore();
    printf("\n");

    absolute_position_++;
    PrintSymbolsAfterCursore();
}

void consoleEditor::ClearSymbolsAfterCursore()
{
    int i;
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    for ( i = consoleInfo.dwCursorPosition.X; i < 50; i++ )
    {
        printf(" ");
    }
    for ( i = consoleInfo.dwCursorPosition.X; i < 50; i++ )
    {
        printf("\b");
    }
    /*int n;
    n = moveBack(INT_MAX, false);
    int posBegY, posEndY;
    posBegY = consoleInfo.dwCursorPosition.Y;

    moveForward(INT_MAX, false);
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    posEndY = consoleInfo.dwCursorPosition.Y;*/


    /*int Sum =0 ;
    for ( i = 0; i < absolutePosition; i++)
    {
        if ( text[i] == '\r' )
        {
            Sum++;
        }
    }
    Sum = Sum + absolutePosition;
    //int n = std::count(text.begin(), text.begin() + absolutePosition, '\r');
    //int numberNewLineSymbols = std::count(text.begin() + absolutePosition, text.end(), '\n');
    for ( i = 0; i < posEndY - posBegY; i++ )
    {
        // print 80 spaces
        printf("                    "
               "                    "
               "                    "
               "                    ");
    }
    printf(" N = %d", posEndY - posBegY);
    moveBack(n, false);
    //printf("\n%d %d", absolutePosition, text.size() );*/
}

void consoleEditor::DeletePreviousSymbol()
{
    int i;
    int messageLength = text_.size();
    text_.erase(text_.begin() + absolute_position_ - 1);
    absolute_position_--;
    printf("\b");
    for ( i = absolute_position_; i < messageLength; i++)
    {
        printf("%c", text_[i]);
    }
    printf("\b \b"); // delete the last symbol
    messageLength--;
    for ( i = absolute_position_; i < messageLength; i++)
    {
        printf("\b");
    }
}
