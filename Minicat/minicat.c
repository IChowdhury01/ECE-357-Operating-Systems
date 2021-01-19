#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define DEFAULTBUF 4096     // Default Buffer size

// Infiles and outfiles beginning with a dash will be parsed correctly using this program, as normal infiles/outfiles would be.

void copyfile(int infile, int outfile, int buffersize);     /* Declaring function for copying file*/


int main(int argc, char *argv[])
{
    int fd = STDIN;  // File descriptor for infile, set to standard input by default
    int fdout = STDOUT;  // File descripter for outfile, set to standard output by default
	int size = DEFAULTBUF;   // Initialize buffer with default size
    int ocount = 0; // Counts number of times -o is used
 
    if(argc == 1) {     // Case 1: no parameters
        copyfile(fd, fdout, size);      // Copy file from standard input to standard output, using size buffer
    }
    else 
    { 
        while(--argc > 0) {
            ++argv;

            if (strcmp(*argv, "-b") == 0) {       // Test if buffer parameter is used.
                size = atoi(*++argv);
                --argc;
            }
            
            else if (strcmp(*argv, "-o") == 0) {   // Test if outfile parameter is used                
                --argc;

                if (ocount > 0) {
                    fprintf(stderr,"Error: multiple output files not allowed\n");
                    exit(-1);
                }
                else {
                    ++ocount;
                    
                    if((fdout = open(*++argv,O_RDWR|O_CREAT|O_TRUNC,0666)) == -1) {   // Open outfile for writing
                        fprintf(stderr,"Error: failed to open file %s for writing\n%s",*argv,strerror(errno));
                        exit(-1);
                    }
                }
            }
            else {
                if((fd = open(*argv,O_RDONLY)) == -1) {       // Open infile for reading
                    fprintf(stderr,"Error: failed to open file %s for reading\n%s",*argv,strerror(errno));
                    exit(-1);
                }
                copyfile(fd,fdout,size);    // Copy infile contents to outfile
            }
        }  
        
        if(close(fd) == -1) {   // Close files
            fprintf(stderr,"Error: failed to close input file %s\n%s",fd,strerror(errno));
            exit(-1);
        }
        if(close(fdout) == -1) {
            fprintf(stderr,"Error: failed to close outfile %s\n%s",fdout,strerror(errno));
            exit(-1);        
        }
    }
    return(0);
}

void copyfile(int infd, int outfd, int bufsize)     // Copyfile function: copies infile to outfile, using file descriptors. Source: http://www.learntosolveit.com/cprogramming/Ex_8.1_mycat.html 
{
    int i;    
    char * buf = malloc(bufsize);   // Dynamically allocate memory to buffer array

    if (buf == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory\n%s",strerror(errno));
        exit(-1);
    }  

    while((i = read(infd, buf, bufsize))) {     // Copies infile characters to outfile
        if(i == -1) {
            fprintf(stderr, "Error: Reading failed\n%s",strerror(errno));
            exit(-1);
        }
        else {
            if(write(outfd, buf, i) != i) {
                fprintf(stderr, "Error: Writing failed\n%s",strerror(errno));
                exit(-1);
            } 
        }  
    }
    
    free(buf);  // Frees memory from buffer array
}