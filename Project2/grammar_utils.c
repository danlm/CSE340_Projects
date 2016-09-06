//-----------------------------------------------------------------
//  CSE 340 Project 2
//  Student Name: Daniel Martin
//  
//  Description: This program reads in a grammar in modified
//  Backus-Naur Form (BNF), and does one of the following options:
//  (1) See whether each rule can generate a one-token string
//  (2) Calculates the FIRST set 
//  (3) Calculates the FOLLOW set
//-----------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>   // includes definitions for boolean type and constants
#include <string.h>
#include "lexer.h"     // functions to read in grammar

#define MAX_SYMBOLS  100
#define MAX_RHS_SIZE 100
#define MAX_RULES    100

// in C++ you should use string vectors to store the symbols.
// Using MAX_SYMBOLS is convenient but can lead to programming
// errors and is not efficient memory-wise. In the code below,
// I assume that the symbols array has all non-terminals followed
// by epsilon followed by a representation of EOF,
// and finally followed by all terminals sorted according to
// dictionary order.

// this can be represented as follows
//
//    ________________________________________________________
//   | NT0 | NT1 | ... | NTk-1 | # | $ | T0 | T1 | ... | Tm-1 |
//    --------------------------------------------------------
//
// In this array there are k non-terminals and m terminals.
// The values of k and m are stored in the variables num_non_terminals
// and num_terminals respectively (see declarations below). 
//
// epsilon is at position k, eof is at position k+1
// and the terminals start at position k+2 = 2 + num_non_terminals
//
// Note that with this organization there is an simple correspondence between
// indices in a FIRST and FOLLOW sets and indices in the symbol array.
//
// FIRST and FOLLOW sets can be represented as boolean arrays with indices
// ranging from 0 to m+1, where m is the num_terminals.
//
// If position i in a set is true, this means the corresponding element in 
// the symbol array is in the set.
//
// The correspondence between the boolean array index and the symbols 
// array index is given by the following relation:
//
//    symbol_array_index = set+index + num_non_terminals
//
// This way, after calculating the sets, it is easy to print the 
// corresponding symbols from the symbols set

struct rule
{
    int rhs_length;
    int LHS;               // is the index of the non-terminal LHS in 
                           // the symbols array
    int RHS[MAX_RHS_SIZE]; // RHS is an array of indices of RHS symbols
};

int num_symbols;
int num_non_terminals;
int num_terminals;
int num_rules;
char *symbols[MAX_SYMBOLS];
struct rule rule[MAX_RULES];
bool gen_epsilon[MAX_SYMBOLS];
bool gen_string[MAX_SYMBOLS];
bool** FIRST;
bool** FOLLOW;

/*
 * This function should print out the grammar and symbol table.
 */
void print_grammar(void)
{
    int i, j, r;
    printf("Non-terminals: ");
    for (i = 0; i < num_non_terminals; ++i) {
        printf("%s ", symbols[i]);
    }
    printf("\n");
    printf("Terminals: ");
    for (i = 2 + num_non_terminals; i < num_symbols; i++) {
        printf("%s ", symbols[i]);
    }
    printf("\n");
    for (r = 0; r < num_rules; r++)
    {
        printf("%s -> ", symbols[rule[r].LHS]);
        for (j = 0; j < rule[r].rhs_length; j++)
        {
            printf("%s ", symbols[rule[r].RHS[j]]);
	    }
        printf("\n");
    }
}

void sort_array(char *name[], int n)
{
    int i, j; 
    char temp[MAX_TOKEN_LENGTH];
    for (i = 0; i < n - 1 ; i++)
    {
        for (j = i + 1; j < n; j++)
        {
            if (strcmp(name[i], name[j]) > 0)
            {
                strcpy(temp, name[i]);
                strcpy(name[i], name[j]);
                strcpy(name[j], temp);
            }
        }
    }
}

/*
 * This function should check if a given symbol is in the symbol table.
 */
int find_in_symbol_table(char* symbol)
{
    int i;
	for (i = 0; i < num_symbols; ++i) {
        if (strcmp(symbol, symbols[i]) == 0) {
	
            return i;
        }
    }    
    return -1;
}

