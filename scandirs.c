#include "mysync.h"
#include  <sys/stat.h>
#include  <dirent.h>
#include  <sys/param.h>
#include  <time.h>
#include  <string.h>


// This function will allocate memory for one Directory struct and store the
// path to the directory, all other attributes will be null for now
// Return a deferenced pointer to the actual struct.
Directory createSubDir(char* path) {
    Directory* new_dir = (Directory*)malloc(sizeof(Directory));
    CHECK_ALLOC(new_dir);
    new_dir->name = strdup(path);
    new_dir->files = NULL;
    new_dir->nFiles = 0;
    new_dir->nSubDirs = 0;
    new_dir->sub_directories = NULL;
    new_dir->isEmpty = true;
    return *new_dir;
}

// Creates a File type and stores the values needed into the newly created File.
// Return a dereferenced pointer to the actual struct
File createFile(char* filename, char* path, time_t modification_time, mode_t permissions) {
    File* new_file = (File*)malloc(sizeof(File));
    CHECK_ALLOC(new_file);
    new_file->filename = strdup(filename);
    new_file->file_path = strdup(path);
    new_file->modification_time = modification_time;
    new_file->permissions = permissions;
    return *new_file;
}

// Following the createSubDir(), this function will add the sub-directory
// to the parent directory. It will append the newly created directory to the sub_directories array
// in the parent.
// Return a pointer to the newly added sub-directory from within the Directories array.
Directory* addSubDir(Directory* parent, Directory sub_dir) {
    parent->sub_directories = realloc(parent->sub_directories, (parent->nSubDirs+1)*sizeof(Directory));
    CHECK_ALLOC(parent->sub_directories);
    parent->sub_directories[parent->nSubDirs] = sub_dir;
    Directory *new_sub_dir = &parent->sub_directories[parent->nSubDirs];
    parent->nSubDirs++;
    return new_sub_dir;
}

// Following the createFile(), this function will add the file
// to the parent directory. It will append the newly created file to the files array
// in the parent.
void addFile(Directory *parent, File new_file) {
    parent->files = realloc(parent->files, (parent->nFiles+1)*sizeof(File));
    CHECK_ALLOC(parent->files);
    parent->files[parent->nFiles] = new_file;
    parent->nFiles++;
}

// Walk/traverse a given directory, possibly recursively and build a dynamic tree structure
// as it sees files and directories
// Empty directories will be marked as empty and ignored during syncing.
void scan_directory(char *dirname, Directory *parent) {

    struct dirent *dp;
    DIR *dirp = opendir(dirname);

    if (dirp == NULL) {
        perror(dirname);
        exit(EXIT_FAILURE); 
    }

    // Traverse the directorie's entries
    while ((dp = readdir(dirp)) != NULL) {
        char file_or_dir_path[MAXPATHLEN];
        struct stat stat_info;
        sprintf(file_or_dir_path, "%s/%s", dirname, dp->d_name);

        if (stat(file_or_dir_path, &stat_info) != 0) {
            perror(file_or_dir_path);
            continue; 
        }

        if (dp->d_name[0] == '.' && !a_flag) continue;

        if (S_ISDIR(stat_info.st_mode)) {
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
                continue; // skip over the . and .. directories (current and parent directory)
            }
            if (r_flag) {
                struct Directory sub_dir = createSubDir(file_or_dir_path);
                Directory *added_sub_dir = addSubDir(parent, sub_dir);
                scan_directory(file_or_dir_path, added_sub_dir); 
                if (!added_sub_dir->isEmpty) parent->isEmpty = false; 
            }
        } 
        else if (S_ISREG(stat_info.st_mode)) {
            bool only = false;
            bool ignore = false;

            // Check if the file matches any -o or -i patterns
            if (o_flag) {
                for (int p = 0; p < nPatterns; p++) {
                    struct regex_val reg = regex_values[p];
                    regex_t regex = get_comped_regex(reg);
                    if (reg.type == 'o' && regexec(&regex, dp->d_name, 0, NULL, 0) == 0) {
                        if (verbose) printf("Recognised file with FILENAME (-o): %s\n", dp->d_name);
                        only = true;  
                        break;  
                    }
                }
            }
            if (i_flag) {
                for (int p = 0; p < nPatterns; p++) {
                    struct regex_val reg = regex_values[p];
                    regex_t regex = get_comped_regex(reg);
                    if (reg.type == 'i' && regexec(&regex, dp->d_name, 0, NULL, 0) == 0) {
                        if (verbose) printf("Recognised file with FILENAME (-i): %s\n", dp->d_name);
                        ignore = true;  
                        break;  
                    }
                }
            }

            if (nPatterns > 0) {
                if (i_flag && ignore) continue;
                if (o_flag && !only) continue;
            }

            File new_file = createFile(dp->d_name, file_or_dir_path, stat_info.st_mtime, stat_info.st_mode);
            addFile(parent, new_file);
            parent->isEmpty = false;

        } 
    }

    closedir(dirp);
}

