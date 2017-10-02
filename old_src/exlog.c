/*----------------------------------------------------------------------------
* "THE SCOTCH-WARE LICENSE" (Revision 42):
* <don@0xbeef.org> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a scotch whisky in return
* Marco 'don' Kaulea
* ----------------------------------------------------------------------------*/

#define _GNU_SOURCE

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "exlog.h"
#include "res.h"
#include "add.h"
#include "rm.h"
#include "list.h"

void usage (void);
void setup (void);
void teardown (void);

void callAdd (int, char*[]);
void callRm (int, char*[]);

char* gStorageFolder;



int
main (int argc, char *argv[])
{
    size_t cmd;
    const char *commands[]={"add", "rm", "list", NULL};
    enum {ADD,RM,LIST};

    setup ();

    if (argc <= 1)
    {
        list (gStorageFolder);
        teardown ();
        exit (EXIT_SUCCESS);
    }

    for (cmd=0;commands[cmd] && strcasecmp(argv[1],commands[cmd]);cmd++);
    switch (cmd)
    {
        case ADD:
            callAdd (argc, argv);
            break;
        case RM:
            callRm (argc, argv);
            break;
        case LIST:
            list (gStorageFolder);
            break;
        default:
            fprintf (stderr, "Unknown command...\n");
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
        gStorageFolder = malloc (strlen (path) + 1);
        strncpy (gStorageFolder, path, strlen (path) + 1);
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
    gStorageFolder = NULL;
}

void
callAdd (int argc, char* argv[])
{
    const struct option long_options[] =
    {
        {"help",    0,  NULL,   'h'},
        {"location",    1,  NULL,   'l'},
        {"project", 1, NULL,    'p'},
        {NULL,  0,  NULL,   0}
    };

    const char* location = NULL;
    const char* project = NULL;
    int next_option;
    while ((next_option = getopt_long (argc, argv, "h", long_options, NULL))
            != -1)
    {
        switch (next_option)
        {
            case 'h':
                usage ();
                teardown ();
                exit (EXIT_SUCCESS);
            case 'l':
                location = optarg;
                break;
            case 'p':
                project = optarg;
                break;
            default:
                usage ();
                teardown ();
                exit (EXIT_FAILURE);
         }
    }

    add (gStorageFolder, location, project);

}

void
callRm (int argc, char* argv[])
{
   uint id;
   if (argc < 2)
   {
       printf ("You need to specify what to delete.\n");
       teardown ();
       exit (EXIT_FAILURE);
   }else
   {
       if (sscanf (argv[2], "%u", &id) == 1)
       {
           rm (id);
       }
   }
}
