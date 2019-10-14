#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sstream>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <openssl/sha.h>
using namespace std;
string command[100];
int command_index;

void error (const char *msg)
{
	perror (msg);
	exit(1);
}

/*
// void *clientRead (void *id)
// {
// 	while(1)
// 	{
// 		bzero(buffer , 255);
// 		n = read(newsockfd,buffer,255);
// 		if(n<0)
// 			error("Error on reading");

// 		 printf("Client : %s\n",buffer);
// 		// bzero(buffer,255);
// 		// fgets(buffer,255,stdin);

// 		// n=write(newsockfd,buffer,strlen(buffer));
// 		// if(n<0)
// 		// 	error("Error on writing");
		
// 		// int i = strncmp("Bye",buffer,3);
// 		// if(i==0)
// 		// 	break;
// 	}
// }
*/
void commandTokenizer(string input_command)
{
	int i,j,k=0;
	command_index=0;
	for(i=0; i<input_command.length(); )
	{
		string s="";
		for(j=i; j<input_command.length(); j++)
		{
			if(input_command[j]!=' ' && input_command[j]!='\n')
			{
				s=s+input_command[j];			
			}
			else
				break;
		}
		command[command_index++] = s;
		i=j+1;
	}
}

int main(int argc, char *argv[])
{
	// pthread_attr_t custom1,custom2,custom3;
	// pthread_attr_init(&custom1);
	// pthread_attr_init(&custom2);
	// // pthread_attr_init(&custom3);

	int sockfd , portno, n;
	struct sockaddr_in server_address;
	
	struct hostent *server;

	string groupOwner="";
	char recvBuffer[2048];
	char user[255];
	char pass[255];
	
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
		bzero(user,255);
		bzero(pass,255);
		bzero(recvBuffer,2048);
		// bzero(buffer,255);
		int i,j,k=0;
		string input_command="";
		fflush(stdin);
		getline(cin,input_command);

		commandTokenizer(input_command);
		
		if(command[0]=="cu")
		{
			write(sockfd,"cu",3);
			
			// string u&p = command[1]+" "+command[2];
			
			strcpy(user,command[1].c_str());
			strcpy(pass,command[2].c_str());
			
			write(sockfd,user,255);
			write(sockfd,pass,255);

			read(sockfd,recvBuffer,255);
			printf("Server: %s",recvBuffer);

		}
		else if(command[0]=="login")
		{
			write(sockfd,"login",6);
			groupOwner = command[1];
			// string u&p = command[1]+" "+command[2];
			
			strcpy(user,command[1].c_str());
			strcpy(pass,command[2].c_str());
			
			write(sockfd,user,255);
			write(sockfd,pass,255);

			read(sockfd,recvBuffer,255);
			printf("Server: %s",recvBuffer);	
		}
		else if(command[0]=="cg")
		{
			char groupOwn[255];
			char groupId[10];
			write(sockfd,"cg",3);
			// string groupInfo = command[1]+"-"+groupOwner;
			strcpy(groupOwn,groupOwner.c_str());
			strcpy(groupId,command[1].c_str());
			
			write(sockfd,groupId,10);
			write(sockfd,groupOwn,255);
			
			read(sockfd,recvBuffer,255);
			printf("Server: %s",recvBuffer);
		}
		else if(command[0]=="jg")
		{
			char groupId[10];
			write(sockfd,"jg",3);

			strcpy(groupId,command[1].c_str());
			write(sockfd,groupId,10);
			bzero(recvBuffer,255);
			read(sockfd,recvBuffer,255);
			printf("Server: %s", recvBuffer);

		}
		else if(command[0]=="ar")
		{
			char groupId[10];
			char userID[20];
			write(sockfd,"ar",3);

			strcpy(groupId,command[1].c_str());
			strcpy(userID,command[2].c_str());
			
			write(sockfd,groupId,10);
			write(sockfd,userID,20);
			
			bzero(recvBuffer,255);
			read(sockfd,recvBuffer,255);
			printf("Server : %s",recvBuffer);
		}
		else if(command[0]=="lg")
		{
			char numberOfGroups[20];
			write(sockfd,"lg",3);
			// fflush(stdout);
			// read(sockfd,numberOfGroups,20);
			// cout<<"No of groups are :"<<numberOfGroups<<endl;
			// string num(numberOfGroups);
			// int numOfGroups = stoi(num);
			bzero(recvBuffer,255);
			cout<<"Groups are :\n";
			read(sockfd,recvBuffer,20);
			// cout<<endl;
			printf("%s",recvBuffer);

		}
		else if(command[0]=="leave_group")
		{
			char groupId[10];
			write(sockfd,"leg",5);

			strcpy(groupId,command[1].c_str());
						
			write(sockfd,groupId,10);

			bzero(recvBuffer,2048);
			read(sockfd,recvBuffer,255);
			printf("Server : %s",recvBuffer);

		}
		else if(command[0]=="list_groups")
		{

		}
		else if(command[0]=="uf")
		{
			char file[20];
			char groupId[10],h[3];
			write(sockfd,"uf",3);
			

			strcpy(file,command[1].c_str());
			strcpy(groupId,command[2].c_str());
			cout<<"filename is "<<file<<endl;
			struct stat fileName;
			int fileSize;
			
			stat(file,&fileName);
			cout<<"Stat Executed\n";
			fileSize=fileName.st_size;
			int files = fileSize;
			fileSize=min(fileSize,20);

			// char shaBuff[fileSize];

 			unsigned char *filebuff;
 			char hsh[3];
 			filebuff = (unsigned char *)malloc (fileSize);
			int fd = open(file,O_RDWR,0666);
			cout<<"File opened Succefully.\n";
			
			read(fd,filebuff,fileSize);
			cout<<"File Read.\n";
			cout<<filebuff<<endl;
			close(fd);
			
			string hashValue = "";
			
			unsigned char shaBuff[SHA_DIGEST_LENGTH];
			const unsigned char *f=filebuff;

			SHA1(f, fileSize, shaBuff);
			cout<<"hash computed"<<endl;
			
			for(int i = 0; i < SHA_DIGEST_LENGTH; i++)
			{
				sprintf(hsh,"%02x",shaBuff[i]);
				// string k(hsh);
				// hashValue+=k;
				hashValue.append(hsh);
			}
			
			cout<<"Hash is "<<hashValue<<endl;
			string fileInfo = "";
			fileInfo = command[1]+" "+command[2]+" "+ hashValue +" "+to_string(files);
			cout<<fileInfo<<endl;
			write(sockfd,fileInfo.c_str(),strlen(fileInfo.c_str()));

			bzero(recvBuffer,2048);
			read(sockfd,recvBuffer,255);
			printf("Server : %s",recvBuffer);
		}
		else
		{
			cout<<"Enter a valid Command\n";
		}
	}
		

		// n = write(sockfd,buffer,strlen(buffer));
		
		// if(n<0)
		// {
		// 	error("Error on writing");
		// 	break;
		// }

		// bzero(recvBuffer,255);
		// n = read(sockfd,recvBuffer,255);
		
		// if(n<0)
		// 	error("Error on reading");
		// printf("Server: %s",recvBuffer);

	

	// close(sockfd);
	return 0;
}