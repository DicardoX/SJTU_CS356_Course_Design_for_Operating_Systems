/**
 * This is a C program that simulates the UNIX shell.
 * Created by Chunyu Xue on 2019/5/23
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_LINE 80                     // The maximum length command
#define MAX_SIZE 256                    // The maximum length of each argument
#define BOUNDED_ERROR -1
#define OUTPUT_REDIRECTION 1
#define INPUT_REDIRECTION 2
#define PIPE_COMMUNICATION 3
#define WAITING 8
#define NON_WAITING 12

int get_input(char *args[], char *cur_cmd)
{
    const char *separator = " \n";      // Note: can dived both on " " and "\n"
    int idx = 0;
    char *clause;

    clause = strtok(cur_cmd, separator);
    while(clause != NULL){
        if(idx == MAX_LINE / 2 + 1)
            return BOUNDED_ERROR;

        args[idx++] = clause;
        clause = strtok(NULL, separator);
    }
    args[idx] = NULL;
    return idx;                         // idx = # of arguments in args[]
}

void display_current_args(int argNum, char *args[])
{
    for(int i=0; i < argNum; i++){
        printf("%s ", args[i]);
    }
    printf("\n");
}

void Detect(int* opt, int* optIdx, char *args[], int argNum)
{
    for(int i=0; i < argNum; i++)
    {
        if(strcmp(args[i], ">") == 0){      // Output redirection
            (*opt) = OUTPUT_REDIRECTION;
            (*optIdx) = i;
            return;
        }
        else if(strcmp(args[i], "<") == 0){ // Input redirection
            (*opt) = INPUT_REDIRECTION;
            (*optIdx) = i;
            return;
        }
        if(strcmp(args[i], "|") == 0){      // Pipe communication
            (*opt) = PIPE_COMMUNICATION;
            (*optIdx) = i;
            return;
        }
    }
}

int output_redirection(int optIdx, char *args[], int *argNum)
{
    fflush(stdout);
    int fd_out = open(args[optIdx + 1], O_WRONLY | O_TRUNC | O_CREAT, 0644);          // Authority: read/write/create
    args[optIdx] = NULL;
    (*argNum) = optIdx;
    if(fd_out >= 0){
        int dup_res = dup2(fd_out, STDOUT_FILENO);                  /* just a little different from dup(), dup2() can specify the newfd*/
        if(dup_res < 0){                                         /*we want to use. It the newfd is already opened, this function will first close it and reopen */
            printf("Dup2 in output redirection error...\n");
            exit(1);
        }
    }

    if(execvp(args[0], args) < 0){
        perror("Execution error!\n");
    }

    if(close(fd_out) < 0){                                          // Close the file descriptor
        printf("Error occurred when closing file descriptor in output redirection...\n");
        exit(1);
    }
    return fd_out;
}

int input_redirection(int optIdx, char *args[], int *argNum)
{
    int fd_in = open(args[optIdx + 1], O_RDONLY, 0644);                  // Authority: read only
    char *fileName = args[optIdx + 1];
    args[optIdx] = NULL;
    (*argNum) = optIdx;
    if(fd_in >= 0){
        int dup_res = dup2(fd_in, STDIN_FILENO);
        if(dup_res < 0){
            printf("Dup2 in input redirection error...\n");
            exit(1);
        }
    }
    else{
        printf("Failed to open %s...\n", fileName);
        exit(1);
    }

    execvp(args[0], args);

    if(close(fd_in) < 0){                                          // Close the file descriptor
        printf("Error occurred when closing file descriptor in input redirection...\n");
        exit(1);
    }
    return fd_in;
}

