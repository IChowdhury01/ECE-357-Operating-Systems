/*  Ivan Chowdhury
    ECE-357: Computer Operating-Systems
    Recursive Filesystem Lister in C */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <time.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>

int DirectorySearch (char *currentdir);
int printMode(void);

struct stat st;

int main (int argc, char *argv[]) {
    char *stdirectory;

    if (argc > 2) {
        fprintf(stderr, "Error: Too many arguments supplied");
        exit(-1);
    }
    else if (argc == 2) {
        stdirectory = *++argv;
    }
    else if (argc == 1) {
        stdirectory = ".";
    }
    else {
        fprintf(stderr, "Error: Invalid number of arguments");
        exit(-1);
    }
    DirectorySearch(stdirectory);
    return 0;
}

int printMode() {    // Prints mode string
    printf((S_ISDIR(st.st_mode)) ? "d" : "-");  //start string with d if directory, - otherwise
    printf((st.st_mode & S_IRUSR) ? "r" : "-");
    printf((st.st_mode & S_IWUSR) ? "w" : "-");
    printf((st.st_mode & S_IXUSR) ? "x" : "-");
    printf((st.st_mode & S_IRGRP) ? "r" : "-");
    printf((st.st_mode & S_IWGRP) ? "w" : "-");
    printf((st.st_mode & S_IXGRP) ? "x" : "-");
    printf((st.st_mode & S_IROTH) ? "r" : "-");
    printf((st.st_mode & S_IWOTH) ? "w" : "-");
    printf((st.st_mode & S_IXOTH) ? "x" : "-");
    return 0;
}

int DirectorySearch (char *curDirectory) {
    DIR *dirStream; 
    struct dirent *ndEntry;
    struct tm timedata;
    struct gid *grp;
    struct uid *usr;

    char path[1024], link[1024], date[50]; 
    char *user, *group;
    int ptest;

    if ((dirStream = opendir(curDirectory)) == NULL) {   // Opening directory
        fprintf(stderr,"Error: Failed to open directory %s: %s\n", curDirectory, strerror(errno));
        exit(-1);
    }
    errno = 0;  // Preclear for readdir error detection
    
    while (ndEntry = readdir(dirStream))    // Read through directory
    {
        if(ndEntry == NULL) {
            fprintf(stderr, "Error: Reading failed for directory %s: %s\n", curDirectory, strerror(errno));
            exit(-1);
        }
        
        snprintf(path, sizeof(path), "%s/%s", curDirectory, ndEntry->d_name);   // Store path name
        
        if (lstat(path, &st) == -1) {   // Use stat system call to retrieve target information
            fprintf(stderr, "Error: Failed to retrieve information about file %s: %s\n", path, strerror(errno)); 
            exit(-1);
        }

        strftime(date, sizeof(date), "%m %d, %Y %H:%M", localtime(&st.st_mtime));    // Store human-readable time in string

        if(S_ISREG(st.st_mode)) {   // Print stored information as output - Regular files
            printf("%22llu %5lli \t", st.st_ino, st.st_blocks);
            if ((ptest = printMode()) != 0) {
                fprintf(stderr, "Error: Failed to print mode string: %s\n", strerror(errno));
                return -1;
            }
            printf("%5hu%15u%15u%15lli%20s\t%s\n",st.st_nlink, st.st_uid, st.st_gid, st.st_size, date, path);
        }
        else if (S_ISLNK(st.st_mode)) { // Links
            ssize_t length = readlink(path, link, 1023);
            
            if (length == -1) {
                fprintf(stderr, "Error: Failed to read path of symbolic link: %s\n", strerror(errno));
            }
            else {
                link[length] = '\0';
            }

            printf("%22llu%5lli \t", st.st_ino, st.st_blocks);

            if ((ptest = printMode()) != 0) {
                fprintf(stderr, "Error: Failed to print mode string: %s\n", strerror(errno));
                return -1;
            }
            printf("%5hu%15u%15u%15lli%20s\t%s -> %s\n", st.st_nlink, user, group, st.st_size, date, path, link);
        }
        else if (S_ISDIR(st.st_mode)) { // Directories
            if ((strcmp(ndEntry->d_name, "..") == 0) || (strcmp(ndEntry->d_name, ".") == 0)) {
                if (strcmp(ndEntry->d_name, ".") == 0) {
                    printf("%22llu%5lli \t", st.st_ino, st.st_blocks);

                    if ((ptest = printMode()) != 0) {
                        fprintf(stderr, "Error: Failed to print mode string: %s\n", strerror(errno));
                        return -1;
                    }
                    printf("%5hu%15u%15u%15lli%20s\t%s\n", st.st_nlink, user, group, st.st_size, date, curDirectory);
                }
            } 
            else {
                DirectorySearch(path);  // Recursively search through subdirectories
            }   
        }
        else {
            fprintf(stderr, "File type unknown (not a regular file, link, or directory: %s", strerror(errno));
        } 
    }
    if (closedir(dirStream) == -1) {     // Close directory
        fprintf(stderr, "Error: Failed to close directory %s: %s\n", curDirectory, strerror(errno));
        exit(-1);
    }
    return 0;
}