void epsilon_generation_test(void)
{
    int i,j;
    
    // epsilon generates epsilon, so we set epsilon's entry
    // to true. Remember that epsilon's entry has index
    // equal to num_non_terminals (see above)
    gen_epsilon[num_non_terminals] = true;
   
    // initially all symbols other than epsilon do not generate epsilon
    for (i = 0; i < num_symbols; i++)
        if (i != num_non_terminals)
             gen_epsilon[i] = false;

    // Find out which non-terminals can generate epsilon
    bool changed = true;
    while (changed)
    {
       changed = false;  // if we change something, we will set 
                         // changed back to true

       for (i = 0; i < num_rules; i++)
       {
           if ( gen_epsilon[rule[i].LHS] )
                continue;
	   else if (( rule[i].rhs_length == 1 ) && (rule[i].RHS[0] == num_non_terminals)) // A -> epsilon
	   {
               gen_epsilon[rule[i].LHS] = true;
               changed = true;
           }
	   else   // A -> A1 A2 ... An
           {
              bool some_does_not_gen_epsilon = false;
              for (j = 0; j < rule[i].rhs_length; j++)
                   some_does_not_gen_epsilon |= !gen_epsilon[rule[i].RHS[j]];

              // if all symbols on RHS generate epsilon
              // LHS also generates epsilon
              if (!some_does_not_gen_epsilon)
              {
                  gen_epsilon[rule[i].LHS] = true;
                  changed = true;
              }
           }
        }
     }
}

void string_generation_test(void)
{
    int i;
    
    // initially all symbols do not generate a string
    for (i = 0; i < num_symbols; i++)
             gen_string[i] = false;

    // Find out which non-terminals can generate a string
    bool changed = true;
    while (changed)
    {
       changed = false;  // if we change something, we will set 
                         // changed back to true
       for (i = 0; i < num_rules; i++) {
            if ( gen_string[rule[i].LHS] )
                continue;
	        else if ( rule[i].rhs_length == 1 ) {
                // Check if A -> a, where a is a terminal symbol
                if (rule[i].RHS[0] >= (2+num_non_terminals)) {
	                gen_string[rule[i].LHS] = true;
                    changed = true;
                }
                // Check if A -> B, where B is a non-terminal that can generate a string of length 1
                else if ((rule[i].RHS[0] < num_non_terminals) && (gen_string[rule[i].RHS[0]])) {
                    gen_string[rule[i].LHS] = true;
                    changed = true;
                }
	        }
            else {     // A -> A1 A2 ... An
                int j = 0;
                int k = 0;
                bool isFound = false;
                while((j < rule[i].rhs_length) && (!isFound)){
                    // Check if A -> a, where a is a terminal symbol or a non-terminal that can generate a string of length 1
                    if ((rule[i].RHS[j] >= (num_non_terminals+2)) || ((rule[i].RHS[j] < num_non_terminals) && (gen_string[rule[i].RHS[j]]))) {
                        bool some_does_not_gen_epsilon = false;
                        k = 0;
                        while (k < rule[i].rhs_length) {
                            if (k != j) {
                                some_does_not_gen_epsilon |= !gen_epsilon[rule[i].RHS[k]];
                            }
                            ++k;
                        }
                        if (!some_does_not_gen_epsilon)
                          {
                              gen_string[rule[i].LHS] = true;
                              isFound = true;
                          }
                    }
                    ++j;
                }
                if (isFound) {
                    changed = isFound;
                }
            }
        }
    }
}

/*
 * This function should calculate the union of set1 and set2
 * and store the results in set1. It should return true if as a result of 
 * this operation set1 is changed.
 *
 * set1 <- set1 U set2
 *
 */
bool completeUnion(bool set1[], bool set2[])
{
    bool changed = false, current = false;
	int i;
	// Assumed to be follow set
	for (i = 0; i < (1+num_terminals); ++i) {
		current = set1[i];
		set1[i] |= set2[i];
		if (!changed)
        {
		    changed = (current == set1[i]) ? false : true;
        }
	}
	return changed;  // TODO: Replace this with a correct implementation
}