void pipe_communication(int optIdx, char *args[], int *argNum, int *son_argNum)
{
    pid_t ppid = 0;
    int waitFlag = 1;
    int fd[2];
    char *secArgs[MAX_LINE / 2 + 1];
    char *firArgs[MAX_LINE / 2 + 1];

    for(int i = optIdx+1; i < (*argNum); i++)   // Copy the second sub command
        secArgs[i - optIdx - 1] = args[i];
    for(int i = 0; i <= optIdx; i++)
        firArgs[i] = args[i];


    (*son_argNum) =(*argNum) - optIdx - 1;      // # of arguments in the second sub command
    (*argNum) = optIdx;                         // # of arguments in the first sub command
    firArgs[(*argNum)] = NULL;                        // To remove the rest part behind < or > or |
    secArgs[(*son_argNum)] = NULL;

    if(strcmp(firArgs[(*argNum) - 1], "&") == 0){
        firArgs[(*argNum) - 1] = NULL;             // Delete "&", no matter which sub command "&" is in, the father will wait for son
        (*argNum)--;
        waitFlag = 0;
    }
    if(strcmp(secArgs[(*son_argNum) - 1], "&") == 0){
        secArgs[(*son_argNum) - 1] = NULL;         // Delete "&"
        (*son_argNum)--;
        waitFlag = 0;
    }

    if(pipe(fd) < 0){
        printf("Failed to construct pipe in pipe communication...\n");
        exit(1);
    }
    ppid = fork();
    if(ppid < 0){
        perror("Fork error!\n");
    }
    if(ppid == 0)                   // Child process writes, so close fd[0]
    {
        if(close(fd[0]) < 0){		
            printf("Error occurred when closing fd[0] in pipe communication...\n");
            exit(1);
        }


        int res_dup_son = dup2(fd[1], STDOUT_FILENO);
        if(res_dup_son < 0){
            printf("fd[0] dup in pipe communication error...\n");
            exit(1);
        }
        int res = 0;
        execvp(firArgs[0], firArgs);                  // The kernel load new program into the child process and execute
        if(res < 0){
            printf("The second command execution error in pipe communication...\n");
            exit(1);
        }
        if(close(fd[1]) < 0){
            printf("Error occurred when closing fd[1] in pipe communication...\n");
            exit(1);
        }
        exit(0);                                                    // Once child process have finished, exit(0)
    }
    else if(ppid > 0)               // Father process reads, so close fd[1]
    {
        if(close(fd[1]) < 0){
            printf("Error occurred when closing fd[1] in pipe communication...\n");
            exit(1);
        }
        wait(NULL);

        int res_dup_father = dup2(fd[0], STDIN_FILENO);
        if(res_dup_father < 0){
            printf("fd[1] dup in pipe communication error...\n");
            exit(1);
        }
        execvp(secArgs[0], secArgs);                  // Note: this will return -1 since it is exactly the current program
        if(close(fd[0]) < 0){
            printf("Error occurred when closing fd[0] in pipe communication...\n");
            exit(1);
        }
    }
}

