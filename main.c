/*
 * This is the main part of sws
*/

#include "extern.h"
#include "sws.h"

/*
 * This is the main funciont of sws
 * We start our program here
*/
int
main(int argc, char** argv){
	int mode;
	int port;
	int sock;
	int msgsock;

	const char* options, *synopsis;

	char ch;
	char log_content[MAX_LOGGING_LENGTH];
	//char timebuf[128];

	char* cgi_dir;
	char* dir;
	char* log_file;
	char* ipTemp,*portTemp;
	char* ipv6;

	FILE* logFD;

	socklen_t length;

	struct argument thread_arg;
	struct sockaddr_in6 server;
	struct sockaddr_in6 client;

	pid_t pid;

	pthread_t tid;

	mode=MODE_MULTIPROCESS;

	cgi_dir=NULL;
	dir=NULL;
	log_file=NULL;
	logFD=NULL;
	options="c:dhit::l:p::";
	synopsis="sws [-dht] [-c dir] [-i ip address] [-p port] [-l file] dir";

	port=8080;
	ipTemp=portTemp=NULL;
	ipv6=NULL;

	if(argc<2){
		usage(synopsis);
		exit(EXIT_FAILURE);
	}

	while((ch=getopt(argc,argv,options))!=-1){
		switch(ch){
			case 'c':
				if((cgi_dir=(char*)malloc(sizeof(char)*MAX_DIRECTORY_LENGTH))==NULL)
					usage(strerror(errno));
				strncpy(cgi_dir,optarg,MAX_DIRECTORY_LENGTH);
				printf("cgi_dir: is %s\n",cgi_dir);
				break;
			case 'd':
				mode=MODE_DEBUG;
				break;
			case 'h':
				usage(synopsis);
			case 'i':
				if(optarg!=NULL){
					if((ipTemp=(char*)malloc(sizeof(char)*strlen(optarg)))==NULL)
						usage(strerror(errno));
					strcpy(ipTemp,optarg);
				}
				else if(argv[optind]&&*(argv[optind])!='-'){
					if((ipTemp=(char*)malloc(sizeof(char)*strlen(argv[optind])))==NULL)
						usage(strerror(errno));
					strcpy(ipTemp,argv[optind]);
					optind+=1;
				}
				break;
			case 'l':
				if((log_file=(char*)malloc(sizeof(char)*MAX_DIRECTORY_LENGTH))==NULL)
					usage(strerror(errno));
				strncpy(log_file,optarg,MAX_DIRECTORY_LENGTH);
				if((logFD=fopen(optarg,"a+"))==NULL)
        			usage(strerror(errno));
				printf("l: is given");
				break;
			case 'p':
				if(optarg!=NULL){
					if((portTemp=(char*)malloc(sizeof(char)*strlen(optarg)))==NULL)
						usage(strerror(errno));
					strcpy(portTemp,optarg);
				}
				else if(argv[optind]&&*(argv[optind])!='-'){
					if((portTemp=(char*)malloc(sizeof(char)*strlen(argv[optind])))==NULL)
						usage(strerror(errno));
					strcpy(portTemp,argv[optind]);
					optind+=1;
				}
				break;
			case 't':
				mode=MODE_MULTITHREAD;
				break;
			default:
				usage(synopsis);
		}
	}

	if(optind>=argc)
		usage(synopsis);
	dir=argv[optind];
	printf("directory: %s\n",dir);
	if(mode==MODE_DEBUG)
		logFD=stdout;
	else if(!logFD){
		if((logFD=fopen(PATH_LOG_DEFAULT,"a+"))==NULL)
        	usage(strerror(errno));
	}
	if(optind+1<argc)
		usage(synopsis);

	printf("Finish reading options\n");

	if(ipTemp)
		if(checkIPAddress(ipTemp)==0){
			if((ipv6=(char*)malloc(sizeof(char)*\
							(strlen(ipTemp)+strlen("::FFFF:")+1)))==NULL)
				usage(strerror(errno));
			strcpy(ipv6,"::FFFF:");
			strncat(ipv6,ipTemp,strlen(ipTemp));
		}

	// check ip address and port
	if(ipTemp){
		int a;
		if((a=inet_pton(AF_INET6,ipv6,server.sin6_addr.s6_addr))==0)
			usage("This is not a vaild ip address");
		else if(a<0)
			usage(strerror(errno));
		printf("ip address get: %s\n", ipTemp);
	}
	else{
		server.sin6_addr=in6addr_any;
		printf("listen all ip address\n");
	}
	server.sin6_family=AF_INET6;
	if(portTemp)
		port=checkPort(portTemp);
	server.sin6_port=htons(port);
	printf("port is :%d\n", port);

	if((sock=socket(AF_INET6,SOCK_STREAM,0))<0)
		usage("opening stream socket fail");
	if(bind(sock,(struct sockaddr*)&server,sizeof(server))!=0)
		usage(strerror(errno));

	if(mode!=MODE_DEBUG){
		printf("Server start at backend\n");
		if(server_daemon(0,"/home/oem/Desktop/631team")==EXIT_FAILURE)
			usage(strerror(errno));
	}
	
	listen(sock,SERVER_QUEUE_LENGHT);
	signal(SIGCHLD,sig_child);
	while(1){
		length=sizeof(client);
		msgsock=accept(sock,(struct sockaddr*)&client,&length);

		if((ipTemp=(char*)malloc(sizeof(char)*MAX_IPv6_LENGTH))==NULL)
			usage(strerror(errno));

		if(msgsock==-1)
			usage("accept error");
		if(mode==MODE_DEBUG){
			if(logFD){
				if(!inet_ntop(AF_INET6,&(client.sin6_addr),ipTemp,MAX_IPv6_LENGTH))
					usage(strerror(errno));
				log_connection(log_content,ipTemp);
			}
			receiver(msgsock,logFD,log_content,cgi_dir);
		}
		else if(mode==MODE_MULTIPROCESS){
			if((pid=fork())<0)
				usage("fork error");
			else if(pid>0){
				close(msgsock);
			}
			else{
				close(sock);
				if(logFD){
					if(!inet_ntop(AF_INET6,&(client.sin6_addr),ipTemp,MAX_IPv6_LENGTH))
						usage(strerror(errno));
					log_connection(log_content,ipTemp);
				}
				receiver(msgsock,logFD,log_content,cgi_dir);
				return EXIT_SUCCESS;
			}
		}
		else if (mode==MODE_MULTITHREAD){
			thread_arg.msgsock=msgsock;
			thread_arg.logFD=logFD;
			strncpy(thread_arg.log_content,log_content,MAX_LOGGING_LENGTH);
			if(cgi_dir)
				strncpy(thread_arg.cgi_dir,cgi_dir,MAX_LOGGING_LENGTH);
			if(pthread_create(&tid,NULL,&receiver_thread,&thread_arg))
				usage(strerror(errno));
			if(pthread_detach(tid))
				usage(strerror(tid));
		}
	}

	return EXIT_SUCCESS;
}