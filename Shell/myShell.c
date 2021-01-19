// Ivan Chowdhury
// Cooper Union ECE357: Computer Operating Systems
// Professor Hakner
// Fall 2018
// Program 3 - Simple Shell

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <sys/wait.h>

#define BUFSIZE 4096    // Default buffer size 

int myShell(FILE *input);   // Shell function
int builtpwd();     // Built-in commands: pwd, cd, and exit
int builtcd(char *CDpath);
void builtexit(char *exit_status);

int main(int argc, char **argv) {
    if (argc > 2) {     // Program takes in 1 argument max
        fprintf(stderr, "Error: Only one argument is accepted: %s\n", strerror(errno));
        return -1;
    }
    else if (argc == 2) {   // If input file argument is given, open for reading
        FILE *input;
        if ((input = fopen(argv[1], "r")) == NULL) {
            fprintf(stderr, "Error: Failed to open input file %s: %s\n", argv[1], strerror(errno));
            return -1;
        }
        myShell(input);  
        
        if (argc == 2 && fclose(input) != 0) {    // Close input file
            fprintf(stderr, "Error: Failed to close input file %s: %s\n", argv[1], strerror(errno));
            return -1;
        }
    }
    else 
        myShell(stdin);  // If no arguments given, read from standard input

    return 0;
}

