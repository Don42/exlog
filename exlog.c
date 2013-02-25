/*----------------------------------------------------------------------------
* "THE SCOTCH-WARE LICENSE" (Revision 42):
* <DonMarco42@gmail.com> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you thin
* this stuff is worth it, you can buy me a scotch whisky in return
* Marco 'don' Kaulea
* ----------------------------------------------------------------------------*/

#define _GNU_SOURCE

#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <regex.h>

#include "exlog.h"

size_t  copyTemplate (size_t ,char*, char*);
char* getFileName ();
size_t hasContent (char*);
int getNextID ();
int getFileID (char*);

int filterFiles (const struct dirent *);

void setup (void);
void teardown (void);


char* gStorageFolder;



int
main (int argc, char *argv[])
{
    size_t ch, cmd;
    const char *commands[]={"add", "rm", "list", NULL};
    enum {ADD,RM,LIST};

    while ((ch = getopt(argc, argv, "h")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            default:
                usage();
                exit(EXIT_FAILURE);
         }
    }

    setup ();
    if (argc <= 1)
    {
        list ();
        exit (EXIT_SUCCESS);
    }

    for (cmd=0;commands[cmd] && strcasecmp(argv[1],commands[cmd]);cmd++);
    switch (cmd)
    {
        case ADD:
            return add ();
            break;
        case RM:
            rm ();
            break;
        case LIST:
            list ();
            break;
        default:
            fprintf (stderr, "Unknown command...");
            exit (EXIT_FAILURE);
            break;
    }
    teardown ();
    return 0;
}

void
usage (void)
{
    fprintf (stderr, "usage: exlog [command] [-h]\n");
}

void
setup (void)
{
    struct stat sts;
    char *home = getenv ("HOME");
    if (home != NULL)
    {
        size_t sizeNeeded = snprintf (NULL, 0, DEFAULT_FOLDER, home) + 1;
        char *path = malloc (sizeNeeded);
        snprintf (path, sizeNeeded, DEFAULT_FOLDER, home);
        size_t ret = stat(path, &sts);
        if ((ret == -1) || &sts != NULL && !S_ISDIR (sts.st_mode))
        {
            printf ("Storage folder not found.\n"
                    "Creating folder \"%s\" now...\n", path);
            if (mkdir (path, 0700) != 0)
            {
                fprintf (stderr, "Failed to create storage folder.\n"
                            "ERROR: %s\n", strerror(errno));
            }
        }
        gStorageFolder = malloc (strlen (path));
        strncpy (gStorageFolder, path, strlen (path));
        free (path);

    }else
    {
        fprintf (stderr, "HOME not set");
        exit (1);
    }
}

void
teardown (void)
{
    free (gStorageFolder);
}

int
list(void)
{
    printf ("List operation");
    struct dirent **nameList;
    size_t numberFiles;

    numberFiles = scandir (gStorageFolder, &nameList, filterFiles, alphasort);
    printf ("%d Logentries found\n", numberFiles);
    return 0;
}

int
add (void)
{
    printf ("Add operation\n");
    size_t mode = S_IRUSR | S_IWUSR;
    char* buffer = "This is a Testentry\n";

    char* fileName = getFileName ();

    int id = getNextID ();
    if (id < 0)
    {
        exit (25);
    }
    copyTemplate(id, NULL, NULL);
    char* command = malloc (strlen (gStorageFolder) + 21);
    snprintf (command,strlen (gStorageFolder) + 21,
            "$EDITOR %s/REPORT_BASE", gStorageFolder);
    size_t ret = system (command);
    snprintf (command, strlen(gStorageFolder) + 21,
            "%s/REPORT_BASE\0", gStorageFolder);
    if (ret == 0 && hasContent(command))
    {
        if (execl ("/bin/mv", "/bin/mv", command, fileName, (char *)0) != 0)
        {
            fprintf (stderr, "%s\n", strerror (errno));
            exit (10);
        }
    }else
    {
        if (execl ("/bin/rm", "/bin/rm", command, (char *)0) != 0)
        {
            fprintf (stderr, "%s\n", strerror (errno));
            exit (11);
        }
    }
    free (command);
    free (fileName);
    return 0;
}

int
rm (void)
{
    printf ("Rm operation");
    return 0;
}

char*
getFileName()
{
    const char *fileNameFormat = "%s/%d-%02d-%02dT%02d:%02d:%02d";

    time_t t = time(NULL);
    struct tm te = *localtime(&t);

    size_t sizeNeeded = strlen (gStorageFolder) + FILENAME_LENGTH;
    char* buf = malloc (sizeNeeded);
    snprintf (buf, sizeNeeded, fileNameFormat, gStorageFolder,
            te.tm_year + 1900, te.tm_mon + 1, te.tm_mday,
            te.tm_hour, te.tm_min, te.tm_sec);

    return buf;
}

size_t
hasContent (char* fileName)
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
copyTemplate (size_t id, char* location, char* project)
{
    char* fileName = malloc (strlen (gStorageFolder) + 13);
    snprintf (fileName, strlen (gStorageFolder) + 13, "%s/REPORT_BASE",
            gStorageFolder);
    FILE* fp = fopen (fileName, "w");
    if (fp == NULL)
    {
        fprintf (stderr, "Could not open template file: %s", strerror (errno));
        exit (1);
    }
    fprintf (fp, REPORT_ID, id);
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
getNextID ()
{
    struct dirent **nameList;
    size_t numberFiles;
    int id = -1;
    int i = 0;

    numberFiles = scandir (gStorageFolder, &nameList, filterFiles, alphasort);
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
        char* fileName = malloc ( strlen (gStorageFolder) + strlen (
                    nameList[numberFiles - 1]->d_name) + 2);
        snprintf (fileName, strlen (gStorageFolder) + strlen (
                    nameList[numberFiles - 1]->d_name) + 2,
                    "%s/%s", gStorageFolder, nameList[numberFiles - 1]->d_name);
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
getFileID (char* fileName)
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

int
filterFiles (const struct dirent* de)
{
    regex_t regex;
    int ret;
    char* fileName = malloc (strlen (de->d_name) + 1);

    strncpy (fileName, de->d_name, strlen (de->d_name) + 1);
    if (strlen (fileName) != 19)
    {
        return 0;
    } else
    {
        ret = regcomp (&regex,
                "[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}",
                REG_NOSUB | REG_EXTENDED);
        if (ret != 0)
        {
            fprintf (stderr, "Could not compile regex\n");
            exit (4);
        }

        ret = regexec (&regex, fileName, 0, NULL, 0);
        regfree (&regex);
        free (fileName);
        if (ret == 0)
        {
            return 1;
        }else if (ret == REG_NOMATCH)
        {
            return 0;
        }else
        {
            fprintf (stderr, "Error executing regex\n");
            exit (4);
        }
    }
}