/*
 * This function should calculate the union of set1 and set2 - { epsilon }
 * and store the results in set1. It should return true if as a result of 
 * this operation set1 is changed.
 *
 * set1 <- set1 U (set2 - {epsilon})
 *
 */
bool unionMinusEps(bool set1[], bool set2[])
{
    bool changed = false, current = false;
	int i;
    /*printf("Set1\tSet2\n");
    for (i = 0; i < (2+num_terminals); ++i) {
        printf("%d\t%d\n", set1[i], set2[i]);
    }*/
	// Start at index 1 to skip epsilon
	for (i = 1; i < (2+num_terminals); ++i) {
		current = set1[i];
		set1[i] |= set2[i];
        if (!changed)
        {
		    changed = (current == set1[i]) ? false : true;
        }
	}
    /*printf("%s\n", changed ? "true" : "false");
    for (i = 0; i < (2+num_terminals); ++i) {
        printf("%d\t%d\n", set1[i], set2[i]);
    }*/
	return changed;  // TODO: Replace this with a correct implementation
}

/*
 * Same as above, except for FOLLOW.
 *
 */
bool unionMinusEpsFollow(bool set1[], bool set2[])
{
    bool changed = false, current = false;
	int i;
	// Start at index 1 to skip epsilon
	for (i = 1; i < (2+num_terminals); ++i) {
        int k = i-1;
		current = set1[k];
		set1[k] |= set2[i];
		if (!changed)
        {
		    changed = (current == set1[k]) ? false : true;
        }
	}
	return changed;  // TODO: Replace this with a correct implementation
}

/*
 * This function should check if epsilon is a member of the given set 
 * or not. It should return true if epsilon is NOT a member of set.
 */
bool not_epsilon_in(bool set[])
{
	return (set[0]) ? false : true; // TODO: Replace this with a correct implementation
}

/*
 * This function should add epsilon to the given set. It should return true
 * if as a result of this operation set is changed.
 *
 * set <- set U {epsilon}
 *
 */
bool add_epsilon_to_set(bool set[])
{
	if (not_epsilon_in(set)){
		set[0] = true;
		return true;
	}
	return false;  // TODO: Replace this with a correct implementation
}

/*
 * This function should be called after reading the input and
 * before calling first(). It allocates memory for the FIRST sets
 */
void allocate_first_sets()
{
    int i;
    if (FIRST == NULL)
    {
        FIRST = (bool**) malloc(sizeof(bool*) * num_symbols);
        for (i = 0; i < num_symbols; i++)
            FIRST[i] = (bool*) malloc(sizeof(bool) * (2 + num_terminals));
    }
}

/*
 * This function should be called after reading the input and
 * before calling follow(). It allocates memory for the FOLLOW sets
 */
void allocate_follow_sets()
{
    int i;
    if (FOLLOW == NULL)
    {
        FOLLOW = (bool**) malloc(sizeof(bool*) * num_non_terminals);
        for (i = 0; i < num_non_terminals; i++)
            FOLLOW[i] = (bool*) malloc(sizeof(bool) * (1 + num_terminals)); //epsilon cannot be in a follow set
    }
}

void first(void)  /* Modified to match the above requirements on symbols */
{
	int i,j,r;

    // Initially all FIRST sets are empty.
    for (i = 0; i < num_symbols; i++)
        for (j = 0; j < 2 + num_terminals; j++)
            FIRST[i][j] = false;

    // FIRST(epsilon) = { epsilon }
    FIRST[num_non_terminals][0] = true;

    // The FIRST set of a terminal contains only the terminal itself
    for (i = 2 + num_non_terminals; i < num_symbols; i++)
        FIRST[i][i - num_non_terminals] = true;

    // Changed tells us if there were any changes in the FIRST sets
    // during an iteration. Initially we set it to true so that we
    // can enter the main loop
    bool changed = true;
    while (changed)
    {
       // we set changed to false so that we exit the loop after an iteration
       // unless something changed in which case changed will be set to true
       // inside the loop
       changed = false;
       for (r = 0; r < num_rules; r++)
       {
          for (j = 0; j < rule[r].rhs_length; j++)
          {
               changed |=  unionMinusEps(FIRST[rule[r].LHS],
                                         FIRST[rule[r].RHS[j]]); 

               if ( not_epsilon_in(FIRST[rule[r].RHS[j]]) )
			        break;
	      }
          bool add_epsilon = true;
          for (j = 0; j < rule[r].rhs_length; j++)
          {
               if ( not_epsilon_in(FIRST[rule[r].RHS[j]]) )
                    add_epsilon = false;
	  }
	  if (add_epsilon)
               changed = changed | add_epsilon_to_set(FIRST[rule[r].LHS]);
       }
   }
}

