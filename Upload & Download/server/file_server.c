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
#include <errno.h>

#define MAXLINE 127
#define MAXBUF	500000

void upload(int accp_sock);
void download(int accp_sock);
int tcp_listen(int host, int port, int backlog);

int main(int argc, char *argv[])
{
	struct sockaddr_in servaddr, cliaddr;
	int listen_sock, accp_sock, addrlen = sizeof(cliaddr), nbyte;
	char command[10];

	if(argc != 2){
		printf("usage : %s port\n", argv[0]);
		exit(0);
	}

	listen_sock = tcp_listen(INADDR_ANY, atoi(argv[1]), 5);
	listen(listen_sock, 5);
	puts("wait for client...");
	
				
	accp_sock = accept(listen_sock, (struct sockaddr *)&cliaddr, &addrlen);
	if(accp_sock < 0){
		perror("accept fail");
		exit(0);
	}
	puts("client is connected...");
	
	// 1. check command
	read(accp_sock, command, MAXLINE);
	if(strcmp(command, "put") == 0)
		upload(accp_sock);
	else if(strcmp(command, "get") == 0)
		download(accp_sock);
	else{
		perror("Input right command");
		exit(0);
	}
	close(accp_sock);
	close(listen_sock);
}

void upload(int accp_sock)
{
	int read_len, des_fd, file_read_len, nbyte;
	char file_name[MAXLINE]; // local val
	char buf[MAXBUF];

	FILE *file = NULL;
	size_t filesize = 0, bufsize = 0;	

	/* file name */
	memset(file_name, 0x00, MAXLINE);
	read_len = read(accp_sock, file_name, MAXLINE);

	if(strstr(file_name, "binary") != NULL){
		file = fopen(file_name, "wb");
		read(accp_sock, &filesize, sizeof(filesize)); 
		printf("file size = [%lu]\n", filesize);

		bufsize = MAXLINE;

		while(filesize != 0){
			if(filesize < MAXLINE)
				bufsize = filesize;
			nbyte = recv(accp_sock, buf, bufsize, 0);
			filesize = filesize - nbyte;
			fwrite(buf, sizeof(char), nbyte, file);
			nbyte = 0;
			
		}
		fclose(file);
	}
	else{
		des_fd = open(file_name, O_WRONLY | O_CREAT | O_EXCL, 0700);
		if(!des_fd) {
			perror("file open error : ");
			exit(1);
		}

		memset(buf, 0x00, MAXBUF);
		file_read_len = recv(accp_sock, buf, MAXBUF, 0);
		write(des_fd, buf, file_read_len);

		printf("Upload Successfully!!!\n");
	}
	close(des_fd);
}

void download(int accp_sock)
{
	int read_len, des_fd, file_read_len;
	char file_name[MAXLINE]; // local val
	char buf[MAXBUF];
	FILE *file = NULL;

	/* file name */
	memset(file_name, 0x00, MAXLINE);
	read_len = read(accp_sock, file_name, MAXLINE);

	if(strstr(file_name, "binary") != NULL){
		sleep(1);

		size_t fsize = 0, nsize = 0, fpsize = 0;
		file = fopen(file_name, "rb");
		
		fseek(file, 0, SEEK_END);
		fsize = ftell(file);
		fseek(file, 0, SEEK_SET);

		send(accp_sock, &fsize, sizeof(fsize), 0);
		printf("file size = [%lu]\n", fsize);

		sleep(1);
		
		while(nsize != fsize){
			fpsize = fread(buf, 1, MAXLINE, file);
			nsize = nsize + fpsize;
			send(accp_sock, buf, fpsize, 0);
		}
		fclose(file);
	}
	else{
		sleep(1);
		des_fd = open(file_name, O_RDONLY, 0700);
		if(!des_fd) {
			perror("file open error : ");
			exit(1);
		}
		while(read_len = read(des_fd, buf, MAXBUF))
			send(accp_sock, buf, read_len, 0); 
	}
	printf("Download Successfully!!!\n");
}

int tcp_listen(int host, int port, int backlog)
{
	int sd;
	struct sockaddr_in servaddr;
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if(sd == -1){ perror("socket fail"); exit(1); }

	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(host);
	servaddr.sin_port = htons(port);

	if(bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		perror("bind fail");
		exit(1);
	}
	listen(sd, backlog);
	return sd;
}


