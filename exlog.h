/*----------------------------------------------------------------------------
* "THE SCOTCH-WARE LICENSE" (Revision 42):
* <DonMarco42@gmail.com> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you thin
* this stuff is worth it, you can buy me a scotch whisky in return
* Marco 'don' Kaulea
* ----------------------------------------------------------------------------*/

#define DEFAULT_FOLDER "%s/.exlog"
#define FILENAME_LENGTH 21 // 19 for Date, 1 for preceding slash, 1 \0

void usage (void);
void setup (void);

void list (void);
int add (void);
void rm (void);
