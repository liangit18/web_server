/*
 * This is the declaration of all function that may be used.
*/

#include "sws.h"

#ifndef EXTERN_H
#define EXTERN_H

typedef struct argument{
	int msgsock;
	FILE* logFD;
	char log_content[MAX_LOGGING_LENGTH];
	char cgi_dir[MAX_DIRECTORY_LENGTH];
}argument;

// Function in checks.c
long int 	checkDigit(char* string);
int 		checkSegment(char* string);
int  		checkIPAddress(char* string);
int 		checkPort(char* string);
int 		checkRequestLine(char* string,int* methodType,char* uri_return);

// Function in receiver.c
int 		receiver(int msgsock,FILE* logFD,char* log_content,char* cgi);
void 		*receiver_thread(void* thread_arg);

// Function in tools.c
int 		comparator(const void* first, const void* second);
int			indexOrDir(char* path);
int			parse_uri(char* abs_path,char* uri, char* cgi,char* arguments);
int 		parse_argument(char* string,char arguments[ARGUMENT_NUM][ARGUMENT_LENGTH]);
int 		server_daemon(int noclose,char* workspace);
time_t		string2time(char* string);
void		log_connection(char* log_content,char* source_addr);
void 		reverse(char s[]);
void 		my_itoa(int n, char s[]);
void		sig_child();
void 		usage(const char* error);

// Function in sendFunction.c
int 		sendcgi(int to_fd,int method,char* arguments,char* path);
int			senddir(int to_fd, char* path);
int 		sendfile(int to_fd, time_t ltime, char* path);
int 		send_response(int socket_fd, time_t modified_time, char* content, char* content_type, int status_code);

#endif