#include "sws.h"
#include "extern.h"

int
receiver(int msgsock,FILE *logFD,char* log_content,char* cgi){
	int line;
	int method;
	int rval;
	int status_code;
	int sel;
	int response_size;

	char* first_line;
	char* temp;
	char CRLF[5];
	char status_str[3];
	char receive_buff[MAX_RECEIVE_BUFF];
	char header[MAX_NUM_HEADER_LINE][MAX_HTTP_HEADER_LINE];
	char uri[MAX_URI_LENGTH];
	char abs_path[MAX_URI_LENGTH];
	char arguments[ARGUMENT_LENGTH];
	char response_size_c[8];

	fd_set readfds;
	struct timeval timeout;
	time_t timestamp;

	timestamp=0;
	response_size=0;
	FD_SET(msgsock,&readfds);
	FILE *target=fopen("result.txt", "a");

	line=0;
	while(line<MAX_NUM_HEADER_LINE){
		timeout.tv_sec=REQUEST_WAIT;
		timeout.tv_usec=0;

		sel=select(msgsock+1,&readfds,NULL,NULL,&timeout);
		if(sel<0)
			usage(strerror(errno));
		else if(sel==0){
			printf("time out\n");
			if((sel=close(msgsock))==-1)
				usage(strerror(errno));
			return -1;
		}

		bzero(receive_buff,MAX_RECEIVE_BUFF);
		if((rval=recv(msgsock,receive_buff,MAX_RECEIVE_BUFF,0))<0){
			usage(strerror(errno));
			close(msgsock);
		}

		if(line==0&&((temp=strstr(receive_buff,"\r\n"))||(temp=strstr(receive_buff,"\n")))&&temp==receive_buff)
			continue;
		if(line>0&&strncmp(receive_buff,"\r\n",strlen("\r\n"))==0)
			break;
		else if(line>0&&strncmp(receive_buff,"\n",strlen("\n"))==0)
			break;
		
		if((temp=strtok(receive_buff,"\r\n"))){
			if(!strncpy(CRLF,"\r\n",5))
				usage(strerror(errno));
		}
		else if((temp=strtok(receive_buff,"\n"))){
			if(!strncpy(CRLF,"\n",5))
				usage(strerror(errno));
		}
		else{
			strncpy(header[line],"empty",MAX_HTTP_HEADER_LINE);
			line+=1;
			continue;
		}
		
		strncpy(header[line],temp,MAX_HTTP_HEADER_LINE);
		fprintf(target,"%d %s\n",(int)strlen(header[line]) ,header[line]);
		temp=strtok(NULL,CRLF);
		line+=1;
		int count=0;
		while(temp!=NULL&&line<MAX_NUM_HEADER_LINE){
			strncpy(header[line],temp,MAX_HTTP_HEADER_LINE);
			temp=strtok(NULL,CRLF);
			fprintf(target,"%d %s\n",(int)strlen(header[line]) ,header[line]);
			if(!temp){
				if(line>0)
					count=-1;
				line+=1;
				break;
			}
			count+=1;
			line+=1;
		}
		if(count==-1)
			break;
	}

	if(rval==0){
		printf("Finish reading header\n");
	}
	else{
		if((first_line=(char*)malloc(sizeof(char)*MAX_LOGGING_LENGTH))==NULL)
			usage(strerror(errno));
		strncpy(first_line,header[0],MAX_LOGGING_LENGTH);
		first_line=strtok(first_line, "\r\n");

		status_code=checkRequestLine(header[0],&method,uri);

		int i=0;
		while(i<line){
			if(strncmp(header[i],"If-Modified-Since:",strlen("If-Modified-Since:"))==0){
				timestamp=string2time(header[i]+strlen("If-Modified-Since:"));
				if(timestamp<0)
					status_code=400;
				break;
			}
			i+=1;
		}

		time_t now;
		time(&now);

		printf("method is %d\n",method);
		printf("uri is %s\n",uri);
		if(status_code==200){
			switch(parse_uri(abs_path,uri,cgi,arguments)){
				case 400:
					status_code=400;
					response_size=send_response(msgsock,now,"Error","text/html",status_code);
					break;
				case 404:
					status_code=404;
					response_size=send_response(msgsock,now,"Error","text/html",status_code);
					break;
				case RESPONSE_DIR:
					response_size=senddir(msgsock,abs_path);
					break;
				case RESPONSE_FILE:
					response_size=sendfile(msgsock, timestamp, abs_path);
					break;
				case RESPONSE_CGI:
					response_size=sendcgi(msgsock,method,arguments,abs_path);
					break;
				default:
					break;
			}
		}
		else
			response_size=send_response(msgsock,now,"Error","text/html",status_code);
		printf("abs_path is %s\n",abs_path);
		printf("%d\n", status_code);
		my_itoa(status_code,status_str);
		my_itoa(response_size,response_size_c);
		close(msgsock);
		if(logFD){
			strcat(log_content," \"");
			strcat(log_content,first_line);
			strcat(log_content,"\" ");
			strcat(log_content,status_str);
			strcat(log_content," ");
			strcat(log_content,response_size_c);
			strcat(log_content,"bytes");
			fprintf(logFD, "%s\n", log_content);
			free(first_line);
		}
		printf("Finish\n");
	}
	fclose(target);
	return 0;
}

void
*receiver_thread(void* thread_arg){
	struct argument* arg;

	arg=thread_arg;

	receiver(arg->msgsock,\
			arg->logFD,\
			arg->log_content,\
			arg->cgi_dir);
	return NULL;
}