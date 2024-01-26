#include "mysync.h"
#define OPTLIST "arnpvo:i:"


// Initialise global variabls
bool a_flag = false;
bool r_flag = false;
bool n_flag = false;
bool p_flag = false;
bool i_flag = false;
bool o_flag = false;
bool verbose = false;
int nDirectories = 0;
Directory *Directories = NULL;
int nPatterns = 0;
struct regex_val* regex_values = NULL;

void printDirectories(Directory* dir, int depth) {
    if (dir == NULL) {
        return;
    }

    // Print the current directory name
    printf("%*s[%s] (empty: %s)\n", depth * 4, "", dir->name, dir->isEmpty ? "true" : "false");

    // Print files in the current directory
    for (int i = 0; i < dir->nFiles; i++) {
        printf("%*s=> %s\n", (depth + 1) * 4, "", dir->files[i].filename);
    }

    // Recursively print subdirectories
    for (int i = 0; i < dir->nSubDirs; i++) {
        printDirectories(&(dir->sub_directories[i]), depth + 1);
    }
}

void printRegexValues(struct regex_val *regex_values, int nPatterns) {
    for (int i = 0; i < nPatterns; i++) {
        printf("Entry %d:\n", i);
        printf("Type: %c\n", regex_values[i].type);
        printf("Glob: %s\n", regex_values[i].glob);
        printf("\n");
    }
}

void usage(char* prog){
    fprintf(stderr, "Usage: %s [-a] [-i pattern] [-r] [-n] [-o pattern] [-p] [-v] directory1 directory2 [directory3 ...]\n", prog);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    int opt;

    // Process command-line options using getopt()
    while ((opt = getopt(argc, argv, OPTLIST)) != -1) {
        switch (opt) {
            case 'a':
                a_flag = true;
                break;
            case 'i':
                // populate the regex_values array
                i_flag = true;
                regex_values = (struct regex_val*) realloc(regex_values, (nPatterns+1)*sizeof(struct regex_val));
                CHECK_ALLOC(regex_values);
                regex_values[nPatterns].glob = strdup(optarg); 
                regex_values[nPatterns].type = 'i';
                nPatterns++;
                break;
            case 'r':
                r_flag = true;
                break;
            case 'n':
                n_flag = true; verbose = true; // -n sets -v
                break;
            case 'p':
                p_flag = true;
                break;
            case 'o':
                o_flag = true;
                regex_values = (struct regex_val*) realloc(regex_values, (nPatterns+1)*sizeof(struct regex_val));
                CHECK_ALLOC(regex_values);
                regex_values[nPatterns].glob = strdup(optarg); 
                regex_values[nPatterns].type = 'o';
                nPatterns++;
                break;
            case 'v':
                verbose = true; 
                break;
            case '?':
                // Handle invalid options or missing arguments
                usage(argv[0]);
            default:
                fprintf(stderr, "Some error occured!: %c\n", opt);
                exit(EXIT_FAILURE);
        }
    }

    if (verbose) printRegexValues(regex_values, nPatterns);

    nDirectories = argc-optind;

    if (nDirectories < 2){
        fprintf(stderr,"Error: Need at least 2 directories to sync\n");
        exit(EXIT_FAILURE);
    }
    

    // malloc() a new Directories array which is of size nDirectories
    Directories = (Directory *)malloc(nDirectories * sizeof(Directory));
    CHECK_ALLOC(Directories);
    for (int i = optind; i < argc; i++) {
        Directories[i-optind].name = argv[i];
        if (verbose) printf("Directory: %s\n", Directories[i-optind].name);
    }

    if (verbose){
        printf("-a: %d\n", a_flag);
        printf("-r: %d\n", r_flag);
        printf("-n: %d\n", n_flag);
        printf("-p: %d\n", p_flag);
        printf("-v: %d\n", verbose);
    }

    // Build tree for each command line directory
    for (int i = 0; i < nDirectories; i++){
        scan_directory(Directories[i].name, &Directories[i]);
    }

    if (verbose) {
        printf("Directory tree of Files & Directories considered for syncing (empty directories will not be synced, except for the top level directory)\
        \nBEFORE SYNC:\n---------------------------------------------------\n");
        for (int i = 0; i < nDirectories; i++) {printDirectories(&Directories[i], 0); printf("\n");}
    }


    // Function to carry out all syncing operations made in copy_files.c
    sync_files();

    if (verbose) {
        printf("\n\nDirectory tree of Files & Directories considered for syncing (empty directories will not be synced, except for the top level directory)\n\
        AFTER SYNC:\n-----------------------------------------------------\n");
        for (int i = 0; i < nDirectories; i++) {printDirectories(&Directories[i], 0); printf("\n");}
    }
    
    // Free memory used by the directories trees and the regex_values array of structs
    free(Directories);
    free(regex_values);
    exit(EXIT_SUCCESS);
}
