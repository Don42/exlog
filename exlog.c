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


#include "exlog.h"


char* getFileName ();


char *gStorageFolder;



int
main (int argc, char *argv[])
{
    int ch, cmd;
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

    setup();
    if (argc <= 1)
    {
        list();
        exit(EXIT_SUCCESS);
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
            fprintf(stderr, "Unknown command...");
            exit(EXIT_FAILURE);
            break;
    }
    return (0);
}

void
usage (void)
{
    fprintf(stderr, "usage: exlog [command] [-h]\n");
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
        int ret = stat(path, &sts);
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
list(void)
{
    printf ("List operation");
}

int
add (void)
{
    printf ("Add operation\n");
    int mode = S_IRUSR | S_IWUSR;
    char* buffer = "This is a Testentry\n";

    char* fileName = getFileName ();
    int fd = open (fileName, O_CREAT | O_WRONLY, mode);
    if (fd != -1)
    {

        int ret = write (fd, buffer, strlen (buffer));
        close (fd);
        if(ret != -1)
        {
            return 0;
        }else
        {
            fprintf (stderr, "ERROR: %s\n", strerror(errno));
            return 2;
        }
    }
    free (fileName);
    fprintf (stderr, "ERROR: %s\n", strerror(errno));
    return 1;
}

void
rm (void)
{
    printf ("Rm operation");
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
