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
#include<sys/sendfile.h>
using namespace std;

string command[100];
int command_index;
int connfd[100];
string userName="";
string groupOwner="";
pthread_t clientThread;
char destPath[50];

struct file
{
	string name;
	string gid;
	string hashValue;
	int fileSize;
	string userID;
};
struct file f;
struct file newFile()
{
	struct file f;
	f.name="";
	f.gid="";
	f.hashValue="";
	f.fileSize =0;
	f.userID = "";
	return f;
}
void error (const char *msg)
{
	perror (msg);
	exit(1);
}

string extractPortNumber(string userID)
{
	int fd=open("Tracker.txt",O_RDWR,0666);
	int flag=0;
	string u="";
	string p="";
	char c;
	while(1)
	{
		u="";
		p="";
		while(read(fd,&c,1)>0)
		{
			if(c!='-')
				u+=c;
			else
				break;
		}
		if(userID==u)
		{
			while(read(fd,&c,1)>0)
			{
				if(c!='\n')
					p+=c;
				else
					break;
			}
			break;
		}
		else
			while(read(fd,&c,1)>0)
				if(c=='\n')
					break;
	}
	// cout<<p<<endl;
	close(fd);
	return p;
}
void commandTokenizer(string input_command)
{
	int i,j,k=0;
	// cout<<"Received in tokenizer "<<input_command<<endl;
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
void *clientClient(void *arg)
{
	int sockfd , portno, n;
	struct sockaddr_in server_address;
	
	struct hostent *server;

	char recvBuffer[2048];
	char user[255];
	char pass[255];
	
	// cout<<"User id of Client requesting file is"<<f.userID<<endl;
	string p = extractPortNumber(f.userID);
	// cout<<"Port number in string  is : "<<p;
	// portno = atoi(p.c_str());
	
	portno = atoi(p.c_str());
	// cout<<"Port number in integer is : "<<p;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(sockfd < 0)
		error("ERROR opening socket");

	server = gethostbyname("127.0.0.1");
	if(server == NULL)
	{
		fprintf(stderr, "Error,no such hosts\n");
		pthread_exit(NULL);
	}

	bzero((char *)&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	bcopy((char *)server->h_addr,(char *)&server_address.sin_addr.s_addr,server->h_length);
	server_address.sin_port = htons(portno);

	if(connect(sockfd,(struct sockaddr *)&server_address,sizeof(server_address))<0)
	{
		error("Connection failed in Client\n");
		pthread_exit(NULL);
	}

	char fn[100];

	strcpy(fn,f.name.c_str());
	// cout<<"File to be copied is "<<t<<endl;
	
	send(sockfd,fn,100,0);
	
	int fs=0;
	
	
	recv(sockfd, &(fs), sizeof(int),0);
	// cout<<"Server sent filesize "<<size1<<endl;
	
	char filebuffer[fs];
	recv(sockfd,filebuffer,fs,0);
	
	int fd=open(destPath,O_RDWR|O_CREAT,0666);
	
	write(fd,filebuffer,fs);
	cout<<"File is Succefully Downloaded\n";
	close(fd);
	close(sockfd);

}
void *clientServer(void *arg)
{
	int i=0,ci=0;
	// int sockfd = 0;
	struct stat f;
	int fsize;
	
	// cout<<"Server of client called\n";
	int sockfd, newsockfd, portno,n;
	struct sockaddr_in server_address, client_address;
	socklen_t clilen;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(sockfd < 0)
	{
		error("Error opening socket");
	}

	bzero((char *)&server_address, sizeof(server_address));
	// cout<<"User id of Client requesting file is"<<userName<<endl;
	
	string p = extractPortNumber(userName);
	
	// cout<<"Port number in string  is : "<<p;
	portno = atoi(p.c_str());
	// cout<<"Port number in integer is : "<<p;

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(portno);	

	if(bind(sockfd , (struct sockaddr *) &server_address,sizeof(server_address)) < 0)
		error("Binding Failed in server of client");

	listen(sockfd, 5);
	clilen = sizeof(client_address);

	int conid = accept(sockfd, (struct sockaddr*)NULL, NULL); 
	char filePath[50];		
	recv(conid,filePath,50,0);
	cout<<"client sent file "<<filePath<<endl;
	
	stat(filePath,&f);
	fsize=f.st_size;
	
	cout<<"Size of file is "<<fsize<<endl;
	send(conid, &fsize, sizeof(int),0);
	int fd=open(filePath,O_RDWR,0666);
	cout<<"file opened \n";
	sendfile(conid,fd,NULL,fsize);
	cout<<"file Sent Succefully\n";
	close(fd);
	close(conid);
		
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
		error("Connection failed\n");

	while(1)
	{
		bzero(user,255);
		bzero(pass,255);
		bzero(recvBuffer,2048);
		// bzero(buffer,255);
		int i,j,k=0;
		string input_command="";
		fflush(stdin);
		// fflush(stdout);
		getline(cin,input_command);

		commandTokenizer(input_command);
		cout<<command[1]<<" "<<command[2]<<endl;
		
		if(command[0]=="create_user")
		{
			fflush(stdout);
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
			
			// string u&p = command[1]+" "+command[2];
			
			strcpy(user,command[1].c_str());
			strcpy(pass,command[2].c_str());
			
			write(sockfd,user,255);
			write(sockfd,pass,255);
			
			bzero(recvBuffer,255);
			read(sockfd,recvBuffer,255);
			// cout<<"Serever sent for login: "<<recvBuffer;
			if(strcmp(recvBuffer,"1")==0)
			{
				groupOwner = command[1];
				bzero(recvBuffer,255);
				read(sockfd,recvBuffer,255);
				printf("Server: %s",recvBuffer);
				pthread_t newServer;
				userName = command[1];
				// cout<<""<<userName<<endl;
				pthread_create(&newServer,NULL,&clientServer,NULL);
			}
			else
			{
				bzero(recvBuffer,255);
				read(sockfd,recvBuffer,255);
				printf("Server: %s",recvBuffer);	
			}
		}
		else if(command[0]=="create_group")
		{
			char groupOwn[255];
			char groupId[10];
			
			write(sockfd,"cg",3);
			// string groupInfo = command[1]+"-"+groupOwner;
			strcpy(groupOwn,groupOwner.c_str());
			strcpy(groupId,command[1].c_str());
			
			write(sockfd,groupId,10);
			write(sockfd,groupOwn,255);
			
			bzero(recvBuffer,255);
			read(sockfd,recvBuffer,255);
			printf("Server: %s",recvBuffer);
		}
		else if(command[0]=="join_group")
		{
			char groupId[10];
			
			write(sockfd,"jg",3);

			strcpy(groupId,command[1].c_str());

			write(sockfd,groupId,10);
			bzero(recvBuffer,255);
			read(sockfd,recvBuffer,255);
			printf("Server: %s", recvBuffer);

		}
		else if(command[0]=="accept_request")
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
		else if(command[0]=="list_groups")
		{
			char numberOfGroups[20];
			write(sockfd,"lg",3);
			
			bzero(recvBuffer,255);
			
			read(sockfd,recvBuffer,2048);
			
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
		else if(command[0]=="list_request")
		{
			char groupId[10];
			write(sockfd,"lir",4);
			
			strcpy(groupId,command[1].c_str());

			write(sockfd,groupId,10);
			
			bzero(recvBuffer,255);
			read(sockfd,recvBuffer,2048);
			printf("%s",recvBuffer);
		}
		else if(command[0]=="upload_file")
		{
			char file[20];
			char groupId[10];
			write(sockfd,"uf",3);
			

			strcpy(file,command[1].c_str());
			strcpy(groupId,command[2].c_str());
			
			cout<<"filename is "<<file<<endl;
			
			struct stat fileName;
			int fileSize;
			
			stat(file,&fileName);
			cout<<"Stat Executed\n";
			fileSize=fileName.st_size;
			int filesize = fileSize;
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
				hashValue.append(hsh);
			}
			
			cout<<"Hash is "<<hashValue<<endl;
			string fileInfo = "";
			fileInfo = command[1]+" "+command[2]+" "+ hashValue +" "+to_string(filesize);
			cout<<fileInfo<<endl;
			write(sockfd,fileInfo.c_str(),strlen(fileInfo.c_str()));

			bzero(recvBuffer,2048);
			read(sockfd,recvBuffer,255);
			printf("Server : %s",recvBuffer);
		}
		else if(command[0]=="download_file")
		{
			char gId[10];
			char fileName[50];
			
			
			char sha[100];
			char fileO[50];
			int sizeOfFile;

			char found[2];

			write(sockfd,"df",3);

			strcpy(gId,command[1].c_str());
			strcpy(fileName,command[2].c_str());
			strcpy(destPath,command[3].c_str());
			cout<<"destination Path is "<<destPath<<endl;

			write(sockfd,gId,10);
			write(sockfd,fileName,50);

			// read(sockfd,found,1);
			// cout<<"server sent "<<found<<endl;
			// if(strcmp(found,"1")==0)
			{
				bzero(fileO,50);
				read(sockfd,fileO,50);
				cout<<"file Owner is "<<fileO<<endl;
				read(sockfd,&sizeOfFile,sizeof(int));
				read(sockfd,sha,100);
				
				f = newFile();

				f.name=string(fileName);
				f.gid=string(gId);
				f.hashValue=string(sha);
				f.fileSize =sizeOfFile;
				f.userID=string(fileO);
				cout<<"file Owner is "<<f.userID<<endl;
				cout<<"file Info "<<f.name<<" "<<f.gid<<" "<<" "<<f.hashValue<<" "<<f.userID<<endl;
				pthread_create(&clientThread,NULL,&clientClient,NULL);
				pthread_join(clientThread,NULL);

			}
			// else
			// {
			// 	bzero(recvBuffer,2048);
			// 	read(sockfd,recvBuffer,255);
			// 	printf("Server : %s",recvBuffer);
			// }
			// fflush(stdin);
		}
		else if(command[0]=="logout")
		{
			write(sockfd,"lo",3);
			
			cout<<"You have Succefully logged out.\n";
			exit(0);		
		}
		else
		{
			cout<<"Enter a valid Command\n";
		}
	}
	return 0;
}