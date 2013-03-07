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
char* readFromPipe (FILE*);
char* getTimezone ();

int
add (const char* storageFolder, char* location, char* project)
{
    printf ("Add operation\n");
    tzset ();
    size_t mode = S_IRUSR | S_IWUSR;
    FILE* fp;
    char* fileName = getFileName (storageFolder);
    int id = getNextID (storageFolder);
    char* contentBuffer = NULL;

    if (id < 0)
    {
        return (-25);
    }


    fp = popen ("/usr/bin/vipe </dev/null", "r");
    if (fp == NULL)
    {
        printf ("Failed to run command\n");
        return -1;
    }

    contentBuffer = readFromPipe (fp);
    if (contentBuffer != NULL && strlen (contentBuffer) < 0)
    {
        struct LogEntry* entry = malloc (sizeof (struct LogEntry));
        entry->time = time(NULL);

        entry->id = id;
        entry->timezone = getTimezone ();
        entry->location = location;
        entry->project = project;
        entry->content = contentBuffer;
        printf ("%s", contentBuffer);

        freeEntry (entry);
    }

    fclose (fp);
    free (fileName);
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
        buffer_offset += block_size;
    }

    if (chars_io < 0)
    {
        perror ("Read error");
        return "";
    }
    return read_buffer;
}

char*
getFileName(const char* storageFolder)
{
    const char *fileNameFormat = "%s/%010d";
    const char* timeFormat = "%FT%T%z";

    time_t t = time (NULL);

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
            free (line);
            return 1;
        }
    }
    fclose (fp);
    free (line);
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
    if (location == NULL)
    {
        location = "";
    }
    fprintf (fp, REPORT_LOCATION, location);
    if (project == NULL)
    {
        project = "";
    }
    fprintf (fp, REPORT_PROJECT, project);

    fclose (fp);
    free (fileName);
    return 0;
}

char*
getTimezone ()
{
    char* tz;
    tz = getenv ("TZ");

    if (tz == NULL)
    {
        tz = malloc (4);
        strncpy (tz, "UTC\0", 4);
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
