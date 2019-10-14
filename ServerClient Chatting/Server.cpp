#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

using namespace std;

void error (const char *msg)
{
	perror (msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		fprintf(stderr,"Port no. not provided. Program terminated\n");
		exit(1);
	}
	
	int sockfd, newsockfd, portno,n;
	char buffer[255];

	struct sockaddr_in server_address, client_address;
	socklen_t clilen;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(sockfd < 0)
	{
		error("Error opening socket");
	}

	bzero((char *)&server_address, sizeof(server_address));
	portno = atoi(argv[1]);

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(portno);	

	if(bind(sockfd , (struct sockaddr *) &server_address,sizeof(server_address)) < 0)
		error("Binding Failed");

	listen(sockfd, 5);
	clilen = sizeof(client_address);

	newsockfd = accept(sockfd,(struct sockaddr *)&client_address,&clilen);

	if(newsockfd < 0)
		error("Error on Accept");

	while(1)
	{
		bzero(buffer , 255);
		n = read(newsockfd,buffer,255);
		if(n<0)
			error("Error on reading");

		 printf("Client : %s\n",buffer);
		// bzero(buffer,255);
		// fgets(buffer,255,stdin);

		// n=write(newsockfd,buffer,strlen(buffer));
		// if(n<0)
		// 	error("Error on writing");
		
		// int i = strncmp("Bye",buffer,3);
		// if(i==0)
		// 	break;
	}

	close(newsockfd);
	close(sockfd);
	return 0;


}