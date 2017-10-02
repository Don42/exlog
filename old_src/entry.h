/*----------------------------------------------------------------------------
* "THE SCOTCH-WARE LICENSE" (Revision 42):
* <don@0xbeef.org> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a scotch whisky in return
* Marco 'don' Kaulea
* ----------------------------------------------------------------------------*/

#define _GNU_SOURCE

#include <stdlib.h>
#include <time.h>

struct LogEntry
{
    int id;
    time_t time;
    char* timezone;
    char* location;
    char* project;
    char* content;
};


struct LogEntry* getEntryFromFile (const char*);
int writeEntryToFile (const char* gStorageFolder, struct LogEntry*);
void freeEntry (struct LogEntry*);
void printEntry (const struct LogEntry*);