void follow(void)  /* Modified to match the above requirements on symbols */
{
	int i,j,r;

    // Initially all FOLLOW sets are empty.
    for (i = 0; i < num_non_terminals; i++)
    {
        for (j = 0; j < 1 + num_terminals; j++)
        {
            FOLLOW[i][j] = false;
        }
    }

    // Rule I: FOLLOW(S) = { eof }
    FOLLOW[0][0] = true;

    // Changed tells us if there were any changes in the FOLLOW sets
    // during an iteration. Initially we set it to true so that we
    // can enter the main loop
    bool changed = true;
    while (changed)
    {
       // we set changed to false so that we exit the loop after an iteration
       // unless something changed in which case changed will be set to true
       // inside the loop
       changed = false;
       for (r = 0; r < num_rules; r++)
       {
          // Rule II
          if (rule[r].RHS[rule[r].rhs_length-1] < num_non_terminals) {
            changed |= completeUnion(FOLLOW[rule[r].RHS[rule[r].rhs_length-1]], FOLLOW[rule[r].LHS]);
          }
          
          // Rule IV
          for (j = 1; j < rule[r].rhs_length; j++)
          {
              if (rule[r].RHS[j-1] < num_non_terminals) { 
                changed |= unionMinusEpsFollow(FOLLOW[rule[r].RHS[j-1]], FIRST[rule[r].RHS[j]]); 
              }
	      }
          // Rules III and V
          for (i = 1; i < rule[r].rhs_length-1; i++)
          {
              if (rule[r].RHS[i] < num_non_terminals) {
                  bool has_epsilon = true;
                  for (j = i+1; j < rule[r].rhs_length; j++)
                  {
                       if ( not_epsilon_in(FIRST[rule[r].RHS[j]]) )
                            has_epsilon = false;
                            break;
	              }
                  if (has_epsilon) {
                      changed |= completeUnion(FOLLOW[rule[r].RHS[i]], FOLLOW[rule[r].LHS]);
                      j = i+2;
                      while (j < rule[r].rhs_length)
                      {
                           changed |= unionMinusEpsFollow(FOLLOW[rule[r].RHS[i]],FIRST[rule[r].RHS[j]]);
                           ++j; 
	                  }
                  }
              }
          }
       }
   }
}

