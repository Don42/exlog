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
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>

#include "add.h"
#include "res.h"
#include "entry.h"
#include "filter.h"

size_t  copyTemplate (size_t ,const char*, const char*, const char*);
char* getFileName (const char*);
size_t hasContent (const char*);
int getNextID(const char*);

int
add (const char* storageFolder)
{
    printf ("Add operation\n");
    tzset ();
    size_t mode = S_IRUSR | S_IWUSR;

    char* fileName = getFileName (storageFolder);

    int id = getNextID (storageFolder);
    if (id < 0)
    {
        exit (25);
    }
    copyTemplate(id, NULL, NULL, storageFolder);
    char* command = malloc (strlen (storageFolder) + 21);
    snprintf (command,strlen (storageFolder) + 21,
            "$EDITOR %s/REPORT_BASE", storageFolder);
    size_t ret = system (command);
    snprintf (command, strlen(storageFolder) + 21,
            "%s/REPORT_BASE\0", storageFolder);
    if (ret == 0 && hasContent(command))
    {
        if (execl ("/bin/mv", "/bin/mv", command, fileName, (char *)0) != 0)
        {
            fprintf (stderr, "%s\n", strerror (errno));
            exit (10);
        }
    }else
    {
        if (remove (command) != 0)
        {
            fprintf (stderr, "%s\n", strerror (errno));
            exit (11);
        }
    }
    free (command);
    free (fileName);
    return 0;
}

char*
getFileName(const char* storageFolder)
{
    const char *fileNameFormat = "%s/%010d";
    const char* timeFormat = "%FT%T%z";

    time_t t = time(NULL);

    size_t sizeNeeded = strlen (storageFolder) + 12;

    char* buf = malloc (sizeNeeded);
    snprintf (buf, sizeNeeded, fileNameFormat, storageFolder, t);
    return buf;
}

size_t
hasContent (const char* fileName)
{
    FILE* fp = fopen (fileName, "r");
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    if (fp == NULL)
        return 0;

    while ((read = getline ( &line, &len, fp)) != -1)
    {
        if (line[0] != "#"[0])
        {
            fclose (fp);
            return 1;
        }
    }
    fclose (fp);
    return 0;
}


size_t
copyTemplate (size_t id, const char* location, const char* project,
        const char* storageFolder)
{
    char* fileName = malloc (strlen (storageFolder) + 13);
    snprintf (fileName, strlen (storageFolder) + 13, "%s/REPORT_BASE",
            storageFolder);
    FILE* fp = fopen (fileName, "w");
    if (fp == NULL)
    {
        fprintf (stderr, "Could not open template file: %s", strerror (errno));
        exit (1);
    }
    fprintf (fp, REPORT_ID, id);
    char* tz;
    tz = getenv ("TZ");
    if (tz == NULL)
    {
        tz = malloc (9);
        tz = "UTC";
    }
    fprintf (fp, REPORT_TZ, tz);
    if (location != NULL)
    {
        fprintf (fp, REPORT_LOCATION, location);
    }
    if (project != NULL)
    {
        fprintf (fp, REPORT_PROJECT, project);
    }
    fclose (fp);
    return 0;
}


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
        FILE* fp = fopen (fileName, "r");
        if (fp != NULL)
        {
            id = getIDFromFile (fp) + 1;
        }else
        {
            id = 0;
        }
        free (fileName);
    }

    for (i = 0; i < numberFiles; i++)
    {
        free(nameList[i]);
    }
    free (nameList);
    return id;
}
