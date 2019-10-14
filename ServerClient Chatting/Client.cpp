#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;

void error (const char *msg)
{
	perror (msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	int sockfd , portno, n;
	struct sockaddr_in server_address;
	
	struct hostent *server;

	char buffer[255];
	
	if(argc < 3)
	{
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(1);
	}

	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(sockfd < 0)
		error("ERROR opening socket");

	server = gethostbyname(argv[1]);
	if(server == NULL)
	{
		fprintf(stderr, "Error,no such hosts\n");
	}

	bzero((char *)&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	bcopy((char *)server->h_addr,(char *)&server_address.sin_addr.s_addr,server->h_length);
	server_address.sin_port = htons(portno);

	if(connect(sockfd,(struct sockaddr *)&server_address,sizeof(server_address))<0)
		error("Connection failed");

	while(1)
	{
		bzero(buffer,255);
		fgets(buffer,255,stdin);
		n = write(sockfd,buffer,strlen(buffer));
		
		if(n<0)
			error("Error on writing");

		// bzero(buffer,255);
		// n = read(sockfd,buffer,255);
		// if(n<0)
		// 	error("Error on reading");
		// printf("Server: %s",buffer);

		// int i = strncmp("Bye",buffer,3);
		// if(i==0)
		// 	break;
	}

	close(sockfd);

	return 0;
}