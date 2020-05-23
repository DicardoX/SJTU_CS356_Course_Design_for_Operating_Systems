/**
 * This is a C program that simulates the UNIX shell.
 * Created by Chunyu Xue on 2019/5/23
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define MAX_LINE 80                     // The maximum length command
#define MAX_SIZE 256                    // The maximum length of each argument
#define BOUNDED_ERROR -1
#define OUTPUT_REDIRECTION 1
#define INPUT_REDIRECTION 2
#define PIPE_COMMUNICATION 3
#define WAITING 6
#define NON_WAITING 7

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

        /** Transfer to string array */
        argNum = get_input(args, cur_cmd);

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
            switch (opt) {
                case 0:
                    printf("Normal operation...\n");
                    break;
                case OUTPUT_REDIRECTION:
                    printf("Output redirection...\n");
                    break;
                case INPUT_REDIRECTION:
                    printf("Input redirection...\n");
                    break;
                case PIPE_COMMUNICATION:
                    printf("Pipe communication...\n");
                    break;
                default:
                    printf("Error occurred when determining options!\n");
                    break;
            }

            printf("Child process finished...\n");

            char line[10];
            int father_info = read(fd[0], line, 10);
            if(father_info == NON_WAITING)
                printf("osh>");

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
        }


        /** Clean input */
        //free(cur_cmd);
        for(int i=0; i < argNum; i++)
            args[i] = NULL;
        argNum = 0;
        /**
         *  After reading user input, the steps are:
         *  (1) Fork a child process using fork()
         *  (2) The child process will invoke execvp()
         *  (3) parent will invoke wait() unless command included &
         */

    }

    return 0;
}
