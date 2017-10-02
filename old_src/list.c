/* ----------------------------------------------------------------------------
 * "THE SCOTCH-WARE LICENSE" (Revision 42):
 * <don@0xbeef.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a scotch whisky in return
 * Marco 'don' Kaulea
 * ----------------------------------------------------------------------------*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>

#include "list.h"
#include "filter.h"
#include "res.h"
#include "entry.h"

int
list(const char* storageFolder)
{
    printf ("List operation\n");
    struct dirent **nameList;
    size_t numberFiles;
    struct LogEntry **entries;
    char* fileNameBuffer = malloc (strlen (storageFolder) + 12);
    int i = 0;

    numberFiles = scandir (storageFolder, &nameList, filterFiles, alphasort);
    entries = malloc (numberFiles * sizeof (struct LogEntry));
    for (i = 0; i < numberFiles; i++)
    {
        snprintf (fileNameBuffer, strlen (storageFolder) + 12, "%s/%s",
                    storageFolder, nameList[i]->d_name);
        entries[i] = getEntryFromFile (fileNameBuffer);
        free (nameList[i]);
        if (entries[i] != NULL)
        {
            printEntry (entries[i]);
        }
    }

    for (i = 0; i < numberFiles; i++)
    {
        freeEntry (entries[i]);
    }

    free (fileNameBuffer);
    free (entries);
    free (nameList);
    return 0;
}
