
#include "sws.h"
#include "extern.h"

//#include <magic.h>

int
senddir(int to_fd, char* path){
  DIR* dp;
  struct dirent *dirp;
  char* current_fileptr;
  char* fileptr;
  size_t fileptr_size;
  size_t nw;
  size_t fptr_cnt;
  char* buff;
  size_t i;
 
  fptr_cnt =0;
  fileptr_size =5;
   
  if((fileptr = (char*)malloc(fileptr_size*MAXFILENAME))==NULL){

    //senderror()internal server error;
    perror("malloc failed");
    exit(-1);
  }
  current_fileptr = fileptr;
  if((dp = opendir(path))==NULL){

    /* send the internal server error */
    perror("unable to opent the dir");
    exit(-1);
  }
  while((dirp = readdir(dp)) != NULL){
    if(dirp->d_name[0]=='.'){
      continue;
    }
    else{
    
    if(fileptr_size <= fptr_cnt){
      fileptr_size = fileptr_size*2;
      if((fileptr = (char*) realloc((void *)fileptr,
                    fileptr_size*MAXFILENAME))==NULL){
    //senderror()internal server error;
    perror("malloc failed");
    exit(-1);
      }
    }
    current_fileptr = fileptr +fptr_cnt*MAXFILENAME;
    strcpy(current_fileptr, dirp->d_name);
    current_fileptr+=MAXFILENAME;
    fptr_cnt +=1;
    }

    
    
  }

  /* 
   *call the logger
   */

  qsort((void*)fileptr, fptr_cnt, MAXFILENAME, comparator);

  if((buff = malloc(fptr_cnt*MAXFILENAME))==NULL){
    //senderror()internal server error;
    perror("malloc failed");
    exit(-1);
  }
  bzero(buff,fptr_cnt*MAXFILENAME);
  current_fileptr = fileptr;
  for(i=0;i<=fptr_cnt;i++){
    strcat(buff, current_fileptr);
    strcat(buff, "\r\n");
    current_fileptr=current_fileptr+MAXFILENAME;
  }
  send_response(to_fd, 0, buff, "Text", 200);
  /*
    if((nw=write(to_fd, fileptr, strlen(fileptr)))<strlen(fileptr)){
    //senderror()internal server error;
    perror("write failed");
    exit(-1);
    
    }*/
  nw = strlen(buff);
  free(fileptr);
  free(buff);

  return nw;
}


int 
sendfile(int to_fd, time_t ltime, char* path){
  size_t buffsize;
  char* buff;
  //char* filetype;
  //magic_t magic_cookie;
  //  char* currentbuffptr;
  struct stat sb;
  //int source;
  //int nr;
  //int nw;
  FILE* f;
  size_t content_size;
  
  
  
  //find the size of the file

  if(stat(path, &sb)!=0){
    //senderror(to_fd, FILENOTFOUND);
    fprintf(stderr, "'stat' failed for '%s': %s.\n",
      path, strerror (errno));
    return EXIT_FAILURE;
  }
  buffsize = sb.st_size + 20;

  if(ltime!=0){
    if(difftime(ltime,sb.st_mtime)>0){
      send_response(to_fd, sb.st_mtime, "Nothing changes after \'If-modified-sience\'", "text/html", 200);
      return 0;
    }
  }


  if((buff = malloc(buffsize))==NULL){
    //senderror(to_fd, SERVERERROR);
    fprintf(stderr, "Unable to allocate memory: for file:%s error: %s\n",
      path, strerror(errno));
    return EXIT_FAILURE;
  }

  /*
  if((magic_cookie = magic_open(MAGIC_NONE))==NULL){
      if((filetype = (char*)malloc(10))==NULL){
  perror("malloc failed");
      }
      strcat(filetype, "unknown");
      
    }
  if (magic_load(magic_cookie, NULL) != 0) {
          printf("cannot load magic database - %s\n", magic_error(magic_cookie));
          magic_close(magic_cookie);
          //return 1;
      }
  filetype = (char*) magic_file(magic_cookie, path);
    //printf("%s\n", magic_full);
    magic_close(magic_cookie);
  */

    f=fopen(path, "rb");
    fread(buff, sb.st_size, 1, f);
    fclose(f);

    content_size = strlen(buff);

    send_response(to_fd, sb.st_mtime, buff, "text/html", 200);
    free(buff);
      
  
  return content_size;
  
}

