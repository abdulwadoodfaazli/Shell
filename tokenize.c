// Authors: Abdulwadood Ashraf Faazli, Muhammad Mubeen
// NUID: 002601201, 002604679
// Project 1 - Shell - CS3650

// ************** Including relevant libraries **************

#include <stdio.h>
#include <assert.h>

// ************** Including the necessary header file **************

#include "tokens.h"

// ************** Defining the main function **************

// Demo for printing shell arguments parsed in tokens.c
int main(int argc, char **argv) {
  // initializing an array for holding the shell arguments as input
  char input[256];

  // getting input from the stdin stream and populating the input array with it
  fgets(input, 256, stdin);

  // creating the tokens array (from tokens.c)
  char **tokens = create_tokens(input);
  
  // making sure there was no error in creating the tokens array
  assert(tokens != NULL);

  // creating a copy of the tokens array to test tokenizer out
  char **tokens_copy = tokens;

  // printing each shell argument
  while (*tokens_copy != NULL) {
    printf("%s\n", *tokens_copy);
    ++tokens_copy;
  }

  free_tokens(tokens); // freeing all memory held upon ending program
  return 0;
}
