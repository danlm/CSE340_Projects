//--------------------------------------------------------------
//  CSE 340 Project 4
//  Student Name: Daniel Martin
//  
//  Description: This is a header for the code graph functions.
//-------------------------------------------------------------- 
#ifndef _CODE_GRAPH_H_
#define _CODE_GRAPH_H_

#include "compiler.h"
#include <string.h>
#include <stdlib.h>

// This implementation does not allow more than 20 symbols
#define MAX_NUMBER_OF_VARS 20
#define ALLOC(t) (t*) calloc(1, sizeof(t))

// The following global variables are defined in code_graph.c:
extern struct ValueNode* table[MAX_NUMBER_OF_VARS];
extern int number_of_vars;
extern int number_of_ints;

#endif /* _CODE_GRAPH_H_ */
