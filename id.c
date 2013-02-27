/* ----------------------------------------------------------------------------
 * "THE SCOTCH-WARE LICENSE" (Revision 42):
 * <DonMarco42@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a scotch whisky in return
 * Marco 'don' Kaulea
 * ----------------------------------------------------------------------------*/

#define _GNU_SOURCE

#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "id.h"
#include "res.h"
#include "filter.h"
#include "fcntl.h"

int
getNextID (const char* storageFolder)
{
    struct dirent **nameList;
    size_t numberFiles;
    int id = -1;
    int i = 0;

    numberFiles = scandir (storageFolder, &nameList, filterFiles, alphasort);
    if (numberFiles < 0)
    {
        fprintf (stderr, "Could not open storage folder: %s\n",
                strerror (errno));
        exit(2);
    }else if (numberFiles == 0)
    {
        id = 0;
    }else
    {
        char* fileName = malloc ( strlen (storageFolder) + strlen (
                    nameList[numberFiles - 1]->d_name) + 2);
        snprintf (fileName, strlen (storageFolder) + strlen (
                    nameList[numberFiles - 1]->d_name) + 2,
                    "%s/%s", storageFolder, nameList[numberFiles - 1]->d_name);
        id = getFileID (fileName) + 1;
        free (fileName);
    }

    for (i = 0; i < numberFiles; i++)
    {
        free(nameList[i]);
    }
    free (nameList);
    return id;
}


int
getFileID (const char* fileName)
{
    FILE* fp = fopen (fileName, "r");
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    if (fp == NULL)
        return -1;

    if ((read = getline (&line, &len, fp)) != -1)
    {
        size_t id = -1;
        size_t ret = sscanf (line, REPORT_ID, &id);
        fclose (fp);
        return id;
    }
    fclose (fp);
    return -1;

}
