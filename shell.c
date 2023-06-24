/*********************

* Code based on code CS 162 assignment from UC Berkeley
*********************/

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <dirent.h>

#include "tokenizer.h"



/* Convenience macro to silence compiler warnings about unused function parameters. */
#define unused __attribute__((unused))
#define MAX 100
/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens *tokens);
int cmd_help(struct tokens *tokens);
int cmd_pwd(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "Show the help menu"},
  {cmd_exit, "exit", "Exit command shell"},
  {cmd_pwd,"pwd","Prints current working directory"},
  {cmd_cd,"cd","Changes the current working directory"}
};

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens *tokens) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  return 1;
}

/* Exits this shell */
int cmd_exit(unused struct tokens *tokens) {
  exit(0);
}
/*Prints current working directory*/
int cmd_pwd(unused struct tokens *tokens)
{
  char pwd[MAX];
  getcwd(pwd,sizeof(pwd));
  fprintf(stdout,"%s\n",pwd);

  return 1;
}
/*Changes current working directory*/
int cmd_cd(unused struct tokens *tokens)
{
  char *targ_dir=tokens_get_token(tokens,1);
  if (targ_dir==NULL || strcmp(targ_dir,"~")==0)
  {
    targ_dir=getenv("HOME");
  }
  if(chdir(targ_dir))
  {
    printf("No such file or directory");
  }
}
/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
      return i;
  return -1;
}
/*get all our arguments by token*/
/*This function is like the most importmant in this assignment, it make sure we have every argument/parameter we need for execution/redirection/piping*/
char ** getcommand_args(struct tokens *tokens,int* pipeNumber,int* argsNumber){
  size_t length=tokens_get_length(tokens);
  char **args=malloc((length+1)*sizeof(char*));//+1 to terminate the array with null
  int FileDescriptor;
  int argsIndex=0;
  char* token;
  for (int i=0;i<length;i++){
    //Only pass arguments and keep pipes for the shell to interpret
  token =tokens_get_token(tokens,i);
  switch (token[0]){
  /*switch case to know every little argument we are handling in this function. either a redirection or pipe or a default execution argument*/
   case '<':;//added semi-colon(Could have added curly braces) to fix declaration error bcs switch case)
    if(strcmp(token,"<")==0){
    char* newtoken = tokens_get_token(tokens,++i);
    FileDescriptor=open(newtoken,O_RDONLY,0644);
    dup2(FileDescriptor,STDIN_FILENO);
    close(FileDescriptor);
    break;
  }

   case '>':;//same as first case
    if(strcmp(token,">")==0)
    {
   char* newtoken=tokens_get_token(tokens,++i);
    FileDescriptor=open(newtoken,O_CREAT|O_TRUNC|O_WRONLY,0644);
    dup2(FileDescriptor,STDOUT_FILENO);
    close(FileDescriptor);
    break;
  }

    case '|'://same as first case 
    if(strcmp(token,"|")==0){
    args[argsIndex]=token;
    argsIndex++;
    (*pipeNumber)++;
    break;
  }

    default:
  
    args[argsIndex]=token;
    argsIndex++;
    break;
}
}
  args[argsIndex]=0;//null terminate the args array
  (*argsNumber)=argsIndex;//get our number of arguements ( since in c we have no global variables. il just pass it to the function and make sure it is saved)
  return args;
}

/*returns our full command path*/
char* run_command_with_path(char *command,char*args[]){
  char *PATH = strdup(getenv("PATH"));//get our path(took 3 hours of debugging to figure out i need to copy or dup the path)
  char *command_path;
  char *path_directory = strtok(PATH,":");
  while(path_directory!=NULL){
    command_path=(char*)malloc(strlen(path_directory)+strlen(command)+2);
    sprintf(command_path,"%s/%s",path_directory,command);
    /*we can use either sprintf for one line of code
    or we can do strcat(strcpy)(strcpy))*/
    if(access(command_path,F_OK)!=-1){//ifwe have access to the file ( with flag F_OK) means file exists. then we return the full path
      return command_path;
    }
    path_directory=strtok(NULL,":");
  }
  return NULL;
}


/*mode Execution , checks if '.'' or '/'' char to know if we running from CWD or not*/
void mode_execute(char* cmd,char** args)
{
      if(cmd[0]=='.' || cmd[0]=='/')
      {
        execv(cmd,args);
      }
     else {
      char * PathOfCommand=run_command_with_path(args[0],args);
      execv(PathOfCommand,args);
        fprintf(stdout,"Command Does not Exist: %s\n",cmd);
      }
  }
