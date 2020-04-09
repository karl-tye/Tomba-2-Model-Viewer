#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

char* slash = "/";

bool EndsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

bool StartsWith(const char *a, const char *b)
{
    if (strncmp(a, b, strlen(b)) == 0)
        return 1;
    return 0;
}

bool ProperDirectory(struct dirent* dir)
{
    return strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0;
}

char** getSMSTList(DIR* d, short* SMSTCount)
{
    struct dirent* dir;
    int smstCount = 0;
    char** list = malloc(sizeof(char*)*40);
    while ((dir = readdir(d)) != NULL) {
        if (ProperDirectory(dir) && EndsWith(dir->d_name, "SMST")) {
            char* SMST = strdup(dir->d_name);
            list[smstCount] = SMST;
            smstCount++;
        }
    }
    char** trimmedList = malloc(sizeof(char*)*(smstCount+1));
    memcpy(trimmedList, list, sizeof(char*)*(smstCount+1));
    free(list);
    *SMSTCount = smstCount;
    closedir(d);
    return trimmedList;
}

char** getSMSTFromChunk(DIR* d, char* dir_name)
{
    char* folder_location; char* ting;
    char* d_name;
    char** SMSTList;
    struct dirent* dir;
    DIR* folder;
    bool sdatsVisited;
    sdatsVisited = false;
    while ((dir = readdir(d)) != NULL) {
        folder_location = strdup(dir_name);
        ting = malloc(sizeof(char)*(strlen(folder_location)+13));
        strcpy(ting, folder_location);
        d_name = strdup(dir->d_name);
        if (ProperDirectory(dir) && !EndsWith(d_name, "vrams")) {
            strcat(ting, slash); strcat(ting, d_name);
            folder = opendir(ting);
            if (folder && !sdatsVisited) {
//                SMSTList = getSMSTList(folder);
                printf("%s\n", ting);
            }
            else if (!folder) {
                printf("Error opening directory: %s\n", ting);
                exit(-2);
            }
            if (EndsWith(ting, "sdats"))
                sdatsVisited = true;
        }
        closedir(folder);
    }
    return SMSTList;
}

char*** getSMSTFromDirectory(char* dir_name)
{
    char* new_dir;
    DIR* d;
    DIR* chunkDir;
    struct dirent* dir;
    int chunkCount = 0;
    char*** chunks = malloc(sizeof(char**)*41);
    d = opendir(dir_name);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_DIR && StartsWith(dir->d_name, "chunk")) {
                new_dir = strdup(dir_name);
                strcat(new_dir, slash);
                strcat(new_dir, dir->d_name);
                chunkDir = opendir(new_dir);
                if (chunkDir) {
                    chunks[chunkCount] = getSMSTFromChunk(chunkDir, new_dir);
                    chunkCount++;
                }
                free(new_dir);
            }
        }
    } else {
        printf("Error opening directory!\n");
        exit(-2);
    }
    closedir(d);
    return chunks;
}

int getSMSTFolders(DIR* d)
{
    int folderCount = 0;
    struct dirent* dir;
    while ((dir = readdir(d)) != NULL) {
        if (ProperDirectory(dir) && !EndsWith(dir->d_name, "vrams")) {
            folderCount++;
        }
    }
    return folderCount;
}

char*** getAllSMSTS(short* SMSTCount)
{
    char*** chunks = malloc(sizeof(char**)*41);
    int folderCount;
    char* dir_name = malloc(sizeof(char)*50);
    DIR* d;
    for (int i=0; i<41; i++) {
        folderCount = 0;
        sprintf(dir_name, "Extracted/retail-us/chunk_%02d", i);
        d = opendir(dir_name);
        if (d) {
            DIR* folder; char folderName[50];
            folderCount = getSMSTFolders(d);
            closedir(d);
            if (folderCount == 1) {
                sprintf(folderName, "Extracted/retail-us/chunk_%02d/%02d_trail", i, i);
                folder = opendir(folderName);
                chunks[i] = getSMSTList(folder, &SMSTCount[i]);
            } else if (folderCount == 2) {
                sprintf(folderName, "Extracted/retail-us/chunk_%02d/%02d_sdats", i, i);
                folder = opendir(folderName);
                chunks[i] = getSMSTList(folder, &SMSTCount[i]);
            } else {
                printf("Error opening SMST folders!\n");
                exit(-3);
            }
        } else {
            printf("Error opening directory %s!\n", dir_name);
            exit(-2);
        }
    }

    return chunks;
}