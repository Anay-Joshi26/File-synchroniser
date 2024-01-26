
// Added headers
#define _POSIX_C_SOURCE  200809L
#include <string.h>
#include  <stdio.h>
#include  <stdbool.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <sys/param.h>
#include <regex.h>
#define	CHECK_ALLOC(p) if(p == NULL) {perror(__func__); exit(EXIT_FAILURE);}

// GLOBAL CONSTANTS (for the flags and associated options aswell as other variables)
extern bool verbose;
extern bool a_flag; 
extern bool r_flag;  
extern bool n_flag;
extern bool p_flag;
extern bool i_flag;
extern bool o_flag;
extern int nDirectories;
extern int nPatterns;

// Array of globs and the -i/-o type
extern struct regex_val {
    char type; // -i or -o
    char *glob;
} *regex_values;

// A typedef which represents a File
typedef struct File{
    char *filename;
    char *file_path;
    time_t modification_time;
    mode_t permissions;
}File;

// A directory struct which is self-referentially defined to also contain an array of (sub) Directories
// This creates a tree data structure allowing systematic traversal of a file-system
typedef struct Directory{
    char* name;
    File *files;
    struct Directory *sub_directories;
    int nSubDirs;
    int nFiles;
    bool isEmpty;
} Directory;

extern Directory *Directories;

// GLOBAL FUNCTIONS
extern void scan_directory(char *, Directory *);
extern void sync_files();
extern Directory createSubDir(char*);
extern File createFile(char*, char*, time_t, mode_t);
extern Directory* addSubDir(Directory*, Directory);
extern void addFile(Directory *, File);
extern regex_t get_comped_regex(struct regex_val);



