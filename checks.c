/*
 * This file contains all check functions
 * Check ip address, port, HTTP header
*/

#include "sws.h"
#include "extern.h"

/*
 * check if the input string is a number
 * if not return -1; 
*/
long int
checkDigit(char* string){
	char* i;
	
	i=string;
	while(*i!='\0'){
		if(!isdigit(*i))
			return -1;
		i+=1;
	}
	return atol(string);
}

/*
 * check if the input string is a hexadecimal number
 * if not return -1; 
*/
int
checkSegment(char* string){
	char* i;
	int count;

	count=0;
	i=string;
	while(*i!='\0'){
		if(!isxdigit(*i))
			return -1;
		count+=1;
		i+=1;
	}
	if(count<=4)
		return 0;
	else
		return -1;
}

/*
 * This Funcion will be called by main
 * return 0--ipv4, 1--ipv6, or jump to usage when error
*/
int
checkIPAddress(char* string){
	int ipVersion;	//1:ipv4 -1:ipv6 0 invalid
	char* i;

	ipVersion=0;
	i=string;

	while(*i!='\0'){
		if(*i=='.'){
			ipVersion=1;
			break;
		}
		else if(*i==':'){
			ipVersion=-1;
			break;
		}
		i+=1;
	}

	if(ipVersion==0)
		usage("your ip address is nither ipv4 nor ipv6");

	return ipVersion==1?0:1;
}

/*
 * This Funcion will be called by main
 * This funciont check whether input string is valid port
 * return port number or jump to usage when error
*/
int
checkPort(char* string){
	long int p;

	p=0;

	if((p=checkDigit(string))==-1)
		usage("your port number invalid");
	if(p>MAX_PORT||p<MIN_PORT)
		usage("port number must between 1024 and 65536");

	return p;
}

/*
 * This Function can check the request-line
 * If it is right, return 2xx
 * If method not in GET,HED return 400
 * If protocol is not HTTP/1.0 return 505
 * if uri is too long return 400
*/
int 
checkRequestLine(char* string,int* methodType,char* uri_return){
	int i;
	char* method;
	char* uri;
	char* HTTP;

	method=strtok(string," ");
	if(!method){
		return 400;
	}
	else if(strcmp(method,"GET")==0){
		*methodType=METHOD_GET;
	}
	else if(strcmp(method,"HEAD")==0){
		*methodType=METHOD_HEAD;
	}
	else{
		return 400;
	}

	uri=strtok(NULL," ");
	if(!uri||strlen(uri)>MAX_URI_LENGTH)
		return 400;
	strncpy(uri_return,uri,strlen(uri));
	uri_return[strlen(uri)]='\0';

	HTTP=strtok(NULL, "\r\n");
	if(!HTTP)
		return 200;
	while(*HTTP==' ')
		HTTP+=1;
	if(strncmp(HTTP,"HTTP/1.1",strlen("HTTP/1.0"))==0){
		i=8;
		while(*(HTTP+i)){
			if(*(HTTP+i)!=' ')
				return 503;
			i+=1;
		}
	}
	else
		return 400;
	
	return 200;
}