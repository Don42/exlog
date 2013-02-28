/* ----------------------------------------------------------------------------
 * "THE SCOTCH-WARE LICENSE" (Revision 42):
 * <DonMarco42@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a scotch whisky in return
 * Marco 'don' Kaulea
 * ----------------------------------------------------------------------------*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "list.h"
#include "filter.h"
#include "res.h"

int
list(const char* storageFolder)
{
    printf ("List operation\n");
    struct dirent **nameList;
    size_t numberFiles;
    int i = 0;

    numberFiles = scandir (storageFolder, &nameList, filterFiles, alphasort);

    for (; i < numberFiles; i++)
    {
        char* absPath = malloc (strlen (storageFolder)
                + strlen (nameList[i]->d_name) + 2);
        snprintf (absPath,
                    strlen (storageFolder) + strlen (nameList[i]->d_name) + 2,
                    "%s/%s", storageFolder, nameList[i]->d_name);

        printf ("Log on %s\n", nameList[i]->d_name);
        printEntry (absPath);
        free (absPath);
        free (nameList[i]);
    }

    free (nameList);
    return 0;
}


int
printEntry (const char* fileName)
{
    FILE* fp = fopen (fileName, "r");
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    if (fp == NULL)
        return -1;

    while ((read = getline (&line, &len, fp)) != -1)
    {
        size_t id = -1;
        size_t ret = sscanf (line, REPORT_ID, &id);
        printf ("%s", line);
    }
    if (line != NULL){free (line);}
    printf ("\n");
    fclose (fp);
    return 0;
}