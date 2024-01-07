// Authors: Abdulwadood Ashraf Faazli, Muhammad Mubeen
// NUID: 002601201, 002604679
// Project 1 - Shell - CS3650

#ifndef _TOKENS_H
#define _TOKENS_H

// initializing the tokens array
void init_tokens();

// getting the tokens from the input string
char **create_tokens(const char *input);

// adding a token from the string to the array in our program
void add_token(const char* token);

// reading a string  argument from the shell as it is
int get_string(const char *input, char *string);

// in case more tokens are there than initialized, using dynamic memory allocation to add to the initial array
void grow_tokens();

// freeing the memory held by the tokens array
void free_tokens(char **tokens);

#endif /* _TOKENS_H */