int main(void)
{
    char *args[MAX_LINE/2 + 1];         // Command line arguments
    char *prev_args[MAX_LINE/2 + 1];    // History buffer

    int should_run = 1;                 // Flag to determine when to exit program
    int argNum = 0;                     // # of arguments in command line
    int prev_argNum = 0;                // Counter for history buffer
    char* cur_cmd;
    int fd[2];
    int osh_flag = 1;                   // For father process to determine whether print "osh>" or not
    pid_t pid = 0;

    while (should_run){
        if(osh_flag == 1) {
            printf("osh>");
        }
        fflush(stdout);
        osh_flag = 1;

        /** Read command line */
        cur_cmd = (char*)malloc(sizeof(char) * MAX_SIZE);
        fgets(cur_cmd, MAX_SIZE-1, stdin);

        if(strcmp(cur_cmd, "\n") == 0){
            printf("The current command is empty, please try again...\n");
            continue;
        }
        if(strcmp(cur_cmd, "") == 0){
            printf("Error occurred when input...\n");
            exit(1);
        }

        /** Transfer to string array */
        argNum = get_input(args, cur_cmd);
        args[argNum] = NULL;

        //printf("%d\n", argNum);
        //printf("pid: %d\n", pid);

        if(argNum == BOUNDED_ERROR){
            printf("Error: Exceed maximum length of command!\n");
            continue;
        }

        /** Exit section */
        if(argNum == 1 && strcmp(args[0], "exit") == 0){
            should_run = 0;
            continue;
        }
        /** History feature creating */
        if(strncmp(args[0], "!!", 2) == 0 && argNum == 1 && prev_argNum != 0){
            for(int i=0; i <= prev_argNum; i++){
                args[i] = prev_args[i];                 // Note: If we use strcpy(dst, src) here, there will be a problem: may take NULL as a parameter of
            }                                           // strcpy(), which will cause an exception and terminate the program.
            argNum = prev_argNum;

            // Display current command
            printf("We have executed the most recent command :\n");
            printf("   ");
            display_current_args(argNum, args);

        }
        else if(strncmp(args[0], "!!", 2) == 0 && argNum == 1 && prev_argNum == 0){
            printf("No commands in history...\n");
            continue;
        }
        prev_argNum = argNum;
        for(int i=0; i <= argNum; i++){
            prev_args[i] = args[i];
        }

        /** In case of when "&", father process print "osh>" earlier than child process print thing. So, we use pipe() to help father process
         * and child process communicate to each other to solve this.*/
        if(pipe(fd) < 0){
            printf("Pipe error!\n");
            exit(1);
        }

        /** Process Creation */
        int opt = 0;
        int optIdx = 0;
        int waitFlag = 1;                   // Whether father waits child

        if(strcmp(args[argNum - 1], "&") == 0) {    // Father process isn't going to wait
            waitFlag = 0;
            args[argNum - 1] = NULL;                // Delete "&"
            argNum--;
        }

        pid = fork();

        if(pid < 0){                                // Error occurred when fork()
            perror("Fork error!\n");
        }
        if(pid == 0)                                // Child process -> operating functions
        {
            close(fd[1]);                           // Child process use fd[0]
            // Detect if redirection or communication
            Detect(&opt, &optIdx, args, argNum);

            // Operation based on option above
            int res = -1;
            int son_argNum = 0;

            switch (opt) {
                case 0:
                    execvp(args[0], args);      // Note: this will return -1 since it is exactly the current program
                    printf("Normal operation...\n");
                    break;
                case OUTPUT_REDIRECTION:
                    res = output_redirection(optIdx, args, &argNum);
                    if(res < 0){
                        printf("Failed to output redirection...\n");
                        exit(1);
                    }
                    printf("Output redirection...\n");              // Will be written into the file
                    printf("Normal operation...\n");                // Perform the operation and write the result into the file
                    break;
                case INPUT_REDIRECTION:
                    res = input_redirection(optIdx, args, &argNum);
                    if(res < 0){
                        printf("Failed to input redirection...\n");
                        exit(1);
                    }
                    printf("Input redirection...\n");
                    printf("Normal operation...\n");                // Read the operation from the file and perform the operation
                    break;
                case PIPE_COMMUNICATION:
                    pipe_communication(optIdx, args, &argNum, &son_argNum);
                    printf("Pipe communication...\n");
                    printf("Normal operation...\n");                // Perform the first operation
                    printf("Normal operation...\n");                // Perform the second operation
                    break;
                default:
                    printf("Error occurred when determining options!\n");
                    break;
            }

            printf("Child process finished...\n");

            char line[15];
            int father_info = read(fd[0], line, 15);
            if(father_info == NON_WAITING)
                printf("osh>");

            close(fd[0]);
            exit(0);                               // Once child process have finished, exit(0)
        }
        else if(pid > 0)                            // Father process -> wait or not
        {
            close(fd[0]);                           // Father process use fd[1]
            if(waitFlag){                          // If the last argument isn't "&", wait
                write(fd[1], "Waiting\n", WAITING);
                wait(NULL);
                printf("Child Complete...\n");
            }
            else {
                write(fd[1], "Non-waiting\n", NON_WAITING);
                osh_flag = 0;
                printf("Father is going on...\n");
                signal(SIGCHLD, SIG_IGN);           // Signal to avoid zombie, inform kernel that the child process should be recycled by kernel
            }                                       // SIGCHLD is sent by child process when it finishes, SIG_IGN means father process ignore this signal

            close(fd[1]);
        }


        /** Clean input */
        //free(cur_cmd);
        for(int i=0; i < argNum; i++)
            args[i] = NULL;
        argNum = 0;
    }

    return 0;
}
