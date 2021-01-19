/* 
Ivan Chowdhury 
ECE-357: Computer Operating Systems
Fall 2018 
Professor Hakner
Program 5 - Find and Replace String in Multiple Files
*/

#define _GNU_SOURCE // Used for memmap and other memory management functions

#include <stdio.h> 
#include <stdlib.h> 
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>   // Defines flags for memory management functions


int find_replace (void *memMap, char *target, char *replacement, int FileSize);

int main (int argc, char *argv[]) {

    char *target = argv[1]; 
    char *replacement = argv[2];

    char *FileName;
    void *memMap;
    int fd, FileSize;

    if (argc < 4) {
        fprintf (stderr, "Error: Invalid usage\nCorrect usage: smear TARGET REPLACEMENT file1 [file2...]\n");
        return -1;
    }
    else if (strlen (target) != strlen (replacement)) {
        fprintf (stderr, "TARGET and REPLACEMENT strings must be equal in length.\n");
        return -1;
    }
    else if (strcmp (target, replacement) == 0) {
        fprintf (stderr, "TARGET and REPLACEMENT strings are identical.\n");
        return -1;
    }    
    else {
        
        for (int i = 3; i < argc; i++) {    
            
            FileName = argv[i];   // argv[3+] are infiles
            struct stat st; // Declare stat structure

            if ((fd = open (FileName, O_RDWR)) == -1) {; // Open infile for reading and writing
                fprintf (stderr, "Error: Failed to open infile for reading and writing: %s", strerror (errno));
                return -1;          
            }
            if ((fstat (fd, &st)) == -1) {  // Retrieve filesize
                fprintf (stderr, "Error: Failed to find size of file %s: %s\n", FileName, strerror (errno));
                return -1;
            }
            FileSize = st.st_size;  // Set filesize

            if ((memMap = mmap (0, FileSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {  // Map file to virtual memory space     
                fprintf (stderr, "Error: Failed to set up memory mapping for file %s: %s\n", FileName, strerror (errno));
                return -1;
            }

            find_replace (memMap, target, replacement, FileSize);   // Call find/replace function

            if (munmap (memMap, FileSize) == -1) {  // Unmap file from virtual memory space
                fprintf (stderr, "Error: Failed to unmap file %s from virtual memory space: %s\n", FileName, strerror (errno));
                return -1;
            }

            if (close (fd) == -1) { // Close open file descriptor
                fprintf (stderr, "Error: Failed to close file %s: %s", FileName, strerror (errno));
                return -1;
            }
        }
    }
    return 0;
}

int find_replace (void *memMap, char *target, char *replacement, int FileSize) {
    void *target_pos;  // Pointer to first occurence of target substring in memory area
    size_t replacement_len = strlen (replacement);  // String lengths
    size_t target_len = strlen (target);   

    while ((target_pos = memmem (memMap, FileSize, target, target_len))) {  // Find target substring in memory area
        if (target_pos == NULL) {
            fprintf (stderr, "Error: Target string not found: %s\n", strerror (errno));
            return -1;
        }
        memcpy (target_pos, replacement, replacement_len);  // Copy replacement string over target string

        FileSize -= abs(memMap - target_pos);   // Reset filesize value
        memMap = target_pos + 1;    // Reset memory area
    }
    return 0;
}