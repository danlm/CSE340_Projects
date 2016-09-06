//--------------------------------------------------------------
//  CSE 340 Project 1
//  Student Name: Daniel Martin
//  
//  Description: This program makes a doubly-linked list out
//  of the tokens returned by the lexer, and prints it out.
//--------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"

struct token {
	char* type;
	char content[MAX_TOKEN_LENGTH];
	int line_number;
	struct token *next;
    struct token *prev;
};

/* See whether token should be added to the list:
    1. If token is of type NUM
    2. The token is of type ID and the actual token is equal to one of the following
       values: "cse340", "programming", or "language"
    Return 1 if the token fits, returns 0 otherwise.
*/
static int should_link(void)
{
	if (ttype == NUM) {
        return 1;
    }
    else if (ttype == ID) {
        if ((strcmp(token, "cse340") == 0) || (strcmp(token, "programming") == 0) || (strcmp(token, "language") == 0)) {
            return 1;
        }
    }
    
    return 0;
}

int main (void) {
    struct token *head = NULL, *tail = NULL, *current = NULL, *addition = NULL;

    // Grab the first input token to set the head of the list
    getToken();
    while ((head == NULL) && (ttype != EOF) && (ttype != ERROR)){ //End of the input, or something wrong 
      if (should_link()) {
          current = (struct token *)malloc(sizeof(struct token));
          strcpy(current->content, token);
          current->line_number = line;
          switch (ttype){
                case NUM:
                    current->type = "NUM";
                    break;
                case ID:
                    current->type = "ID";
                    break;
          }
          head = current;
          head->next = NULL;
          head->prev = NULL;
          tail = head;
          getToken();
      }
    }
    
    // Add to the end of the line
    while ((ttype != EOF) && (ttype != ERROR)) {
        if (should_link()) {
          addition = (struct token *)malloc(sizeof(struct token));
          switch (ttype){
                case NUM:
                    addition->type = "NUM";
                    break;
                case ID:
                    addition->type = "ID";
                    break;
          }
          strcpy(addition->content, token);
          addition->line_number = line;
          addition->prev = current;
          addition->next = NULL;
          current->next = addition;
          tail = addition;
          current = tail;
      }
      getToken();
    } 
    
    // Print, then delete the tokens
    current = tail;
    while (current) {
        printf("%d %s %s\n", current->line_number, current->type, current->content);
        if (current->prev != NULL){
            current = current->prev;
            free(current->next);
        }
        else {
            free (current);
            break;
        }
    }

    return 0;
}
