#include "mysync.h"
#include  <sys/stat.h>
#include  <dirent.h>
#include <string.h>
#include <utime.h>


// Check if a file exists in a given directory
// if a file does exist, return a pointer to it
File* check_if_file_exists_in_dir(char *filename, Directory *dir){
    for (int i = 0; i < dir->nFiles; i++){
        if (strcmp(dir->files[i].filename, filename) == 0){
            return &dir->files[i];
        }
    }
    return NULL;
}

// Copy over a file's contents from a source to a destination
void copy_files(char *destination, char *source, time_t source_mod_time, mode_t perm)
{
    FILE *fp_in = fopen(source, "rb");
    FILE *fp_out = fopen(destination, "wb");

    // ENSURE THAT OPENING BOTH FILES HAS BEEN SUCCESSFUL
    if(fp_in != NULL && fp_out != NULL) {
        char buffer[BUFSIZ];
        size_t got, wrote;
        while( (got = fread(buffer, 1, sizeof buffer, fp_in)) > 0) {
            wrote = fwrite(buffer, 1, got, fp_out);
            if(wrote != got) {
                perror("Error when copying files\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    // ENSURE THAT WE ONLY CLOSE FILES THAT ARE OPEN
    if(fp_in != NULL) {
        fclose(fp_in);
    }
    if(fp_out != NULL) {
        fclose(fp_out);
    }

    // Update the modification time of the newly updated file to the modification time of the up-to-date file
    if (p_flag) {
        printf("For |%s| modification time -> %ld\n", destination, source_mod_time);
        struct utimbuf new_time;
        new_time.modtime = source_mod_time;

        if (utime(destination, &new_time) != 0) {
            perror("utime");
            exit(EXIT_FAILURE);
        }

        if (chmod(destination, perm) != 0) {
            perror("chmod");
            exit(EXIT_FAILURE);
        }
    }
}

// Sync a directory one way from source -> target
void sync_to_target(Directory *source, Directory *target) {
    // Traverse the source directory
    for (int i = 0; i < source->nFiles; i++) {
        File *current_file = &source->files[i];
        char *filename = current_file->filename;
        
        mode_t perm = 666;
        if (p_flag) perm = current_file->permissions;

        File* target_file;
        if ((target_file = check_if_file_exists_in_dir(filename, target)) != NULL) {
            if (current_file->modification_time > target_file->modification_time) {
                // If a file does exist and the source file is the more recent file, then update the target file
                if (verbose) printf("Copying file %s from Dir: %s -> %s\n", target_file->filename, source->name, target->name);
                target_file->modification_time = current_file->modification_time;
                if (!n_flag) copy_files(target_file->file_path, current_file->file_path, current_file->modification_time, perm);
            }
        } 
        else {
            // If a file doesn't exist, make the file and copy over the contents
            char path[MAXPATHLEN];
            sprintf(path, "%s/%s", target->name, current_file->filename);

            addFile(target, createFile(current_file->filename, path, current_file->modification_time, perm));
            target->isEmpty = false;
            if (verbose) printf("MADE file %s in %s\n", current_file->filename, target->name);
            if (!n_flag) copy_files(path, current_file->file_path, current_file->modification_time, perm);
        }
    }

    // Traverse the sub-directories and sync them recursively
    if (r_flag){
        for (int i = 0; i < source->nSubDirs; i++) {
            if (source->sub_directories[i].isEmpty) continue;
            char *source_dir_name = strrchr(source->sub_directories[i].name, '/') +1;
            char *target_dir_name = NULL;
            bool dir_found = false;

            for (int j = 0; j < target->nSubDirs; j++) {
                target_dir_name = strrchr(target->sub_directories[j].name, '/') +1;
                if (strcmp(source_dir_name, target_dir_name) == 0) {
                    dir_found = true;
                    sync_to_target(&source->sub_directories[i], &target->sub_directories[j]);
                }
            }
            if (!dir_found){ // If a directory doesn't exist, create it in both the file-system and the add it to the tree
                char path[MAXPATHLEN];
                sprintf(path, "%s/%s", target->name, source_dir_name);
                Directory *new_dir = addSubDir(target, createSubDir(path));
                if (verbose) printf("MADE a dir %s in %s\n", source_dir_name, target->name);
                if (!n_flag) {
                    if (mkdir(path, 0777) != 0){ 
                        perror("An error occured while creating a directory\n");
                        exit(EXIT_FAILURE);
                    }
                } 
                sync_to_target(&source->sub_directories[i], new_dir);
                if (!new_dir->isEmpty) target->isEmpty = false;
            }
        }
    }
}

// Function to perform two-way synchronization between directories
void two_way_sync(Directory *dir1, Directory *dir2) {

    sync_to_target(dir1, dir2);

    sync_to_target(dir2, dir1); 
}

void sync_files() {
    Directory *reference = &Directories[0]; // This reference directory will contain the most up-to-date directory (union of others)
    
    // Loop through argument directories twice.
    // Once to fully 'unionise' our reference directory, then once again to sync all other files according to the new reference directory
    for (int i = 0; i < 2; i++) {
        if (i == 0 && verbose) printf("Syncing all files to the reference: |%s|\n\n", reference->name);
        else if (i == 1 && verbose) printf("\nSyncing all other files according to the reference: |%s|\n\n", reference->name);
        for (int j = 1; j < nDirectories; j++){
            two_way_sync(reference, &Directories[j]);
        }
    }
}































