#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>

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

	// FILE *fp;

	// int ch = 0;
	// fp = fopen("as.txt","w");
	// int words=0;

	// read(newsockfd, &words, sizeof(int));
	// cout<<"Words in file are "<<words<<endl;
	// char c1;
	// while(ch != words)
	// {
	// 	read(newsockfd,buffer,255);
	// 	fprintf(fp,"%s",buffer);
	// 	ch++;
	// }

	int fd = open("as.txt",O_WRONLY | O_CREAT | O_TRUNC);
    ssize_t rd ;  
    while(1) {
        rd= read(newsockfd, buffer, 255);
        if (rd == 0)
            break;
        
        int wr = write(fd, buffer, rd);
        if (wr <255)
            break;
    } 

	// FILE *fp;
	// read(sockfd,buffer,255);
	// fp=fopen("s.txt","w");
	// fprintf(fp,"%s",buffer);

	printf("File has been successfully copied\n");
	close(newsockfd);
	close(sockfd);
	return 0;


}