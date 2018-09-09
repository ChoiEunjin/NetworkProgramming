#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<sys/file.h>
#include<netinet/in.h>
#include<unistd.h>
#include<time.h>
#include<fcntl.h>
#include<arpa/inet.h>

#define MAXBUF	500000
#define MAXLINE	127

int tcp_connect(int af, char *servip, unsigned short port);
void upload(int s);
void download(int s);

int main(int argc, char *argv[])
{
	struct sockaddr_in servaddr;
	int s, nbyte;
	char command[10];

	if(argc != 3){
		printf("Input IP address, Port number\n");
		exit(0);
	}

	s = tcp_connect(AF_INET, argv[1], atoi(argv[2]));
	if(s == -1) {
		printf("tcp_connect fail");
		exit(0);
	}

	printf("Choose the command (put, get) : ");
	scanf("%s", command);
	send(s, command, strlen(command), 0);

	if(strcmp(command, "put") == 0)	upload(s);
	else if(strcmp(command, "get") == 0)	download(s);
	else {
		perror("Input right command");
		exit(0);
	}
		
	return 0;
}

int tcp_connect(int af, char *servip, unsigned short port)
{
	struct sockaddr_in servaddr;
	int s;
	if((s = socket(af, SOCK_STREAM, 0)) < 0)
		return -1;
	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = af;
	inet_pton(AF_INET, servip, &servaddr.sin_addr);
	servaddr.sin_port = htons(port);
	if(connect(s, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
		return -1;
	return s;
}

void upload(int s)
{
	char filename[MAXLINE];
	char buf[MAXBUF];
	int len, fd, read_len;
	FILE *file = NULL;

	// 1. input file name
	memset(filename, 0x00, MAXLINE);
	printf("Input file name : ");
	scanf("%s", filename);
	len = strlen(filename);
	
	send(s, filename, len, 0);

	if(strstr(filename, "binary") != NULL){
		sleep(1);

		size_t fsize = 0, nsize = 0, fpsize = 0;

		file = fopen(filename, "rb");
		
		fseek(file, 0, SEEK_END);
		fsize = ftell(file);
		fseek(file, 0, SEEK_SET);

		send(s, &fsize, sizeof(fsize), 0);
		printf("file size = [%lu]\n", fsize);

		sleep(1);
		
		while(nsize != fsize){
			fpsize = fread(buf, 1, MAXLINE, file);
			nsize = nsize + fpsize;
			send(s, buf, fpsize, 0);
		}
		fclose(file);
	}
	else {

		sleep(1);

		memset(buf, 0x00, MAXBUF); 
		fd = open(filename, O_RDONLY);
		if(!fd) { perror("Error : "); exit(0); } 

		while(read_len = read(fd, buf, MAXBUF))
			send(s, buf, read_len, 0); 
	}
}

void download(int s)
{
	// 1. show files and permission of '/server'
	system("ls /home/diliah0826/server");
	char filename[MAXLINE];
	char buf[MAXBUF];
	int len, fd, read_len, path_len, nbyte;
	FILE *file = NULL;
	size_t filesize = 0, bufsize = 0;	

	// 1. input file name
	memset(filename, 0x00, MAXLINE);
	printf("Input file name : ");
	scanf("%s", filename);
	len = strlen(filename);

	// 2. input path
	system("ls -l /home/diliah0826");
	char path[MAXLINE];
	printf("Save at... : ");
	scanf("%s", path);

	path_len = strlen(path);
	path[path_len] = '/';
	strcat(path, filename);

	send(s, filename, len, 0);
	sleep(1);

	if(strstr(filename, "binary") != NULL){
		file = fopen(path, "wb");
		read(s, &filesize, sizeof(filesize)); 
		printf("file size = [%lu]\n", filesize);
		bufsize = MAXLINE;

		while(filesize != 0){
			if(filesize < MAXLINE)
				bufsize = filesize;
			nbyte = recv(s, buf, bufsize, 0);
			filesize = filesize - nbyte;
			fwrite(buf, sizeof(char), nbyte, file);
			nbyte = 0;
			
		}
		fclose(file);
	}
	else{
		memset(buf, 0x00, MAXBUF); 
		fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0700);
		if(!fd) { perror("Error : "); exit(0); } 

		/* file save */
		memset(buf, 0x00, MAXBUF);
		read_len = read(s, buf, MAXBUF);
		write(fd, buf, read_len);
	}
}

