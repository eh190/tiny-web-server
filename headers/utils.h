#ifndef __UTILS_H__
#define __UTILS_H__
#include "rio.h"

/* $begin get_filetype */
void get_filetype(char *filename, char *filetype);
/* $end get_filetype */

/* $begin serve_static */
void serve_static(int fd, char *filename, int filesize);
/* $end serve_static */

/* $begin serve_dynamic */
void serve_dynamic(int fd, char *filename, char *cgiargs);
/* $end serve_dynamic */

/* $begin parse_uri */
int parse_uri(char *uri, char *filename, char *cgiargs);
/* $end parse_uri */

/* $begin read_requesthdrs */
void read_requesthdrs(rio_t *rp);
/* $end read_requesthdrs */

/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
/* $end clienterror */

/* $begin doit */
void doit(int fd);
/* $end doit */
#endif /* __UTILS_H__ */
