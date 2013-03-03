/* ----------------------------------------------------------------------------
 * "THE SCOTCH-WARE LICENSE" (Revision 42):
 * <DonMarco42@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a scotch whisky in return
 * Marco 'don' Kaulea
 * ----------------------------------------------------------------------------*/

#include <dirent.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


int
filterFiles (const struct dirent* de)
{
    regex_t regex;
    int ret;
    char* fileName = malloc (strlen (de->d_name) + 1);
    strncpy (fileName, de->d_name, strlen (de->d_name) + 1);
    if (strlen (fileName) != 10)
        //This depends on the regex used! Change accordingly
    {
        free(fileName);
        return 0;
    } else
    {
        ret = regcomp (&regex,
                "[0-9]{10}",
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
