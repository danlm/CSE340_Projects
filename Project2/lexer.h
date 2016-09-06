//--------------------------------------------------------------
//  CSE 340 Project 2
//  Student Name: Daniel Martin
//  
//  Description: A header file for the lexer library.
//--------------------------------------------------------------

#ifndef __LEXER__H__
#define __LEXER__H__

// -------------------------------- token types --------------------------------
#define ID         1
#define HASH       2
#define DOUBLEHASH 3
#define ARROW      4
#define ERROR      5

// ----------- Global variables associated with the next input token -----------
#define MAX_TOKEN_LENGTH 200
extern char token[MAX_TOKEN_LENGTH];	// token string
extern int  tokenLength;				// token length
extern int  ttype;						// token type
extern int  line;						// current line number

// ------------------------------ Lexer functions ------------------------------
/*
 * Reads the next token from standard input and returns its type. The actual
 * value of the token is stored in the global variable token and the global
 * variable ttype will store the return value of the last call to getToken()
 */
int getToken();

/*
 * Sets a flag so that the next call to getToken() would return the last token
 * read from the input instead of reading a new token
 */
void ungetToken();

#endif //__LEXER__H__
