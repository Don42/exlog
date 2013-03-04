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
#include <string.h>

#include "entry.h"
#include "res.h"

int getIDFromFile (FILE*);
time_t getTimeFromFile (const char*);
char* getTimezoneFromFile (FILE*);
char* getLocationFromFile (FILE*);
char* getprojectFromFile (FILE*);
char* getContentFromFile (FILE*);


struct LogEntry*
getEntryFromFile (const char* fileName)
{
    struct LogEntry* entry = malloc (sizeof (struct LogEntry));
    FILE* fp = fopen (fileName, "r");
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


    char* content = malloc (10);
    strncpy (content, "test", 4);
    while ((read = getline (&line, &len, fp)) != -1)
    {

    }
    entry->content = content;

    free (line);
    fclose (fp);
    return entry;
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

int
getIDFromFile (FILE* fp)
{
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
