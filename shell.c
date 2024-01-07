// Authors: Abdulwadood Ashraf Faazli, Muhammad Mubeen
// NUID: 002601201, 002604679
// Project 1 - Shell - CS3650

// ************** Including relevant libraries **************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

// ************** Including the necessary header file **************

#include "tokens.h" // for importing the token-parsing funtionalities

// ************** Defining the macro **************

#define LINE_LENGTH 256 // maximum length of a line, in bytes (as instructed in project description)

// ************** Defining the global variable **************

char cachedPrevCmd[256]; // for 'caching' the previous command

// ************** Defining the necessary functions **************

// to exit the shell when "exit" is entered on the shell
int isExit(const char *cmd)
{
  int check = strcmp("exit", cmd);

  if (check == 0)
  {
    printf("Bye bye.\n");
  }

  return check;
}

// changes the current working directory to the specified path
int isCd(const char *path)
{
  if (path[0] == '/')
    return chdir(path + 1);
  else
  {
    return chdir(path);
  }
}

// to print the previous command when "prev" is entered on the shell
int isPrev(char *cmd)
{
  int check = strcmp("prev", cmd);

  if (check == 0)
  {
    printf("%s\n", cachedPrevCmd);
  }

  return check;
}

// To execute the previous command, if available
void execPrev(char *prevCmd)
{
  if (prevCmd == NULL)
  {
    printf("No command has been entered previously!\n");
  }
  else
  {
    char **prevCmdTokens = create_tokens(prevCmd); // creating tokens from the previous command
    execCmd(prevCmdTokens);                        // executing the previous command
    free_tokens(prevCmdTokens);                    // freeing the memory occupied by the previous command
  }
}

// to print the help menu when "help" is entered on the shell
int isHelp(char *cmd)
{
  int check = strcmp(cmd, "help");

  if (check == 0)
  {
    printf("Displaying help menu:\n Available built-in commands:\n cd [dir-path, ..] : This command should change the current working directory  the shell to the path specified as the argument.\n source [file-path] : Execute a script.\n Takes a filename as an argument and processes each line  the file as a command, including built-ins. In other word each line should be processed as if it was entered by t user at the prompt.\n prev : Prints the previous command line and executes it again without becoming the new command line.\n help : Explains all the built-in commands available in the shell\n exit : Exit the shell.\n");
  }

  return check;
}

// To handle cases with redirection
int isRedirect(const char *const *tokens)
{
  int index = 0;

  // iterating over the token to check if there is any redirection
  while (tokens[index] != NULL)
  {
    if (strcmp(tokens[index], ">") == 0)
    {
      return 1; // for output redirection
    }
    else if (strcmp(tokens[index], "<") == 0)
    {
      return 0; // for input redirection
    }
    index++;
  }
  return -1; // for no redirection
}

