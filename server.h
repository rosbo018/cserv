/* Ryan Osborne 8602513 Tue Oct 29 2019 */
/* Assignment 2 part 2 */

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/ip.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<sys/stat.h>
#include<assert.h>
#include<sys/uio.h>

#define PORT 8989
#define LISTEN_QUEUE 50
#define BUFF_SIZE 512
#define SMALL_BUFF_SIZE 80
#define HTML_DOC "index.html"
#define NOTFOUND_404 "notfound.html"
#define DEBUG 1
#define DEBUG_PRINT(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)
/* taken from online ^ */
#define HEADER_FMT "HTTP/1.1 %d\nserver: test\ndate: %s\ncontent-type: %s\ncontent-length: %ld\n\n"
/* built on linux */
typedef enum content_type_selector {
    CT_HTML,
    CT_JPEG,
    CT_MP3,
    CT_MP4,
    CT_UNKNOWN,
} CT;

struct response_header {
    int response_code;
    char *content_type;
    CT CT_code;
    unsigned long content_length;
    FILE *associated_fptr;
    char *date;
};


/* accepted content types */
const char *CONTENT_TYPE[] = {"text/html", "image/jpeg", "audio/mpeg", "video/mpeg", "application/octet-stream"};


/* both of these should be changed, global vars */
/* buffer */
static char BUFF[BUFF_SIZE];

static char DATE[SMALL_BUFF_SIZE];



/* generates a header from current struct */
char *create_header(struct response_header *rh);

/* reads a file fully into memory */
void *read_file(FILE *fptr, int size);


/* gets the file type from a filename*/
CT get_file_type(const char *filename);

/* creates a response header struct from the filename */
struct response_header *
rh_generate_header (char *filename, FILE *fptr);

/* subroutine for rh_generate_header */
struct response_header *
rh_create(int resp_code, char *c_type, int c_length);

/* deletes reasponse header struct */
void rh_delete(struct response_header *rh);

/* serves a connection */
void serve(int socket_con);

/* zeros out a buffer */
void zero_buff(char *, int);

/* gets the path from a request */
char *path(char *request);

/* creates new memory location and copies a string to it */
char *strcpy_m(const char *);

