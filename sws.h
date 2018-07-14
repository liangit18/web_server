/* Name: Marlon Seaton
 * CS 631
 * Final Project
 * 
 */

#ifndef SWS_H
#define SWS_H

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define ARGUMENT_NUM	5
#define ARGUMENT_LENGTH	255

#define CONTENT_UNIT	1024

#define DEBUG_MODE		1
#define NONDEBUG_MODE	2

#define MAX_DIRECTORY_LENGTH 256
#define MAXFILENAME 1024

#define MAX_IPv4_LENGTH 15
#define MAX_IPv6_LENGTH 45
#define MIN_PORT 		1024
#define MAX_PORT 		65535

#define MAX_RECEIVE_BUFF		4096
#define MAX_HTTP_HEADER_LINE 	1024
#define MAX_NUM_HEADER_LINE 	20
#define MAX_URI_LENGTH			1000

#define MAX_LOGGING_LENGTH 150

#define METHOD_GET 	0
#define METHOD_HEAD	1

#define MODE_DEBUG			0
#define MODE_MULTIPROCESS	1
#define MODE_MULTITHREAD	2

#define REQUEST_WAIT	500

#define RESPONSE_FILE	1
#define RESPONSE_DIR	2
#define RESPONSE_CGI	3

#define SERVER_QUEUE_LENGHT		5

#define _PATH_DEVNULL 		"/dev/null"
#define DEFAULT_INDEX 		"index.html"
#define NAME 				"SWS-Liang"
#define PATH_LOG_DEFAULT	"./log_default.txt"

#endif
