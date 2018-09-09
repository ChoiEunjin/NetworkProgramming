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

#define MAX_SOCK	10
#define MAXLINE_S	127
#define MAX_ID		10
#define MAX_PW		10
#define MEM		5
#define MAXLINE_C	1000
#define NAME_LEN	20
#define MIN_ACCESS	5

char *EXIT_STRING = "exit";
char *NEW_STRING = "New client is accessed. Please wait for a while....\n";
char *START_STRING = "There are enough people to chat. Now you can send message.\n";
char *WAIT_STRING = "There are less people to chat. Wait for new client.\n";

int maxfdp1;
int num_chat = 0;
int clisock_list[MAX_SOCK];
char *login_time[MAX_SOCK];
int listen_sock;
int random_port;
char *ip="192.168.31.132";

void addClient(int s, struct sockaddr_in *newcliaddr);
void removeClient(int s);
int getmax();
int tcp_listen(int host, int port, int backlog);
int tcp_connect(int af, char *servip, unsigned short port);

void errquit(char *mesg) {perror(mesg); exit(1);}

void chat_server();
void chat_client(char *id, char *pw, char *mode, char *ip_address);

typedef struct _Login Login;
struct _Login
{
	char id[MAX_ID];
	char pw[MAX_PW];
};

void keep_data(int who, char *bufmsg);
char *printTime(int mode);

void chat_server()
{
	struct sockaddr_in cliaddr;

	char buf[MAXLINE_S+1];
	int i, j, nbyte, accp_sock, addrlen = sizeof(struct sockaddr_in);
	fd_set read_fds;

	int fd = open("login.txt", O_WRONLY | O_TRUNC);
	close(fd);

	listen_sock = tcp_listen(INADDR_ANY, random_port, 5);
	puts("wait for client...");		

	while(1){
		FD_ZERO(&read_fds);
		FD_SET(listen_sock, &read_fds);
		
		for(i=0; i<num_chat; i++)
			FD_SET(clisock_list[i], &read_fds);
		maxfdp1 = getmax() + 1;
		
		if(select(maxfdp1, &read_fds, NULL, NULL, NULL) < 0)
			errquit("select fail");

		if(FD_ISSET(listen_sock, &read_fds))
		{
			accp_sock = accept(listen_sock, (struct sockaddr *)&cliaddr, &addrlen);

			if(accp_sock == -1)
				errquit("accept fail");

			addClient(accp_sock, &cliaddr);

			if(num_chat < MIN_ACCESS)
				send(accp_sock, NEW_STRING, strlen(NEW_STRING), 0);
			else{
				for(i=0; i<num_chat; i++)
					send(clisock_list[i], START_STRING, strlen(START_STRING), 0);
			}

			printf("------------------------------------------\n");
			printf("| Client Number : %d\t\t\t |\n", num_chat);
			printf("| Access Time : %s |\n", printTime(1));
			printf("| IP : %s\t\t\t |\n", ip);
			printf("------------------------------------------\n");

			if(num_chat == 1)
				printf("%dst client is added.\n\n", num_chat);
			else if(num_chat == 2)
				printf("%dnd client is added.\n\n", num_chat);
			else if(num_chat == 3)
				printf("%drd client is added.\n\n", num_chat);
			else
				printf("%dth client is added.\n\n", num_chat);

		}
		for(i=0; i<num_chat; i++){

			if(FD_ISSET(clisock_list[i], &read_fds) && num_chat >= MIN_ACCESS) {
				nbyte = recv(clisock_list[i], buf, MAXLINE_S, 0);
				
				if(nbyte <= 0){
					removeClient(i);
					continue;
				}
				buf[nbyte] = 0;
				if(strstr(buf, EXIT_STRING) != NULL){
					removeClient(i);
					continue;
				}

				char *get_time = printTime(1);
				printf("%s\n", get_time);

				for(j=0; j<num_chat; j++)
					send(clisock_list[j], buf, nbyte, 0);
				printf("%s\n", buf);

				keep_data(i, buf);
			}

			else if(FD_ISSET(clisock_list[i], &read_fds)){
				nbyte = recv(clisock_list[i], buf, MAXLINE_S, 0);
				if(nbyte <= 0){
					removeClient(i);
					continue;
				}
				buf[nbyte] = 0;
				if(strstr(buf, EXIT_STRING) != NULL){
					removeClient(i);
					continue;
				}
			}
		}
	}
}

char *printTime(int mode)
{
	char *get_time = '';

	struct tm tm = *localtime(&ti);
	if(mode == 1)
		sprintf(get_time, "%d.%2d.%d.%2d:%2d:%2d \t", 
			tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, 
			tm.tm_hour, tm.tm_min, tm.tm_sec);
	if(mode == 2)
		sprintf(get_time, "%2d:%2d", tm.tm_hour, tm.tm_min);

	return get_time;
}