int 
sendcgi(int to_fd,int method,char* arguments,char* path){
  int count;
  int input_fd[2];
  int output_fd[2];
  int pid;
  int response_size;
  int status;

  char requestMethod[ARGUMENT_LENGTH];
  char querryString[ARGUMENT_LENGTH];
  char* content;

  struct stat sb;
  time_t now;
  
  count=1;
  response_size=0;
  content=(char*)malloc(sizeof(char)*CONTENT_UNIT*count);
  time(&now);

  if(stat(path, &sb)!=0){
    response_size=send_response(to_fd,now,"Error","text/html",404);
    //fprintf(stderr, "'stat' failed for '%s': %s.\n",path, strerror (errno));
    return response_size;
  }

  switch(method){
    case METHOD_GET:
      sprintf(requestMethod,"REQUEST_METHOD=GET");
      break;
    case METHOD_HEAD:
      sprintf(requestMethod,"REQUEST_METHOD=HEAD");
      break;
    default:
      response_size=send_response(to_fd,now,"Error","text/html",400);  
      return response_size;
  }

  if(pipe(input_fd)<0){
    response_size=send_response(to_fd,now,"Error","text/html",500);
    return response_size;
  }
  if(pipe(output_fd)<0){
    response_size=send_response(to_fd,now,"Error","text/html",500);
    return response_size;
  }

  if((pid=fork())<0){
    response_size=send_response(to_fd,now,"Error","text/html",500);
    return response_size;
  }

  if(pid==0){
    dup2(output_fd[1],1);
    dup2(input_fd[0],0);
    close(output_fd[0]);
    close(input_fd[1]);

    putenv(requestMethod);
    if(arguments!=NULL){
      sprintf(querryString,"QUERY_STRING=%s",arguments);
      putenv(querryString);
    }
    execlp(path,path,(char*)NULL);
    return 0;
  }
  else{
    close(output_fd[1]);
    close(input_fd[0]);

    char* temp=content;
    while(read(output_fd[0],temp,CONTENT_UNIT)>0){
      count+=1;
      content=(char*)realloc(content,sizeof(char)*count*CONTENT_UNIT);
      temp=content+(count-1)*CONTENT_UNIT;
    }
    close(output_fd[0]);
    close(input_fd[1]);

    response_size=send_response(to_fd,now,content,"text/html",200);
    //printf("%s\n", content); 
    free(content);

    waitpid(pid, &status, 0);

    return response_size;
  }
  
}

int 
send_response(int socket_fd, time_t modified_time, char* content, char* content_type, int status_code){
  char* buff;
  size_t buff_size;
  struct tm tm;
  char date_buff[50];
  time_t now;
  char content_size[10];
  size_t nw;
  
  buff_size = strlen(content) + 300;
  if((buff = (char*) malloc(buff_size))==NULL){
    send_response(socket_fd, 0, "", "", 300);
    perror("malloc failed");
    return -1;
  }
  bzero(buff,buff_size);
  strcat(buff, "HTTP/1.0 ");
  switch(status_code){
  case 200:
    strcat(buff, "200 OK\r\n");
    break;
  case 201:
    strcat(buff, "201 Created\r\n");
    break;
  case 202:
    strcat(buff, "202 Accepted\r\n");
    break;
  case 204:
    strcat(buff, "204 No Content\r\n");
    break;
  case 301:
    strcat(buff, "301 Moved Permanently\r\n");
    break;
  case 304:
    strcat(buff, "304 Not Modified\r\n");
    break;
  case 400:
    strcat(buff, "400 Bad Request\r\n");
    break;
  case 401:
    strcat(buff, "401 Unauthorized\r\n");
    break;
  case 403:
    strcat(buff, "403 Forbidden\r\n");
    break;
  case 404:
    strcat(buff, "404 Not Found\r\n");
    break;
  case 500:
    strcat(buff, "500 Internal SErver Error\r\n");
    break;
  case 501:
    strcat(buff, "501 Not Implemented\r\n");
    break;
  case 502:
    strcat(buff, "502 Bad Gateway\r\n");
    break;
  case 503:
    strcat(buff, "503 Service Unavailable\r\n");
    
  }

  now = time(0);
  tm = *gmtime(&now);
  strftime(date_buff, sizeof(date_buff), "%a, %d %b %Y %H:%M:%S %Z", &tm);
  strcat(buff, "Date: ");
  strcat(buff, date_buff);
  strcat(buff, "\r\n");
  
  strcat(buff, "Server: ");
  strcat(buff, NAME);
  strcat(buff, "\r\n");
      
  if(modified_time>0){
    tm = *gmtime(&modified_time);
    strftime(date_buff, sizeof(date_buff), "%a, %d %b %Y %H:%M:%S %Z", &tm);
    strcat(buff, "Last-Modified: ");
    strcat(buff, date_buff);
    strcat(buff, "\r\n");
  }

  strcat(buff, "Content-Type: ");
  strcat(buff, content_type);
  strcat(buff, "\r\n");

  strcat(buff, "Content-Length: ");
  my_itoa(strlen(content), content_size);
  strcat(buff, content_size);
  strcat(buff, "\r\n\r\n");

  strcat(buff, content);
  printf("%s\n",buff);
  strcat(buff, "\r\n\r\n");

  if((nw = write(socket_fd, buff, strlen(buff)+1))!=(strlen(buff)+1)){
    perror("unable to write to the socket");
  }
  return sizeof(buff);
}
