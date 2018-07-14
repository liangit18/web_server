/*
 * This file contain some tools used by sws
*/

#include "sws.h"
#include "extern.h"

int 
comparator(const void* first, const void* second){
  char* f = (char*) first;
  char* s = (char*) second;
  return strcmp(f,s);
}

int
indexOrDir(char* path){
    struct stat file;
    char* path_temp;

    if(stat(path,&file)==-1)
        return 404;

    if(S_ISDIR(file.st_mode)){
        if((path_temp=(char*)malloc(sizeof(char)*MAX_DIRECTORY_LENGTH))==NULL)
            usage(strerror(errno));
        strcpy(path_temp,path);

        if(strlen(path)+strlen(DEFAULT_INDEX)<MAX_DIRECTORY_LENGTH){
            if(path[strlen(path)-1]!='/')
                strcat(path_temp,"/");
            strcat(path_temp,DEFAULT_INDEX);
            if(stat(path_temp,&file)==0){
                if(S_ISDIR(file.st_mode))
                    return RESPONSE_DIR;
                else{
                    strcpy(path,path_temp);
                    free(path_temp);
                    return RESPONSE_FILE;
                }
            }
            else{
                free(path_temp);
                return RESPONSE_DIR;
            }
        }
        else{
            free(path_temp);
            return 400;
        }
    }
    else{
        return RESPONSE_FILE;
    }
    return 0;
}

int 
parse_argument(char* string,char arguments[ARGUMENT_NUM][ARGUMENT_LENGTH]){
    int i;

    i=0;
    string=strtok(string,"&");
    do{
        strncpy(arguments[i],string,strlen(string));
        string=strtok(NULL,"&");
        i+=1;
    }while(i<ARGUMENT_NUM-1&&string);
    strncpy(arguments[i],"\0",2);
    if(string)
        return 400;
    return 0;
}

int
parse_uri(char* abs_path,char* uri, char* cgi,char* arguments){
    int end;
    int count;
    int ret;

    ret=RESPONSE_DIR;

    if(cgi && (strncmp(uri,"cgi-bin",strlen("cgi-bin"))==0||\
               strncmp(uri,"/cgi-bin",strlen("/cgi-bin"))==0)){
        if(strncpy(abs_path,cgi,MAX_DIRECTORY_LENGTH)==NULL)
                usage("strncpy error");
        if(abs_path[strlen(abs_path)-1]=='/')
            abs_path[strlen(abs_path)-1]='\0';
        if(uri[0]=='/')
            uri+=1;
        uri=strtok(uri,"/");
        ret=RESPONSE_CGI;
    }
    else{
        if((uri[0]=='~')||(uri[0]=='/'&&uri[1]=='~')){
            strncpy(abs_path,"/home/",MAX_URI_LENGTH);
            uri=strtok(uri,"/");
            strncat(abs_path,uri+1,MAX_URI_LENGTH);
            strncat(abs_path,"/sws",MAX_URI_LENGTH);
            uri=strtok(NULL,"/");
            //ret=RESPONSE_USER;
        }
        else{
            if(getcwd(abs_path,MAX_URI_LENGTH)==NULL)
                usage("getcwd error");
            uri=strtok(uri,"/");
        }
    }

    if(uri!=NULL){
        count=0;
        do{
            if(strlen(uri)==0)
                continue;

            if(strcmp(uri,".")==0)
                continue;
            else if(strcmp(uri,"..")==0){
                if(count==0)
                    return 403;
                else
                    count-=1;
                
                end=strlen(abs_path)-1;
                while(end>=0&&abs_path[end]!='/')
                    end-=1;
                if(end<0||strlen(abs_path)==0)
                    return 403;
                else
                    abs_path[end]='\0';
             }
            else{
                count+=1;
                if(cgi&&strstr(uri,"?")!=NULL){
                    uri=strtok(uri,"?");
                    strncat(abs_path,"/",MAX_URI_LENGTH);
                    strncat(abs_path,uri,MAX_URI_LENGTH);
                    uri=strtok(NULL,"?");
                    if(uri){
                        sprintf(arguments,"%s",uri);
                        printf("arguments is %s\n", arguments);
                    }
                    break;
                }
                else{
                    strncat(abs_path,"/",MAX_URI_LENGTH);
                    strncat(abs_path,uri,MAX_URI_LENGTH);
                }
            }
        }while((uri=strtok(NULL,"/"))!=NULL);
    }
    if(ret!=RESPONSE_CGI)
        ret=indexOrDir(abs_path);
    return ret;
}

int 
server_daemon(int noclose,char* workspace){
    if(chdir(workspace)<0)
        usage(strerror(errno));

    if(daemon(1,noclose)==0)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}

time_t
string2time(char* string){

    struct tm T;
    time_t ret;

    if(strlen(string)<1)
        return 0;

    if(strptime(string," %a, %d %b %Y %H:%M:%S GMT",&T)==NULL)
        return -1;

    ret=mktime(&T);
    
    return ret;
}

void
log_connection(char* log_content,char* source_addr){
    char timebuf[128];
    struct tm *gtime;
    time_t now;

    bzero(log_content,MAX_LOGGING_LENGTH);
    strcat(log_content,source_addr);
            
    time(&now);
    gtime=gmtime(&now);
    if((strftime(timebuf,128," %a, %d %b %y %H:%M:%S %Z",gtime))<=0)
        usage("strftime error");
    strcat(log_content,timebuf);
}

/*
 * Transform a integer to string
*/
void 
my_itoa(int n, char s[]){
     int i, sign;

     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
}  

void 
reverse(char s[]){
     int i, j;
     char c;

     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
}  

void
sig_child(){
    int status;
    waitpid(-1,&status,0);
}

/*
 * When error happen, this funcion will be called
*/
void
usage(const char* error){
    (void)fprintf(stderr,"usage:%s \n",error);
    exit(EXIT_FAILURE);
}