void addClient(int s, struct sockaddr_in *newcliaddr)
{
	char buf[20];
	inet_ntop(AF_INET, &newcliaddr->sin_addr, buf, sizeof(buf));
	clisock_list[num_chat] = s;
	num_chat++;
}

void removeClient(int s)
{
	int j;
	close(clisock_list[s]);
	if(s != num_chat - 1)
		clisock_list[s] = clisock_list[num_chat - 1];
	num_chat--;
	printf("One of the client is out. Now client : %d\n", num_chat);
	if(num_chat < MIN_ACCESS){
		for(j=0; j<num_chat; j++)
			send(clisock_list[j], WAIT_STRING, strlen(WAIT_STRING), 0);
	}
}
int getmax()
{
	int max = listen_sock;
	int i;
	for (i=0; i<num_chat; i++)
		if(clisock_list[i] > max)
			max = clisock_list[i];
	return max;
}

int tcp_listen(int host, int port, int backlog)
{
	int sd;
	struct sockaddr_in servaddr;
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if(sd == -1){
		perror("socket fail");
		exit(1);
	}

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

void chat_client(char *id, char *pw, char *mode, char *ip_address)
{
	char bufall[MAXLINE_C + NAME_LEN];
	char *bufmsg;
	int maxfdp_1, s, namelen;
	fd_set read_fds;

	printf("-------------------------\n");
	printf("| ID : %s\t\t|\n", id);
	printf("| Hello, %s\t\t|\n", mode);
	printf("| IP : %s\t|\n", ip_address);
	printf("-------------------------\n");

	sprintf(bufall, "[%s] : ", id);
	namelen = strlen(bufall);
	bufmsg = bufall + namelen;

	s = tcp_connect(AF_INET, ip_address, random_port);
	if(s == -1)
		errquit("tcp_connect fail");

	puts("Server is connected.");
	maxfdp_1 = s + 1;
	FD_ZERO(&read_fds);
	
	while(1){
		FD_SET(0, &read_fds);
		FD_SET(s, &read_fds);

		if(select(maxfdp_1, &read_fds, NULL, NULL, NULL) < 0)
			errquit("select fail");

		if(FD_ISSET(s, &read_fds)){
			int nbyte;
			if((nbyte = recv(s, bufmsg, MAXLINE_C, 0)) > 0){
				bufmsg[nbyte] = 0;
				printf("%s \n", bufmsg);
			}
		}

		if(FD_ISSET(0, &read_fds)){
			if(fgets(bufmsg, MAXLINE_C, stdin)){
				if(send(s, bufall, namelen+strlen(bufmsg), 0) < 0)
					puts("Error : Write error on socket.");
				if(strstr(bufmsg, EXIT_STRING) != NULL) {
					puts("Good bye");
					close(s);
					exit(0);
				}

			}
		}
	}
}


void keep_data(int who, char *bufmsg)
{
	int fd = open("record.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);

	char save_data[MAXLINE_C] = {};

	int p = ':';
	char *ptr = strchr(bufmsg, p);

	int len = strlen(ptr) - 3;	// " : "
	char *get_time = printTime(2);

	sprintf(save_data, "%s(%d byte) \t\t=> %s\n\n", bufmsg, len, get_time);

	write(fd, save_data, sizeof(save_data));
	close(fd);
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
int main(int argc, char *argv[])
{
	Login root = {"root", "1234"};
	Login user[MAX_SOCK] = {};
	int i;

	char* mode = NULL;

	if(argc != 3){
		printf("Please INPUT ID & PASSWORD\n");
		exit(0);
	}
	char ip_address[20];

	if(strcmp(argv[1], root.id) == 0 && strcmp(argv[2], root.pw) == 0)
		mode = "server";

	else if(strlen(argv[1]) < MAX_ID && strlen(argv[2]) < MAX_PW)
	{
		if(strcmp(argv[1], root.id) == 0){
			printf("You can't use this ID. Use another ID\n");
			exit(1);
		}

		int fd = open("login.txt", O_RDWR | O_CREAT, 0644);
		if(fd == -1)
		{
			perror("failed open login file");
			exit(1);
		}

		i = 0;
		int re = 0;
		int wr = 0;

		while((re = read(fd, user, sizeof(user))) > 0)
		{
			if(strcmp(argv[1], user[i].id) == 0)
			{
				printf("This ID is already used.\n");
				exit(0);
			}
			i++;
		}
		
		printf("Press Your IP address : ");
		scanf("%s", ip_address);

		strcpy(user[i].id, argv[1]);
		strcpy(user[i].pw, argv[2]);


		wr = write(fd, &user, sizeof(user));
			
		mode = "client";
		close(fd);
	}
	else{
		printf("Your Input value is too long. Use short ID & PW\n");
		exit(1);
	}

	ip_address[sizeof(ip_address)] = 0;
	
	random_port = rand() % 10000 + 5000;

	if(mode == "server")	chat_server();
	else			chat_client(argv[1], argv[2], mode, ip_address);

	return 0;
}
