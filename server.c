#include"server.h"

int main(int argc, char *argv[]) {
    /* initialise a socket for ipv4 TCP */
    int socket_num = socket(AF_INET, SOCK_STREAM, 0);

    int connection;
    int count = 5;
    int portno;
    unsigned int len;

    if (argc >= 2)
	portno = atoi(argv[1]);
    else
	portno = PORT;
    /* create addr struct for PORT */
    struct sockaddr_in serv_addr;
    struct sockaddr_in *conn_addr;
    
        
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    

    if (bind(socket_num, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	perror("bind failed on port\n");
	abort();
    }
    do {
	if (listen(socket_num, LISTEN_QUEUE) < 0) {
	    perror("listen faild of socket\n");
	    close(socket_num);
	    abort();
	}

	conn_addr = calloc(1, sizeof(*conn_addr));
	len = sizeof(*conn_addr);
    
	connection = accept(socket_num, (struct sockaddr *)conn_addr, &len);
	if (connection < 0) {
	    perror("failed to accept\n");
	    //close(socket_num);
	}
	DEBUG_PRINT("conn len: %d\n", len);
	DEBUG_PRINT("port: %d; addr: %d;\n", conn_addr->sin_port, conn_addr->sin_addr.s_addr);
	serve(connection);
	free(conn_addr);
    }while(1);


    close(socket_num);
    
}
void zero_buff(char *buf, int buf_size){
    memset(buf, 0, buf_size * sizeof(char));
}

void serve(int socket_con) {
    FILE *fptr;
    char *filename;
    char message[] = "hello!";
    char *header;
    void *file;
    ssize_t data_written;
    ssize_t data_to_write;
    ssize_t data_ptr = 0;
    
    
    struct response_header *rh;

    zero_buff(BUFF, BUFF_SIZE);
    
    if (read(socket_con, BUFF, BUFF_SIZE) < 0){
	perror("failed to receive");
	abort();
    }
    
    DEBUG_PRINT("\nrequest: %s\n", BUFF);
    
    filename = path(BUFF);
    DEBUG_PRINT("file: %s\n", filename);
    fptr = fopen(filename, "r");

    rh = rh_generate_header(filename, fptr);
    header = create_header(rh);

    file = read_file(rh->associated_fptr, rh->content_length);
    assert(file != NULL);

    DEBUG_PRINT("header:\n%s\n", header);
    if (rh->CT_code == CT_HTML){
    	/* printf("body:\n%s\n", file); */
	/* printf("assumed size: %d, real size: %ld\n", rh->content_length, strlen(file)); */
	/* assert(strlen(file) == rh->content_length); */
    }
    write(socket_con, header, strlen(header));
    data_written =  write(socket_con, file, rh->content_length);
    
    DEBUG_PRINT("data written: %ld\n", data_written);
    data_to_write = rh->content_length;
    /* keep writing data until finished */
    while(data_written < data_to_write){
	data_to_write -= data_written;
	data_ptr += data_written;
	data_written = write(socket_con, file + data_ptr, data_to_write);
    }
    
    free(filename);
    free(header);
    free(file);
    rh_delete(rh);

    close(socket_con);
}

/* returns date time string */
char *get_date(){
    time_t raw_time;
    struct tm * timeinfo;
    time(&raw_time);
    timeinfo = localtime(&raw_time);
    strftime(DATE, SMALL_BUFF_SIZE, "%a, %d %b %Y %T %Z", timeinfo);
    return DATE;
}

/* creates header */
char *create_header(struct response_header *rh){
    char *header = calloc(BUFF_SIZE, sizeof(*header));
    snprintf(header, BUFF_SIZE, HEADER_FMT,
	     rh->response_code, rh->date, rh->content_type, rh->content_length);
    return header;
}

void *read_file(FILE *fptr, int size){
    if (fptr == NULL){
    	perror("file wrong\n");
	abort();
    }
    
    char *mem = calloc((size + 1), sizeof(*mem)); /* TODO: change to malloc */
    fread(mem, size, 1, fptr);
    return mem;
}


char *path(char * request){
    strtok(request, " ");
    char *path = strtok(NULL, " ") + 1;
    DEBUG_PRINT("path?: %s\n", path);
    /* make a copy so it doesn't get overwrittent when buff is cleared */
    return strcpy_m(path);
}

struct response_header *
rh_generate_header (char *filename, FILE *fptr){

    CT file_type;
    int resp_code;
    unsigned int filesize;
    struct response_header *rh;
    struct stat st;
    
    if(fptr == NULL){ 		/* no file found, 404 */
	file_type = CT_HTML;
	resp_code = 404;
	fptr = fopen(NOTFOUND_404, "r");
	filename = NOTFOUND_404;
    }
    else{
	file_type = get_file_type(filename);
	resp_code = 200;
    }
    stat(filename, &st);
    filesize = st.st_size;	/* filesize */

    /* creates rh */
    rh = rh_create(resp_code, (char *)CONTENT_TYPE[file_type], filesize);
    rh->CT_code = file_type;
    rh->associated_fptr = fptr;
    rh->date = get_date();
    
    return rh;
}

/* going to assume no files have other '.' in the filename */
/* going to be cheap here */
CT get_file_type(const char *filename){
    char *ext;
    CT extension = CT_UNKNOWN;
    /* copy since strtok will mangle the filename otherwise */
    char *fname = strcpy_m(filename);
    strtok(fname, ".");
    ext = strtok(NULL, ".");
    /* switch based off first letter, only use strcmp on mp3/mp4 */
    switch (*ext){
    case 'h':
	extension = CT_HTML;
	break;
    case 'j':
	extension = CT_JPEG;
	break;
    case 'm':
	DEBUG_PRINT("extension: %s\n", ext);
	if (strcmp(ext, "mp3") == 0)
	    extension = CT_MP3;
	else
	    extension = CT_MP4;
	break;
    default:
	extension = CT_UNKNOWN;
	break;
    }
    free(fname);
    return extension;
}

struct response_header *
rh_create(int resp_code, char *c_type, int c_length){
    struct response_header *rh = malloc(sizeof(*rh));
    rh->response_code = resp_code;
    rh->content_type = c_type;
    rh->content_length = c_length;
    rh->date = get_date();
    return rh;
}

void
rh_delete(struct response_header *rh){
    fclose(rh->associated_fptr);
    free (rh);
}

char *
strcpy_m(const char *src){
    size_t len = strlen(src);
    char *cpy = calloc(len, sizeof(*cpy));
    strncpy(cpy, src, len);
    return cpy;
}

