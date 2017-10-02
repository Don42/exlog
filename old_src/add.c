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

int getNextID(const char*);
char* readFromPipe (FILE*);
char* getTimezone ();

int
add (const char* storageFolder, const char* location, const char* project)
{
    printf ("Add operation\n");
    tzset ();
    size_t mode = S_IRUSR | S_IWUSR;
    FILE* fp;
    int id = getNextID (storageFolder);
    char* fLocation = NULL;
    char* fProject = NULL;
    char* contentBuffer = NULL;

    if (id < 0)
    {
        return (-25);
    }

    if (location != NULL)
    {
        fLocation = malloc (strlen (location) + 1);
        strncpy (fLocation, location, strlen (location));
    }
    if (project != NULL)
    {
        fProject = malloc (strlen (project) + 1);
        strncpy (fProject, project, strlen (project));
    }


    fp = popen ("/usr/bin/vipe </dev/null", "r");
    if (fp == NULL)
    {
        printf ("Failed to run command\n");
        return -1;
    }

    contentBuffer = readFromPipe (fp);
    if (contentBuffer != NULL && strlen (contentBuffer) > 0)
    {
        struct LogEntry* entry = malloc (sizeof (struct LogEntry));
        entry->time = time(NULL);

        entry->id = id;
        entry->timezone = getTimezone ();
        entry->location = fLocation;
        entry->project = fProject;
        entry->content = contentBuffer;

        writeEntryToFile (storageFolder, entry);
        freeEntry (entry);
    }

    fclose (fp);
    return 0;
}

char*
readFromPipe (FILE* fp)
{
    const size_t block_size = 40;
    char* read_buffer = NULL;
    size_t buffer_size = 0;
    size_t buffer_offset = 0;
    char* chars_io;

    while (1)
    {
        if (buffer_offset + block_size > buffer_size)
        {
            buffer_size = buffer_size + block_size * 2;
            read_buffer = realloc (read_buffer, buffer_size);
            if (read_buffer == NULL)
            {
                perror ("Not enough memory");
                return "";
            }
        }

        chars_io = fgets (read_buffer + buffer_offset,
                block_size,
                fp);
        if (chars_io == NULL) break;
        buffer_offset += block_size - 1;
    }

    if (chars_io < 0)
    {
        perror ("Read error");
        return "";
    }
    return read_buffer;
}


char*
getTimezone ()
{
    char* env;
    char* tz;
    env = getenv ("TZ");

    if (env == NULL)
    {
        tz = malloc (4);
        strncpy (tz, "UTC\0", 4);
    }else
    {
        tz = malloc (strlen (env) + 1);
        strncpy (tz, env, strlen (env) + 1);
    }

    return tz;
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
        struct LogEntry* entry = getEntryFromFile (fileName);
        id = entry->id + 1;
        freeEntry (entry);
        free (fileName);
    }

    for (i = 0; i < numberFiles; i++)
    {
        free(nameList[i]);
    }
    free (nameList);
    return id;
}