int myShell(FILE *input) {   
    
    char *line;   // Buffer for line string
    char *token, **tokenArgV;    // Contains characters and argument vector for tokens
    char *delim = " \r\n";     // Delimiter for extracting tokens from string

    char *new_path;     // New path for I/O redirection
    int fdnew;      // New file descriptor for I/O redirection
    
    int flags, i;  // Flags for open syscall
    size_t n;   // getline parameters: buffer size and characters read
    ssize_t bytesRead;  

    fdnew = -1;   
    i = 0;
    n = 4096;
   
    
    if (!(new_path = malloc(BUFSIZE * (sizeof(char))))) {   // Dynamically allocate memory for new file path
        fprintf(stderr, "Error: Failed to allocate memory for redirected file path: %s\n", strerror(errno));
    }

    if (!(line = malloc(BUFSIZE * (sizeof(char))))) {   // Dynamically allocate memory for line
        fprintf(stderr, "Error: Failed to allocate memory for line buffer: %s\n", strerror(errno));
    }
    
    tokenArgV = malloc(BUFSIZE * (sizeof(char *)));     //Dynamically allocate memory for token argument vector
    if (tokenArgV == NULL)
        fprintf(stderr, "Error: Failed to allocate memory for argument vector: %s\n", strerror(errno));

    while ((bytesRead = getline(&line, &n, input)) != -1) {   // Read next line from input
     
        if (line[0] == '#' || bytesRead <= 1)   // If first character of line is #, skip to next line
            continue;
        else
        {
            token = strtok(line, delim);     // Extract tokens from string, delimited by carriage return + newline
            

            // Perform I/O redirection
            while (token != NULL) 
            {
                if (token[0] == '<') {      // If token starts with <, open filename and redirect stdin
                    fdnew = 0;
                    flags = O_RDONLY;
                    strcpy(new_path, (token + 1));
                }
                else if (token[0] == '>') { 
                    if (token[1] == '>') {      // If token starts with >>, open/create/append filename and redirect stdout
                        flags = O_WRONLY | O_APPEND | O_CREAT;
                        strcpy(new_path, (token + 2));
                    }
                    else {   // If >, open/create/truncate filename and redirect stdout
                        flags = O_WRONLY | O_TRUNC | O_CREAT;
                        strcpy(new_path, (token + 1));
                    }
                    fdnew = 1;
                }
                else if (token[0] == '2' && token[1] == '>') {
                    if (token[2] == '>') {  // If 2>>, open/create/append and redirect stderr
                        flags = O_WRONLY | O_APPEND | O_CREAT;
                        strcpy(new_path, (token + 3));
                    }
                    else {    // If 2>, open/create/truncate and redirect stderr
                        flags = O_WRONLY | O_TRUNC | O_CREAT;
                        strcpy(new_path, (token + 2));
                    }
                    fdnew = 2;
                }
                else {
                    tokenArgV[i++] = token;
                }
                token = strtok(NULL, delim); 
            }
            tokenArgV[i] = NULL;


            // Check input for built-in commands
            if (strcmp(tokenArgV[0], "pwd") == 0) { // Use built-in pwd
                builtpwd();
            }
            else if (strcmp(tokenArgV[0], "cd") == 0) { // Use built-in cd
                builtcd(tokenArgV[1]);
            }
            else if (strcmp(tokenArgV[0], "exit") == 0) {  // Use built-in exit
                builtexit(tokenArgV[1]);
            }


            // Perform fork/exec and time command execution
            else
            {
                int fd;      // File descriptor
                int pid, status;    // Child process id and exit status

                clock_t start, end;     // For storing time information
                struct tms time_start, time_end;  

                if ((start = times(&time_start)) == -1) {   // Begin timing command execution
                    fprintf(stderr, "Error: Failed to start timing command: %s\n", strerror(errno));
                    return -1;
                }

                // Use fork to create duplicate child process, test return values
                if ((pid = fork()) == -1) {     // Fork process fails
                    fprintf(stderr, "Error: Failed to fork process: %s\n", strerror(errno));
                    exit(-1);
                }
                else if (pid == 0) {    // Fork succeeds
                    if (fdnew > -1)   
                    {
                        if ((fd = open(new_path, flags, 0666)) != -1) {     // I/O Redirection: duplicate file and close old file descriptor
                            if (dup2(fd, fdnew) == -1) {
                                fprintf(stderr, "Error: Failed to duplicate file for I/O redirection: %s\n", strerror(errno));
                                return -1;
                            } 
                            else if (close(fd) == -1) {
                                fprintf(stderr, "Error: Failed to close file for I/O redirection: %s\n", strerror(errno));
                                return -1;
                            }
                        } 
                        else {
                            fprintf(stderr, "Error: Failed to open file %s for I/O redirection: %s\n", new_path, strerror(errno));
                            return -1;
                        }
                    }
                    if (execvp(tokenArgV[0], tokenArgV) == -1) {   // Execute program via program's name and argument vector
                        fprintf(stderr, "Error: Failed to execute command %s: %s\n", tokenArgV[0], strerror(errno));
                        return -1;
                    }
                }
                else {    
                    if (wait(&status) == -1) {  // Wait for process to change state.
                        fprintf(stderr, "Error: Failed to wait for the child process %s to complete\n", tokenArgV[0], strerror(errno));
                        return -1;
                    }
                    if ((end = times(&time_end)) == -1) {   // Time command execution
                        fprintf(stderr, "Error: Failed to get end timing of command:%s\n", strerror(errno));
                        return -1;
                    }

                    long clktck = clktck = sysconf(_SC_CLK_TCK);    // Number of clock ticks per second for kernel

                    fprintf(stderr, "Command returned exit status: %d\n", status);     // Print shell messages
                    fprintf(stderr, "consuming %f real seconds, %f user, %f system.\n", (end-start) / (double) clktck,  (time_end.tms_cutime-time_start.tms_cutime) / (double) clktck, (time_end.tms_cstime-time_start.tms_cstime) / (double) clktck);   
                }
            }
            fdnew = -1;     // Reset parameters for next input line
            flags = 0;
            i = 0;
        }
    }

    free(line);     // Free dynamically allocated memory
    free(new_path);    
    free(tokenArgV);

    return 0;
}

// Built in functions
int builtpwd() {    // Built-in pwd command
    char *WDpath;   // Buffer for holding current working directory path
    if (!(WDpath = malloc(BUFSIZE * sizeof(char)))) {
        fprintf(stderr, "Error: Failed to allocate memory for current working directory path: %s\n", strerror(errno));
        return -1;
    }
    if (!(getcwd(WDpath, BUFSIZE))) {
        fprintf(stderr, "Error: Failed to retrieve current working directory path: %s\n", strerror(errno));
        return -1;
    }
    else {    
        printf("%s\n", WDpath);    // Print current working directory
    }
    free(WDpath);
    return 0;
}

int builtcd(char *CDpath) {    // Built in cd command
    if (CDpath == NULL && chdir (getenv ("HOME")) == -1 || CDpath != NULL && chdir (CDpath) == -1) {    // Receive HOME path from environment variable if no argument given, use given path otherwise.
        fprintf(stderr, "Error: Failed to change to directory %s: %s\n", CDpath, strerror(errno)); 
        return -1;
    }

    return 0;
}

void builtexit(char *exit_status) {     // Built-in exit command
    if (exit_status == NULL)   // Default exit status = 0
        exit(0);    
    else
        exit(atoi(exit_status));   // Convert from string to integer and use given exit status.
}