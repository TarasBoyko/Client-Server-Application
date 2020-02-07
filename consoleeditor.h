#ifndef CONSOLEEDITOR_H
#define CONSOLEEDITOR_H

#include <vector>

using std::vector;


class consoleEditor
{
    friend class Client;
    public:
        consoleEditor();
        void assign(char* begArray, char* afterEndArray);
        int MoveBack(const int& n, const bool& soundFlag);
        int MoveForward(const int& n, const bool& soundFlag);
        void PrintNewLineSymbol();
        void PrintGraficSymbol(char ch);
        int FindOutVerticalPosition();
        void DeletePreviousSymbol();
    private:
        // cursore position
        short x;
        short y;
        unsigned absolute_position_; // number symbols from begin piece of file to input position on console
        int verticalPosition; // number symbols from begin of row to input position on console
        vector<char> text_; // use for edit piece of file. It can change size of this piece unlimitly.

        void MoveCursorePosition(short dx, short dy);
        short Find_X_CursorePosAtUpperRow();
        bool IsCursorePosAtbeginOfRow();
        void PrintSymbolsAfterCursore();
        void ClearSymbolsAfterCursore();




};

#endif // CONSOLEEDITOR_H
