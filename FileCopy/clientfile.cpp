#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

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

	bzero(buffer,255);

	// FILE *f;
	// int words = 0;

	// char c;

	// f= fopen("a.txt","r");

	// while((c = getc(f)) != EOF)
	// {
	// 	cout<<buffer<<endl;
	// 	fscanf(f,"%s",buffer);
		
	// 	if(isspace(c) || c=='\t' || c=='\n')
	// 		words++;
	// }
	// words++;
	// cout<<"No of words are : "<<words<<endl;
	// write(sockfd,&words,sizeof(int));
	// rewind(f);

	// char ch,ch1;
	// while(ch != EOF)
	// {
	// 	fscanf(f,"%s",buffer);
	// 	write(sockfd, buffer ,255);
	// 	ch = fgetc(f);
	// }


	int fd = open("a.txt", O_RDONLY);

	while (1) 
	{
        int rd = read(fd, buffer, 255);
        if (rd == 0)
            break;
        cout<<buffer<<endl;
        int wr = write(sockfd, buffer, rd) ;
        if (wr < 255)
            break;
    }
    

	// FILE *f;
	// f=fopen("a.txt","r");
	// fscanf(f,"%s",buffer);
	// write(sockfd,buffer,255);

	printf("The File Has been successfully sent\n");

	close(sockfd);
	// close(fd);

	return 0;
}