/*this to know how we gonna redirect by using FDSTDIN and FDSTOUT and our redirected commands*/
void redirection(int fdSTDIN,int fdSTDOUT,char** RedirectedCommands){
  pid_t child =fork();
  if(child==0){
    if(fdSTDIN!=STDIN_FILENO){
      dup2(fdSTDIN,STDIN_FILENO);
      close(fdSTDIN);
    }
    if(fdSTDOUT!=STDOUT_FILENO){
      dup2(fdSTDOUT,STDOUT_FILENO);
      close(fdSTDOUT);
    }
    mode_execute(RedirectedCommands[0],RedirectedCommands);
  }else{
    wait(0);
  }
}
 


/*pipe execute mode*/
void mode_pipe(char** args,int pipeNumber,int argsNumber){
  int fds[2];
  int fdSTDIN=STDIN_FILENO;
  int tokenindex=0;
   char** commandargs;
   for(int i=0;i<pipeNumber;i++)
   {
    int index=0;
    commandargs=(char**)malloc(argsNumber*sizeof(char*));
    while(args[tokenindex]){
      commandargs[index]=args[tokenindex];
      if(strcmp(commandargs[index],"|")!=0){
        index++;
        tokenindex++;
      }
      else{
        commandargs[index]='\0';
        tokenindex++;
        break;
      }
    }
    /*we will push a text file to github, it has youtube videos and stackoverflow 
    where we researched and find out how to implement pipe*/
    /*pipe fds[2] array size 2
    1 of write and other for read*/
    commandargs[index]=0;//null terminate array
    pipe(fds);//start pipe
    redirection(fdSTDIN,fds[1],commandargs);//redirect our first commands 
    free(commandargs);
    close(fds[1]);//close fd
    fdSTDIN=fds[0];
   }
   if(fdSTDIN!=STDIN_FILENO){
    dup2(fdSTDIN,STDIN_FILENO);
   }
   char** remainingCommand;
   int remainingIndex=0;
   //loop to find our remainiding command that we found using getcommand function 
   remainingCommand=(char**)malloc(argsNumber*sizeof(char*));
   for(tokenindex;tokenindex<argsNumber;tokenindex++){//this loop is to find the remaining commands by using our args 
    remainingCommand[remainingIndex]=args[tokenindex];
    remainingIndex++;
   }
   remainingCommand[remainingIndex]=0;
   mode_execute(remainingCommand[0],remainingCommand);
   //execute the remaining commands
   free(remainingCommand);
 }
  

/* Initialization procedures for this shell */
void init_shell() {


  /* Our shell is connected to standard input. */
  shell_terminal = STDIN_FILENO;

  /* Check if we are running interactively */
  shell_is_interactive = isatty(shell_terminal);

  if (shell_is_interactive) {
    /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
     * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
     * foreground, we'll receive a SIGCONT. */
    while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
      kill(-shell_pgid, SIGTTIN);

    /* Saves the shell's process id */
    shell_pgid = getpid();

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);

    /* Save the current termios to a variable, so it can be restored later. */
    tcgetattr(shell_terminal, &shell_tmodes);
  }
}



int main(unused int argc, unused char *argv[]) {
  init_shell();

  static char line[4096];
  int line_num = 0;
    signal(SIGTTOU,SIG_IGN);

  /* Please only print shell prompts when standard input is not a tty */
  if (shell_is_interactive)
    fprintf(stdout, "%d: ", line_num);
  while (fgets(line, 4096, stdin)) {
    /* Split our line into words. */
    struct tokens *tokens = tokenize(line);

    /* Find which built-in function to run. */
    int fundex = lookup(tokens_get_token(tokens, 0));

    if (fundex >= 0) {
      cmd_table[fundex].fun(tokens);
    } else {
     pid_t child=fork();
      if(child==0)//in child
      {
        
       setpgid(0,child);//put process in its own process group
          signal(SIGTSTP,SIG_IGN);
        
        char** args;
        int argsNumber=0;
        int pipeNumber=0;
        args=getcommand_args(tokens,&pipeNumber,&argsNumber);
       
        if(pipeNumber==0){
          mode_execute(args[0],args);
        }
        else{
          mode_pipe(args,pipeNumber,argsNumber);
        }
      
        exit(0);

      }
      else{
       
         tcsetpgrp(STDIN_FILENO, child);
     
      wait(0);
         tcsetpgrp(STDIN_FILENO, getpid());

  
      }
     }

      /* REPLACE the following line to run commands as programs. */
     // fprintf(stdout, "The shell doesn't know how to run programs yet.\n");
    if (shell_is_interactive)
      /* Only print shell prompts when standard input is not a tty */
      fprintf(stdout, "%d: ", ++line_num);

    /* Clean up the tokens from memory */
    tokens_destroy(tokens);
  }

  return 0;
}
//