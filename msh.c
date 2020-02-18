/*
Name: Pratik Mahato
UTA ID: 1001661375
CSE-3320 Homework #1 - Simple Shell Assignment
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"  // We want to split our command line up into tokens
                            // so we need to define what delimits our tokens.
                            // In this case  white space
                            // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255   // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10  // shell supports ten arguments

#define MAX_NUM_HISTORY_ENTRIES 15  // No of history entries to be printed
#define MAX_NUM_PID_ENTRIES 15     // No of PID no. entries to be printed

int main()
{
  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  int counter = 0;  //Counts for every command that is given by user
                    // Used for printing (MAX_NUM_HISTORY_ENTRIES) history entries by changing value of it when over(MAX_NUM_HISTORY_ENTRIES)

  char TrackCommands[MAX_NUM_HISTORY_ENTRIES][MAX_COMMAND_SIZE]= {{0}}; // Track commands given by user in a 2D array of strings
                                                                        // doesnt track for \n or NULL values

  int TrackPIDs[MAX_NUM_PID_ENTRIES] = {0}; //array for tracking child PIDs

  int PIDno = 0; // same as counter --> counts for every child executed instead

  int skip = 0 ; // for "!n" history command function
                 // when 1, doesnt ask for userinput of command, automatically helps to execute the command in that history index
                 // when 0, runs basic while() loop below with userinput

  while(1)
  {
    fflush(stdout);

    // Runs if its not a history "!n" command; dont need to ask userinput to execute !n
    if(skip == 0)  {
     // Print out the msh prompt
      printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
      fgets(cmd_str, MAX_COMMAND_SIZE, stdin);
    }

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int token_count = 0;

   // Pointer to point to the token
   // parsed by strsep
    char *arg_ptr;

    char *working_str  = strdup( cmd_str );

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;
    skip = 0; // After executing once for skip = 1 , resets its value
              // To make loop run with user input after the "!n" command executes fully at its index

    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
            (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
      token_count++;
    }

    // If user input is "\n" or null, ignore rest with "continue" and run loop again
    if(token[0] == NULL)
    {
      continue;
    }

    //If the userinput starts with '!', dont count it as a part of a history index
    //Only to make the command executed by '!n' as a history index
    // For instance, executing "!3" with command "ls" in history 3rd command, "!3" wouldnt be printed in history but "ls" prints
    if(token[0][0]!= '!')
    {
      //Runs if history list is over MAX_NUM_HISTORY_ENTRIES
      //only need to handle for MAX_NUM_HISTORY_ENTRIES
      if(counter == MAX_NUM_HISTORY_ENTRIES)
      {
        int j;
        for(j = 0; j < (counter - 1); j++)
        {
          //copies 2nd history command string to 1st history command string of a history array
          // annd 3rd to 2nd..
          strcpy(*(TrackCommands+j),*(TrackCommands+(j+1)));
        }
        counter = counter - 1;
      }
      //if history list < MAX_NUM_HISTORY_ENTRIES, keep adding on history list per command input
      strcpy(*(TrackCommands+counter), cmd_str);

      counter++; //increase for per command
                // also helps to keep one command in different increasing indices of history array
    }

     //If command is exit or quit, exit from msh shell
      if(strcmp(token[0],"quit") == 0 || strcmp(token[0], "exit") == 0 )
      {
        //need to free the allocated memory to avoid memory leak
        free( working_root );
        break;
      }

      //For "history" command input, show all the commands input by user
      // "!n" command executed would print the command at 'n' in history
      else if(strcmp(token[0],"history") == 0)
      {
        int i;

        for(i = 0; i < counter ; i++ )
        {
          printf("%d  %s",i+1,TrackCommands[i]);
        }
      }

      //For "showpids" command input, show all the child processes PIDs
      else if(strcmp(token[0],"showpids")== 0)
      {
        int i;
        for(i = 0; i < PIDno; i++ )
        {
          printf("%d\t%d\n",i+1,TrackPIDs[i]);
        }
      }

      //For "!n" command
      else if(token[0][0] == '!') // extra to be implemented: !a == a.out
      {
        char *ExecuteCommand;
        //char CommandToExecute[15][MAX_NUM_ARGUMENTS];

        //Take the string after '!'
        ExecuteCommand = strtok(token[0],"!");

        //Turn the string to integer
        int CheckInteger = atoi(ExecuteCommand); //strtol works too?

        //After atoi, if it isnt a number or if it is a zero
        // n is a number between 1 & MAX_NUM_HISTORY_ENTRIES
        // If !n for n > MAX_NUM_HISTORY_ENTRIES
        if(CheckInteger > MAX_NUM_HISTORY_ENTRIES || CheckInteger == 0)
        {
          printf("-msh: %s command not found in history\n",token[0]);
          //printf("-msh: %s event not found!\n",token[0]); //omega : 127fjn only prints 127?
          fflush(NULL);
          continue;
        }
        //strcpy(cmd_str,strcpy(CommandToExecute[1], TrackCommands[CheckInteger-1]));

        //copy the command string stored as 'n' command given by user, taken from history of commands array
        strcpy(cmd_str,TrackCommands[CheckInteger-1]);

        //print the command at 'n' in history and then later execute it
        printf("%s",cmd_str);

        //dont need to ask for userinput to execute command at n
        skip = 1;
        continue;
      }

      //for command "cd"
      else if(strcmp(token[0], "cd")==0)
      {
        //chdir implements the change directories as in linux environment
        // cd ~ not implemented
        if (chdir(token[1]) == -1)
        {
          //if no directories found of given directory to change
          printf("%s : No such file or directory.\n", token[1]);
          continue;
        }
      }
      else
      {
        pid_t pid = fork();
        if(pid == 0 ) //Child Process
        {
          int token_index;

          //execvp takes all arguments
          //Executes entered commands by searching in path order: Current Working directory
          //                                                     /usr/local/bin
          //                                                     /usr/bin
          //                                                     /bin

          if(execvp(token[0], token) == -1) //input command not found
          {
            printf("-msh: ");

            //take all the strings in a single command given by the user
            for( token_index = 0; token_index < token_count; token_index ++ )
            {
              printf( token[token_index]);
              printf(" ");
            }
            printf(": Command not found.\n");
            fflush(NULL);
            break;
          }

        }
        else if(pid == -1) //if fork failed
        {
          perror("Fork Failed.");
          fflush(NULL);
          exit(0);
        }
        else
        { // Parent process
          int wait;
          if(PIDno == MAX_NUM_PID_ENTRIES) // same as updating history before
          {                                //Runs if PID list is over MAX_NUM_PID_ENTRIES
                                           //only need to handle for MAX_NUM_PID_ENTRIES
            int k;
            for(k = 0; k < (PIDno - 1); k++)
            {
              //copies 2nd PIDno. to 1st PIDno. PID array
              // annd 3rd to 2nd..
              TrackPIDs[k] = TrackPIDs[k+1]; //
            }
          }

          //if PID list < MAX_NUM_PID_ENTRIES, keep adding on PID list per child process
          TrackPIDs[PIDno]= pid;

          //increase PID counter per child process; helps to assign PID no to each PID list indices
          PIDno++;

          //wait for child process to execute first
          //we dont need zombieeess.. T__T
          waitpid(pid,&wait,0);
        }
      }

      //Need to free allocated memory.
      //We dont need memory leaks! No virtual plumbers >.<
      free( working_root );
    }
    return 0;
  }