int main (int argc, char* argv[])
{
    FIRST = NULL;
    FOLLOW = NULL;
    
    int task = 0;
	num_symbols = 0;
    num_non_terminals = 0;
    num_terminals = 0; 
    num_rules = 0;
	int symbol_index = -1;

    if (argc < 2) {
        printf("Error: missing argument\n");
        return 1;
    }
    /* Note that argv[0] is the name of your executable
    * e.g. a.out, and the first argument to your program
    * is stored in argv[1]
    */
    task = atoi(argv[1]);
	
    getToken();
    // Read in the set of non-terminals
    if ((ttype != DOUBLEHASH) && (ttype != ERROR) && (ttype != EOF)) {
        while ((ttype != HASH) && (ttype != ERROR) && (ttype != EOF)) {
            symbols[num_non_terminals] = malloc(strlen(token)+1);
            strcpy(symbols[num_non_terminals], token);
            ++num_non_terminals;
            getToken();
        }
    }

    // Add EPSILON and EOF to symbol table
    symbols[num_non_terminals] = "#";
    symbols[num_non_terminals+1] = "$";
    num_symbols = num_non_terminals + 2;
    
    // Read in grammar rules
    getToken();
    while ((ttype != DOUBLEHASH) && (ttype != ERROR) && (ttype != EOF)) {
        // Read in LHS of rule
		symbol_index = find_in_symbol_table(token);
        rule[num_rules].LHS = symbol_index;
        // Get ARROW
        getToken();
        getToken();
        if (ttype == HASH){
            rule[num_rules].RHS[rule[num_rules].rhs_length] = num_non_terminals;
            ++(rule[num_rules].rhs_length);
        }
		else if ((ttype == ID)){
            while ((ttype != HASH) && (ttype != ERROR) && (ttype != EOF)){
                symbol_index = find_in_symbol_table(token);
                if (symbol_index >= 0) {
                    rule[num_rules].RHS[rule[num_rules].rhs_length] = symbol_index;
                }
                else {
                    symbols[num_symbols] = malloc(strlen(token)+1);
				    strcpy(symbols[num_symbols], token);
				    rule[num_rules].RHS[rule[num_rules].rhs_length] = num_symbols;
				    ++num_terminals;
					++num_symbols;
				}
                ++(rule[num_rules].rhs_length);
                getToken();
            }
        }
        ++num_rules;
        getToken();  
    }

	//print_grammar();

    if (ttype == EOF) {
        printf("Error: incomplete grammar at line %d\n", line);
    }
    else if (ttype == ERROR) {
        printf("Error: specification error at line %d\n", line);
    }
    else {
		int i, j, numToPrint;
        bool hasPrinted;
        char *printing[num_terminals+1]; // Store strings for printing FIRST and FOLLOW
        switch (task) {
            case 1:
                // Call the function(s) responsible for task 1 here

                epsilon_generation_test();
                string_generation_test();
				for (i = 0; i < num_non_terminals; i++) {
					printf("%s: ", symbols[i]);
					if (gen_string[i]) {
						printf("YES\n");
					}
					else {
						printf("NO\n");
					}
				}
                break;
            case 2:
                // Call the function(s) responsible for task 2 here
                allocate_first_sets();
                first();
                hasPrinted = false;

                for (i = 0; i < num_non_terminals; i++) {
                    numToPrint = 0;
					printf("FIRST(%s) = { ", symbols[i]);
					for (j = 0; j < 2 + num_terminals; j++) {
						if (FIRST [i][j]) {
                            printing[numToPrint] = malloc(strlen(symbols[j+num_non_terminals])+1);
                            strcpy(printing[numToPrint], symbols[j+num_non_terminals]);
                            ++numToPrint;
						}
					}
                    sort_array(printing, numToPrint);

                    for (j = 0; j < numToPrint; j++) {
                            if (hasPrinted) {
                                printf(", ");
                            }
							printf("%s", printing[j]);
                            hasPrinted = true;
					}
                    hasPrinted = false;
                    
					printf(" }\n");
				}
                
                break;
            case 3:
                // Call the function(s) responsible for task 3 here
                allocate_first_sets();
                first();
                allocate_follow_sets();
                follow();
                hasPrinted = false;
				for (i = 0; i < num_non_terminals; i++) {
					numToPrint = 0;
					printf("FOLLOW(%s) = { ", symbols[i]);
					for (j = 0; j < 1 + num_terminals; j++) {
						if (FOLLOW [i][j]) {
                            printing[numToPrint] = malloc(strlen(symbols[j+num_non_terminals+1])+1);
                            strcpy(printing[numToPrint], symbols[j+num_non_terminals+1]);
                            ++numToPrint;
						}
					}
                    sort_array(printing, numToPrint);

                    for (j = 0; j < numToPrint; j++) {
                            if (hasPrinted) {
                                printf(", ");
                            }
							printf("%s", printing[j]);
                            hasPrinted = true;
					}
                    hasPrinted = false;
					printf(" }\n");
				}
                break;
            default:
                printf("Error: unrecognized task number %d\n", task);

        }
    }

    return 0;
}
