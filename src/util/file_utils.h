#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#define PATHLEN 32

char *proc_parser(char *file, char *field);

char *construct_path(int pathparts, ...);

#endif
