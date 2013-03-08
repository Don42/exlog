/*----------------------------------------------------------------------------
* "THE SCOTCH-WARE LICENSE" (Revision 42):
* <DonMarco42@gmail.com> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a scotch whisky in return
* Marco 'don' Kaulea
* ----------------------------------------------------------------------------*/

#define _GNU_SOURCE

#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include "entry.h"
#include "res.h"

int getIDFromFile (FILE*);
time_t getTimeFromFile (const char*);
char* getTimezoneFromFile (FILE*);
char* getLocationFromFile (FILE*);
char* getprojectFromFile (FILE*);
char* getContentFromFile (FILE*);
char* getFileName (const char* storageFolder, const struct LogEntry*);

struct LogEntry*
getEntryFromFile (const char* fileName)
{
    FILE* fp = fopen (fileName, "r");
    if (fp == NULL)
    {
        fprintf (stderr, "Error opening file. %s\n", strerror (errno));
        return NULL;
    }
    struct LogEntry* entry = malloc (sizeof (struct LogEntry));
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    if (fp == NULL){ return NULL;}

    entry->time = getTimeFromFile (fileName);

    if ((read = getline (&line, &len, fp)) != -1)
    {
        int id = -1;
        int ret = sscanf (line, REPORT_ID, &id);
        if (ret != 1) {fprintf (stderr, "ID not found"); return NULL;}
        entry->id = id;
    }

    if ((read = getline (&line, &len, fp)) != -1 &&
            strlen (line) > 10 &&
            (strncmp ("#TimeZone:", line, 10) == 0))
    {
        size_t dataLength = strlen (line) - 10;
        char* tz = malloc (dataLength + 1);
        strncpy (tz, line + 10, dataLength - 1);
        tz[dataLength - 1] = '\0';
        entry->timezone = tz;
    }else
    {
        entry->timezone = malloc (1);
        strncpy (entry->timezone, "", strlen(""));
    }

    if ((read = getline (&line, &len, fp)) != -1 &&
            strlen (line) > 11 &&
            (strncmp ("#Location:", line, 10) == 0))
    {
        size_t dataLength = strlen (line) - 10;
        char* location = malloc (dataLength + 1);
        strncpy (location, line + 10, dataLength -1);
        location[dataLength -1] = '\0';
        entry->location = location;
    }else
    {
        entry->location = malloc (10);
        strncpy (entry->location, "somewhere", 10);
    }

    if (((read = getline (&line, &len, fp)) != -1) &&
            strlen (line) > 9  &&
            (strncmp ("#Project:", line, 9) == 0))
    {
        size_t dataLength = strlen (line) - 9;
        char* project = malloc (dataLength + 1);
        strncpy (project, line + 9, dataLength -1);
        project[dataLength -1] = '\0';
        entry->project = project;
    }else
    {
        entry->project = malloc (8);
        strncpy (entry->project, "default", 8);
    }


    if ((read = getdelim (&line, &len, EOF, fp)) != -1)
    {
        entry->content = malloc (len + 1);
        strncpy (entry->content, line, len);
    }else
    {
        entry->content = malloc (23);
        strncpy (entry->content, "This is an empty entry\0", 23);
    }

    free (line);
    fclose (fp);
    return entry;
}

void
freeEntry (struct LogEntry* entry)
{
    free (entry->timezone);
    free (entry->location);
    free (entry->project);
    free (entry->content);
    free (entry);
}


time_t
getTimeFromFile (const char* fileName)
{
    struct tm* cal;
    time_t timestamp = -1;
    int i = -1;
    sscanf (strrchr (fileName, '/') + 1, "%d", &i);
    if (i != -1)
    {
        timestamp = i;
        return timestamp;
    } else
    {
        return 0;
    }
}

void
printEntry (const struct LogEntry* entry)
{
    printf ("Entry %d at %d in timezone %s\n",
            entry->id,
            entry->time,
            entry->timezone);
    if (entry->project == NULL)
    {
        printf ("Entry for project default ");
    }else
    {
        printf ("Entry for project %s ", entry->project);
    }
    if (entry->location == NULL)
    {
        printf ("at location default\n");
    }else
    {
        printf ("at location %s\n", entry->location);
    }
    printf ("Content: %s\n", entry->content);
}

int
writeEntryToFile (const char* storageFolder, struct LogEntry* entry)
{
    char* fileName = getFileName (storageFolder, entry);
    FILE* fp = fopen (fileName, "w");
    if (fp == NULL)
    {
        fprintf (stderr, "Could not open file %s for writing.\n%s\n",
                fileName, strerror (errno));
        free (fileName);
        return -1;
    }
    fprintf (fp, REPORT_ID, entry->id);
    fprintf (fp, REPORT_TZ, entry->timezone);

    if (entry->location == NULL)
    {
        fprintf (fp, REPORT_LOCATION, "");
    }else
    {
        fprintf (fp, REPORT_LOCATION, entry->location);
    }
    if (entry->project == NULL)
    {
        fprintf (fp, REPORT_PROJECT, "");
    }else
    {
        fprintf (fp, REPORT_PROJECT, entry->project);
    }
    fprintf (fp, "%s", entry->content);

    fclose (fp);
    free (fileName);
    return 0;
}

char*
getFileName(const char* storageFolder, const struct LogEntry* entry)
{
    const char *fileNameFormat = "%s/%010d";
    const char* timeFormat = "%FT%T%z";

    time_t t = entry->time;

    size_t sizeNeeded = strlen (storageFolder) + 12;

    char* buf = malloc (sizeNeeded);
    snprintf (buf, sizeNeeded, fileNameFormat, storageFolder, t);
    return buf;
}
