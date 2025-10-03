/* $begin utils.c */
#include "rio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char **environ;

/* $begin get_filetype */
/* derive filetype from file name */
void get_filetype(char *filename, char *filetype) {
  if (strstr(filename, ".html")) {
    strcpy(filetype, "text/html");
  } else if (strstr(filename, ".gif")) {
    strcpy(filetype, "image/gif");
  } else if (strstr(filename, ".png")) {
    strcpy(filetype, "image/png");
  } else if (strstr(filename, ".jpg")) {
    strcpy(filetype, "image/jpeg");
  } else {
    strcpy(filetype, "text/plain");
  }
}
/*end get_filetype */

/* $begin serve_static */
/*
 * serve_static - copy a file back to the client
 */
void serve_static(int fd, char *filename, int filesize) {
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  /* send response headers to clients */
  get_filetype(filename, filetype);
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
  rio_writen(fd, buf, strlen(buf));
  printf("Response headers:\n");
  printf("%s\n", buf);

  /* Send response body to client */
  srcfd = open(filename, O_RDONLY, 0);
  /* map the requested file to a virtual memory area */
  /* maps the first 'filesize' bytes of file 'srcfd' to a private readonly area
   * of virtual memory that starts at address srcp
   * */
  srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
  /* we no longer need the src file so we close it*/
  close(srcfd);
  /* Send the data to the client - copy the file contents in the virtual memory to the client fd*/
  rio_writen(fd, srcp, filesize);
  /* frees the mapped virtual memory area */
  munmap(srcp, filesize);
}
/* $end serve_static */

/* $begin serve_dynamic */
/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
void serve_dynamic(int fd, char *filename, char *cgiargs) {
  char buf[MAXLINE], *emptylist[] = {NULL};

  /* Return first part of HTTP response */
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  rio_writen(fd, buf, strlen(buf));

  if (fork() == 0) { /* Child */
    /* Real server would set all the CGI vars here */
    setenv("QUERY_STRING", cgiargs, 1);
    /* everything cgi program writes to stdout goes directly
     * to client process without any intervention from the parent
     * process
     */
    dup2(fd, STDOUT_FILENO);              /* redirect stdout to client */
    execve(filename, emptylist, environ); /* run cgi program */
  }
  wait(NULL); /* parent waits for and reaps child */
}
/* $end serve_dynamic */

/* $begin parse_uri */
/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
int parse_uri(char *uri, char *filename, char *cgiargs) {
  char *ptr;

  if (!strstr(uri, "cgi-bin")) { /* static content */
    strcpy(cgiargs, "");
    strcpy(filename, ".");
    strcat(filename, uri);
    snprintf(filename, MAXLINE, "html%s", uri); /* Start with the base directory for static files */
    if (uri[strlen(uri) - 1] == '/') {
      strcat(filename, "home.html");
    }
    return 1;
  } else { /* dynamic content */
    /* extract cgi args from uri */
    ptr = index(uri, '?');
    if (ptr) {
      strcpy(cgiargs, ptr + 1);
      *ptr = '\0';
    } else {
      strcpy(cgiargs, "");
    }
    strcpy(filename, ".");
    strcat(filename, uri);
    return 0;
  }
}
/* $end parse_uri */

/* $begin read_requesthdrs */
/*
 * read_requesthdrs - read and parse HTTP request headers
 */
void read_requesthdrs(rio_t *rp) {
  char buf[MAXLINE];

  rio_readlineb(rp, buf, MAXLINE);
  while (strcmp(buf, "\r\n")) {
    rio_readlineb(rp, buf, MAXLINE);
  }

  return;
}
/* $end read_requesthdrs */

/* $begin clienterror */
/*
 * clienterror - returns an error message to the client
 */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
  char buf[MAXLINE], body[MAXBUF];

  /* Build the HTTP response */
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  /* Print the HTTP response */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)(strlen(body)));
  rio_writen(fd, buf, strlen(buf));
  rio_writen(fd, buf, strlen(body));
}
/* $end clienterror */

/* $begin doit */
/*
 * doit - handle one HTTP request/response transaction
 *
 */
void doit(int fd) {
  int is_static;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;

  /* Read request line and headers */
  rio_readinitb(&rio, fd);
  rio_readlineb(&rio, &buf, MAXLINE);
  printf("Request headers:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);
  if (strcasecmp(method, "GET")) {
    clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
    return;
  }
  read_requesthdrs(&rio);

  /* Parse URI from GET request */ 
  is_static = parse_uri(uri, filename, cgiargs);
  if (stat(filename, &sbuf) < 0) {
    clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
    return;
  }

  if (is_static) { /* server static content */
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
      return;
    }

    serve_static(fd, filename, sbuf.st_size);
  } else { /* serve dynamic */
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the cgi program");
      return;
    }

    serve_dynamic(fd, filename, cgiargs);
  }
}
/* $end doit */

/* $end utils.c */
