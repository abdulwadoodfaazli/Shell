// Authors: Abdulwadood Ashraf Faazli, Muhammad Mubeen
// NUID: 002601201, 002604679
// Project 1 - Shell - CS3650

// ************** Including relevant libraries **************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// ************** Define macros **************

// for growing the size of the tokens array by 256 bytes every time we need to
#define GROW_SIZE 256

// ************** Define global variables **************

static int current_tokens_size; // current size for the tokens array (will be incremented as we add new tokens to it)
static int max_tokens_capacity; // total capacity for tokens array
static char **tokens = NULL;    // initial array for holding tokens

// ************** Declaring the necessary functions **************

static void init_tokens();
char **create_tokens(const char *input);
static int get_string(const char *input, char *string);
static void add_token(const char *token);
void grow_tokens();
void free_tokens(char **tokens);

// ************** Defining the declared functions **************

// initializing the tokens array

void init_tokens()
{
  // since we are just initializing our tokens array here, we set the current size as 0
  current_tokens_size = 0;
  max_tokens_capacity = GROW_SIZE; // since we are initializing the tokens array, we give it the minimal size i.e GROW_SIZE
  tokens = malloc(sizeof(char *) * max_tokens_capacity);
  // making sure the tokens array is not empty after growing it (which was happening in some cases)
  assert(tokens != NULL);
  tokens[0] = NULL;
}

// getting the tokens from the input string

char **create_tokens(const char *input)
{
  char string[256];             // our string of tokens
  unsigned int args_iter = 0;   // for iterating over all of the shell arguments
  unsigned int string_iter = 0; // for iterating over our string[] array of tokens

  // initializing the tokens array before starting to populate it with the tokens
  init_tokens();

  // as long as there is some input coming from the shell
  while (input[args_iter] != 0)
  {
    switch (input[args_iter])
    {
    // for tokens
    case '(':
    case ')':
    case '>':
    case '<':
    case '|':
    case '&':
    case ';':
      // if we are already on a past token, we end it by \0 and prepare for taking the next argument
      if (string_iter > 0)
      {
        string[string_iter] = '\0';
        add_token(string);
        string_iter = 0;
      }
      // getting the next token from shell as it is, and following it by a \0 to mark it as a string
      string[0] = input[args_iter];
      string[1] = '\0';
      add_token(string);
      break;
    // for special characters
    case ' ':
    case '\t':
    case '\n':
      // if we are already on a past token, we end it by \0 and prepare for taking the next argument
      if (string_iter > 0)
      {
        string[string_iter] = '\0';
        add_token(string);
        string_iter = 0;
      }
      break;
    // for quotation mark (to be skipped)
    case '"':
      ++args_iter;
      // in case of a quotation, since we need to grab the entire proceeding string as it is, we do that
      unsigned int bytes = get_string(&input[args_iter], &string[string_iter]);
      // making our iterators skip over the following string sequence as we have a separate function for dealing with that string
      args_iter += bytes;
      string_iter += bytes;
      break;
    default:
      // in a neutral situation, we will just add a shell argument to our string of tokens and create room for the next shell argument
      string[string_iter] = input[args_iter];
      ++string_iter;
    }
    ++args_iter;
  }

  // it is possible that we didn't place our last token in our string of tokens, so we will just grab that as well in such a case
  if (string_iter > 0)
  {
    string[string_iter] = 0;
    add_token(string);
  }

  return tokens;
}

// reading a string argument from the shell as it is

int get_string(const char *input, char *string)
{
  unsigned int bytes = 0; // the space our token string will occupy
  // as long as there is some valid input and not a quotation mark (as specified in instructions)
  while (*input && *input != '"')
  {
    *string = *input; // temporarily storing that string token
    ++bytes;          // adding to the space occupied by the string
    ++string;         // creating room for the next token
    ++input;          // moving on to the next token
  }
  // since we are creating a string in C, we will end it by the \0 character
  *string = '\0';

  return bytes;
}

// adding a token from the string to the array in our program

void add_token(const char *token)
{
  assert(token != NULL); // making sure the token isn't invalid (empty)

  // if the capacity for the tokens array has been reached, growing that array to fit in the new tokens
  if ((current_tokens_size + 1) == max_tokens_capacity)
  {
    grow_tokens();
  }

  // throwing the new token into our (possibly expanded) tokens array
  char *new_token = strdup(token);
  // since this is the latest token we have added to our tokens array so far, it should be the last one in there
  tokens[current_tokens_size] = new_token;
  // now that we added a new token, we increment the size of our tokens array by 1
  ++current_tokens_size;
  // since we are one step ahead in our tokens array, we temporarily keep that last element as NULL and populate it later
  tokens[current_tokens_size] = NULL;
}

// in case more tokens are there than initialized, using dynamic memory allocation to add to the initial array

void grow_tokens()
{
  max_tokens_capacity += GROW_SIZE; // GROW_SIZE is our macro which
  tokens = realloc(tokens, sizeof(char *) * max_tokens_capacity);
  // making sure the tokens array is not empty after growing it (which was happening in some cases, somehow)
  assert(tokens != NULL);
}

// freeing the memory held by the tokens array

void free_tokens(char **tokens)
{
  char **tokens_copy = tokens; // a copy of the tokens array to free everything inside it sequentially

  // since our tokens are strings and strings are arrays of chars, we have to free them first
  while (*tokens_copy != NULL)
  {
    free(*tokens_copy);
    ++tokens_copy;
  }

  free(tokens); // finally freeing the tokens array itself
}
