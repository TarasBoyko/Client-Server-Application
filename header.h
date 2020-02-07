#ifndef HEADER_H_INCLUDED
#define HEADER_H_INCLUDED

#define CLEAR_BUFFER \
	while ( kbhit() ) \
getch();

// value of flags is value of symbols from other languages( I support only ukrainian, russian, english languages)
const int kEmptyFileFlag = -128;
const int kNoFileFlag = -127;
const int kUsingOtherClientFlag = -125;
const int kOverloadedServerFlag = -124;

const int kMaxFileNameLen = 256;

const int kEOKB = 26;; // end of keyBoard
const int kMY_PORT = 3011;
const int kPieceFileSize = 1024;

#endif // HEADER_H_INCLUDED