// to execute the command which includes redirection
int execRedirect(const char *const *tokens, int type)
{
  int index = 0;
  char *redirectionTokens[20]; // basically holding tokens for redirection, excluding the redirection command itself
  char *file;                  // output file

  // populating redirectionTokens with the relevant tokens
  while ((strcmp(tokens[index], ">") != 0) && (strcmp(tokens[index], "<") != 0))
  {
    redirectionTokens[index] = tokens[index];
    index++;
  }

  redirectionTokens[index] = NULL;

  if (tokens[index + 1] != NULL)
  {
    file = tokens[index + 1]; // extracting the name of the given file for redirection to read from/write to
  }
  else
  {
    return -1; // exit function as no file was given for redirection
  }

  pid_t pid;
  pid = fork();
  int state_check;

  if (pid == 0)
  {
    if (close(type) == -1)
    {
      perror("Error closing std");
      exit(1);
    }

    int fwd; // file descriptor for holding the given file

    // for output redirection
    if (type == 1)
    {
      fwd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
    // for input redirection
    else if (type == 0)
    {
      fwd = open(file, O_RDONLY);
    }

    assert(fwd == type);

    if (execvp(redirectionTokens[0], redirectionTokens) != NULL)
    {
      exit(1);
    }
  }
  else
  {
    wait(&state_check);

    if (!(WIFEXITED(state_check) && WEXITSTATUS(state_check) == 0))
    {
      printf("Error performing redirection.\n");
      return -1;
    }
  }

  return 0;
}

// determines whether command is a pipe
int isPipe(const char *const *tokens)
{
  int index = 0;
  while (tokens[index] != NULL)
  {
    if (strcmp(tokens[index], "|") == 0)
    {
      return 0;
    }
    index++;
  }
  return -1;
}

// to get the number of commands to pipe in the case of piping
int numOfPipeCmds(const char *const *tokens)
{
  int index = 0; // to iterate over tokens
  int num = 1;   // starting from 1 as we know that if there is piping, there is at least 1 command to pipe
  // iterating over the tokens
  while (tokens[index] != NULL)
  {
    if (strcmp(tokens[index], "|") == 0)
    {
      num++;
    }
    index++;
  }
  return num;
}

/*
 * Executes a single command by reading from the given input file descriptor and writing to the given output file descriptor.
 * The double char pointer command is passed to exec to execute the command.
 *
 * If the function is executed in the child process, it sets up the ends of the pipe and sets them to either stdin for inpFwd or stdout for outFwd, depending on their given value.
 * It then executes the command using execvp. If the command execution fails, the child process exits with a status of 1.
 *
 * If the function is executed in the parent process, it waits for the child process to finish. If the child process exited with a non-zero status,
 * it prints an error message to the user regarding the given command and returns -1. Otherwise, it returns 0.
 */
int pipeHelper(int inpFwd, int outFwd, const char *const *cmd)
{

  // We Fork TO COPY THE CONTENTS HER
  pid_t pid;
  pid = fork();

  int state_check;
  int check = isRedirect(cmd);

  // If the current process is a child process (i.e., pid == 0), handle any input or output redirection if necessary.
  //  If there is input redirection, set standard input to the input file descriptor (inpFwd) and close the original input file descriptor.
  //  If there is output redirection, set standard output to the output file descriptor (outFwd) and close the original output file descriptor.
  //  If there is no redirection, execute the command using execvp. If execvp fails, exit the child process with an error code.
  //  Returns: void
  if (pid == 0)
  {
    if (inpFwd != 0)
    {
      close(0);
      assert(dup(inpFwd) == 0);
      close(inpFwd);
    }

    if (outFwd != 1)
    {
      close(1);
      assert(dup(outFwd) == 1);
      close(outFwd);
    }

    // for input redirection
    if (check == 0)
    {
      if (execRedirect(cmd, inpFwd) != 0)
      {
        exit(1);
      }
      else
      {
        exit(0);
      }
    }

    // for output redirection
    else if (check == 1)
    {
      if (execRedirect(cmd, outFwd) != 0)
      {
        exit(1);
      }
      else
      {
        exit(0);
      }
    }

    // for no redirection
    else if (check == -1 && execvp(cmd[0], cmd) == -1)
    {
      exit(1);
    }
  }

  //  If the current process is the parent process, wait for the child process to finish and check its exit status.
  //  If the child process exited successfully, return 0.
  //  If the child process did not exit successfully, print an error message indicating that the command was not found and return -1.
  //  Returns: 0 if the child process exited successfully, -1 if the child process did not exit successfully.
  else
  {
    wait(&state_check);
    if (!(WIFEXITED(state_check) && WEXITSTATUS(state_check) == 0))
    {
      printf("%s: command not found\n", cmd[0]);
      return -1;
    }
    return 0;
  }
}

/*
Function will execute the given tokens which contain a pipe symbol
Get the number of commands seperated by the pipes, for each of those minus the last one,
parse the current command, create a new pipe, call executePipeHelper with the previous pipe's input file descriptor
And, the current pipe's output file descriptor and the currentcommand.
close the current pipe's ouput file descriptor, and set inpFwd to the current pipe's input fd, AND KEEP LOOPING.
If the last cmd contains a redirection symbol, call executeRedirection with either input or output fd of the current pipe, depending on the rediretion symbol. If it doesn't, then execute it with executeCommand.
Returns 0 if success, -1 if fails
*/
int execPipe(const char const *const *tokens)
{
  int inpFwd = 0;
  int pipe_Fwd[2];
  int index;
  int tokens_iter = 0; // for iterating over tokens
  int state_check;
  int redirection;

  int num = numOfPipeCmds(tokens);
  char *currCmd[20];

  pid_t pid;
  pid = fork();

  /*
   * This block of code executes multiple piped commands by forking a child process for each command and setting up
   * the pipes between them. The tokens are parsed and stored in currCmd, and the PipeHelper function is called to
   * execute each command with the appropriate input and output file descriptors.
   * For each command, a new pipe is created using pipe and the appropriate input file descriptor is passed to the
   * PipeHelper function. If PipeHelper returns -1, the child process exits with a status of 1.
   * Once all commands have been executed, the child process checks for any output redirection and executes the final
   * command using either the execCmd or Redirection function depending on the presence of output redirection.
   * Finally, the child process closes the pipes and exits with a status of 0.
   */
  if (pid == 0)
  {
    for (index = 0; index < num - 1; ++index)
    {
      // iterate over each token and store/execute it sequentially
      int i = 0;
      while (tokens[tokens_iter] != NULL && strcmp(tokens[tokens_iter], "|") != 0)
      { // "ls", "-F" |
        currCmd[i] = tokens[tokens_iter];
        i++;
        tokens_iter++;
      }
      tokens_iter++;     // inc the count1 to skip over the | in the next iter
      currCmd[i] = NULL; // set the last elt to NULL; this is for execv's sanity
      pipe(pipe_Fwd);

      if (pipeHelper(inpFwd, pipe_Fwd[1], currCmd) == -1)
      { // ls | nl -> currentCmd {ls ... NULL}
        close(pipe_Fwd[1]);
        exit(1);
      }

      close(pipe_Fwd[1]);
      inpFwd = pipe_Fwd[0];
    }

    if (inpFwd != 0)
    {
      close(0);
      assert(dup(inpFwd) == 0);
    }

    if (pipe_Fwd[1] == 1)
    {
      close(pipe_Fwd[1]);
    }

    int i = 0;

    while (tokens[tokens_iter] != NULL)
    {
      currCmd[i] = tokens[tokens_iter];
      i++;
      tokens_iter++;
    }

    currCmd[i] = NULL;
    redirection = isRedirect(currCmd);
    if (redirection == -1)
    {
      execCmd(currCmd);
    }
    else
    {
      execRedirect(currCmd, redirection);
    }

    close(pipe_Fwd[0]);
    close(inpFwd);
    exit(0);
  }

  /* This code block is executed when the process is the parent process,
   i.e., when pid != 0. Here, the parent process waits for the child process
    to finish its execution using the wait() function. If the child process
    has exited with a status other than 0, an error message is printed to
    the console informing the user that an error has occurred while
    performing the redirection. If the child process has executed
    successfully, this code block returns 0 to the calling function,
    indicating that the redirection was performed without any errors. */
  else
  {
    wait(&state_check);
    if (!(WIFEXITED(state_check) && WEXITSTATUS(state_check) == 0))
    {
      printf("Error performing redirection!\n");
      return -1;
    }
    return 0;
  }
}

// to execute the command entered on the shell
int execCmd(const char *const *tokens)
{
  pid_t pid;
  pid = fork();
  int status;
  int exitStatus;

  if (pid == 0)
  {
    exitStatus = execvp(tokens[0], tokens);
    if (exitStatus != NULL)
    {
      exit(1);
    }
  }
  else
  {
    wait(&status);
    if (!(WIFEXITED(status) && WEXITSTATUS(status) == 0))
    {
      printf("%s: command not found\n", tokens[0]);
      return 1;
    }
  }

  return 0;
}

// Reads from the file specified by a path, if it is valid.
// Terminates if one of the commands was exit or `CTRL + D`
int execSource(const char *path)
{
  FILE *file;
  file = fopen(path, "r"); // open the file at the specified path, with read permission

  // if the file was opened successfully
  if (file != NULL)
  {
    char input[LINE_LENGTH];
    char string[LINE_LENGTH] = "source ";

    strcat(string, path); // appends the input from the file to the string[] array

    // as long as some input is coming from the opened file into the input stream
    while (fgets(input, LINE_LENGTH, file))
    {
      // if the string and input from file are the same, exit function
      // an infinite loop is possible in this case as our source command would run the same source command from the file which would again call the same source command from the same file, and so on...
      if (strncmp(string, input, strlen(string)) == 0)
      {
        printf("Error: cannot call same command (Infinite Loop Possible).\n");
        return 0;
      }
      if (sepCommmand(input) == 1)
      {
        return 1;
      }
    }
    return 0;
  }
  // if the file couldn't be opened successfully
  else
  {
    printf("Invalid file path.\n");
    return -1;
  }
}

// To basically manage the shell and run the relevant functions for the each entered command
int manageShell(const char *const *tokens, char *cmd)
{
  int type = isRedirect(tokens); //  to check if there is any redirection or not

  // if the command entered is 'exit'
  if (isExit(tokens[0]) == 0)
  {
    free_tokens(tokens);
    return 1;
  }
  // if the command entered is a pipe
  else if (isPipe(tokens) == 0)
  {
    execPipe(tokens);
  }
  // if the command entered is a redirection
  else if (type != -1)
  {
    execRedirect(tokens, type);
  }
  // if the command entered is 'source'
  else if (strcmp("source", tokens[0]) == 0)
  {
    if (execSource(tokens[1]) == 1)
    {
      free_tokens(tokens);
      return 1;
    }
  }
  // if the command entered is 'prev'
  else if (isPrev(tokens[0]) == 0)
  {
    execPrev(cachedPrevCmd);
  }
  // if the command entered is 'cd'
  else if (strcmp("cd", tokens[0]) == 0)
  {
    if (isCd(tokens[1]) == -1)
    {
      printf("Error changing directory: please enter a valid path.\n");
    }
  }
  // if the command entered is 'help'
  else if (isHelp(tokens[0]) == 0)
  {
    // do nothing, just display the help menu.
  }
  else
  {
    if (strstr(cmd, "\n") != NULL)
    {
      cmd[strlen(cmd) - 1] = '\0';
    }
    // if the command has been executed, update prevCmd with it
    if (execCmd(tokens) == 0)
    {
      strcpy(cachedPrevCmd, cmd);
    }
  }
  return 0;
}

// If manageShell returns 1, then we return 1 in order to exit the program
// Free the tokens at every iteration, and get the next command to be executed.
int sepCommmand(char cmd[]) // Check here if error
{
  char *currentCmd;

  // Convert input into different commands
  // get the first command seperated by ;
  currentCmd = strtok(cmd, ";");

  // Keeps looping till the end of commands:
  // Get the tokens from the currentCmd, and call manageShell
  while (currentCmd != NULL && currentCmd[0] != '\n')
  {
    char **getTokens = create_tokens(currentCmd);
    assert(getTokens != NULL);

    // If manageShell returns 1, then exit func
    if (manageShell(getTokens, currentCmd) == 1)
    {
      return 1;
    }

    free_tokens(getTokens);

    currentCmd = strtok(NULL, ";");
  }

  return 0;
}

// Main keeps running the shell until the user enters exit or cmd-d
int main(int argc, char **argv)
{
  printf("Welcome to mini-shell.\n");
  // to keep the shell running (technically) forever
  while (1)
  {
    char input[LINE_LENGTH];
    printf("shell $ ");
    int result = fgets(input, LINE_LENGTH, stdin);
    if (result == NULL)
    {
      printf("\nBye bye.\n");
      return 0;
    }

    if (sepCommmand(input))
    {
      break;
    }
  }

  return 0;
}

