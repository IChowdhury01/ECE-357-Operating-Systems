// Ivan Chowdhury
// ECE357: Computer Operating Systems
// Concatenation using pipes and signal handling

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define DEFAULTBUF 4096     


void handler(int signo);
void closePipes();


int byteCount = 0, fileCount = 0; // Initialize counter for number of bytes and files processed
int signo;  // Signal number
int pGrep[2], pMore[2]; // Declare pipe file descriptors

struct sigaction sa = {     // Sigaction struct to change signal response
    .sa_handler = handler,  
    .sa_mask = 0,
    .sa_flags = 0
};


void handler(int signo) { // Signal handler function
    fprintf(stderr,"Files processed: %d, Bytes processed: %d\n", fileCount, byteCount);  // Print number of files and bytes processed
    exit(EXIT_SUCCESS); 
}

void closePipes() { // Closes all open pipe file descriptors
    
    if(close(pGrep[0]) == -1) {
        fprintf(stderr, "Error: Failed to close read end of grep pipe: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(close(pGrep[1]) == -1){
        fprintf(stderr, "Error: Failed to close write end of grep pipe: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(close(pMore[0]) == -1) {
        fprintf(stderr, "Error: Failed to close read end of more pipe: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(close(pMore[1]) == -1) {
        fprintf(stderr, "Error: Failed to close write end of more pipe: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {


    int grep_status, more_status; // Status variable for grep and more processes
    pid_t morePid, grepPid; // grep and more process ids
    int fd, rdbytes, wbytes;  // Read/Write variables


    char * pattern = malloc((sizeof(char)) * DEFAULTBUF);    // Dynamically allocate memory for read/write buffer and pattern string
    char * buf =  malloc((sizeof(char)) * DEFAULTBUF);
    if(!pattern || !buf) {  
        fprintf(stderr, "Error: Failed to allocate memory: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    
    strcpy(pattern, argv[1]);  // Pattern string holding the user-specified pattern to search for


    if(argc < 3) {  // Argument check
        fprintf(stderr,"Error: Incorrect usage. Correct syntax:\n./catgrepmore pattern infile1 [...infile2...]\n");
        exit(EXIT_FAILURE);
    }


    sigemptyset(&sa.sa_mask);   // Initialize signal mask as empty

    if(sigaction(SIGINT, &sa, 0) == -1) {    // Sigaction - use handler function instead of standard SIGINT response
        fprintf(stderr, "Error: Failed to handle signal interrupt\n %s\n", strerror(errno));
    }


    for(int i = 2; i < argc; i++) {  // Loops through arguments. At least twice (if 1 infile)

        printf("\nFilename: %s\n", argv[i]);    // Print filename

        if (pipe(pGrep) == -1) { // Create pipes
            fprintf(stderr, "Error: Failed to create grep pipe: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (pipe(pMore) == -1) {
            fprintf(stderr, "Error: Failed to create more pipe: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        switch(grepPid = fork()) {  // Fork grep process      
            case 0: // Perform redirection
                if(dup2(pGrep[0], STDIN) == -1 ) {
                    fprintf(stderr, "Error: Failed to redirect the read end of the grep pipe to standard input: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                if(dup2(pMore[1], STDOUT) == -1) {
                    fprintf(stderr, "Error: Failed to redirect the write end of the more pipe to standard output: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }

                closePipes();   

                if (execlp("grep","grep",pattern, NULL) == -1) {  // Execute grep command
                    fprintf(stderr, "Error: Execution of grep command has failed: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                break;

            case -1:    // Error handling for fork()
                fprintf(stderr, "Error: Failed to fork grep process: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
                break;
            
            default:
                break;
        }
        
        switch(morePid = fork()) {  // Fork 'more' processs
            case 0: // Perform redirection
                if (dup2(pMore[0],STDIN) == -1) { // Redirect read end of pipe to standard input
                    fprintf(stderr, "Error: Failed to redirect the read end of the more pipe to standard input: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }

                closePipes();

                if (execlp("more", "more", NULL) == -1) { // Execute more, with argument more
                    fprintf(stderr, "Error: Execution of 'more' command has failed: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }

            case -1:    // Error handling for fork()
                fprintf(stderr, "Error: Failed to fork 'more' process: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
                break;

            default:
                break;
        }

        if((fd = open(argv[i], O_RDONLY)) == -1) {    // Open infile
            fprintf(stderr, "Error: Failed to open input file %s: %s\n", argv[i], strerror(errno));
            exit(EXIT_FAILURE);
        }

        fileCount++;    // Increment counter for number of files processed 


        while ((rdbytes = read(fd, buf, sizeof(char) *DEFAULTBUF)) > 0) { // Loop through infiles, reading each byte
            if (rdbytes == -1 ) {  
                fprintf(stderr, "Error: Failed to read input file %s: %s\n", argv[i], strerror(errno));
                exit(EXIT_FAILURE);
            }
            else {
                wbytes = write(pGrep[1], buf, rdbytes);  // Write bytes to grep output pipe
                if (wbytes == -1) { 
                    fprintf(stderr, "Error: Failed to write to grep pipe: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                } 
                
                else if (wbytes < rdbytes) { // Partial write handling
                    fprintf(stderr, "Warning: Partial write occured with input file %s: %s\n", argv[i], strerror(errno));
                    rdbytes = rdbytes - wbytes;
                    buf = buf + rdbytes;
                    byteCount += wbytes;
                    wbytes = 0;
                } 
                else {
                    byteCount += wbytes; // Increment byte count by number of bytes written
                }
            }
        }

        // Close file descriptors
        if(close(fd) == -1) {
            fprintf(stderr, "Error: Failed to close input file %s: %s\n", argv[i], strerror(errno));
            exit(EXIT_FAILURE);
        }
        closePipes();

        // Wait for grep and 'more' processes to finish
        waitpid(grepPid, &grep_status, 0); 
        waitpid(morePid, &more_status, 0);
    }
    return 0; 
}
