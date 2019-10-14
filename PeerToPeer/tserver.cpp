#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <map>
#include <string>
#include<fcntl.h>
#include <vector>

using namespace std;

struct file
{
	string name;
	string gid;
	string hashValue;
	int fileSize;
};

pthread_t thread[10];
int listenfd = 0, connfd[10] = {0};
int sockfd, newsockfd, portno,n;
char buffer[255];
struct sockaddr_in server_address, client_address;
socklen_t clilen;
int noOfGroups=0;

string fileInfo[100];
int fileIndex=0;

map<string,string> connIdAndUser;
map<string,string> user;
map<string,string> group;
vector<pair<string,string>> requestList;
vector<pair<string,string>> groupUsers;
vector<struct file > files;


struct file newFile()
{
	struct file f;
	f.name="";
	f.gid="";
	f.hashValue="";
	f.fileSize =0;
}

int loggedIn[100]={0};
void fileTokenizer(string file)
{
	int i,j,k=0;
	fileIndex=0;
	for(i=0; i<file.length(); )
	{
		string s="";
		for(j=i; j<file.length(); j++)
		{
			if(file[j]!=' ' && file[j]!='\n')
			{
				s=s+file[j];			
			}
			else
				break;
		}
		fileInfo[fileIndex++] = s;
		i=j+1;
	}
}
void createNewUser(string username,string password,int id)
{
	char suc[5] = "hi\n";
	map<string,string>::iterator it;
	cout<<"Entered createUser function"<<endl;
	cout<<"Parameters are :"<<username<<" and "<<password<<endl;
	// cout<<"fdsaasdfas\n";
	for(it=user.begin(); it!=user.end(); it++)
	{
		if(it->first==username)
		{
			fflush(stdout);
			write(connfd[id],"User already exists.\n",strlen("User already exists.\n"));
			return ;
		}
	}
	
	int fd=open("UserInfo.txt",O_RDWR|O_CREAT|O_APPEND,0666);
	
	write(fd,username.c_str(),username.length());
	write(fd,":",1);
	
	password[password.length()]='\n';
	password[password.length()+1]='\0';
	
	write(fd,password.c_str(),password.length()+1);
	close(fd);

	user[username]=password;
	fflush(stdout);
	write(connfd[id],"User successfully created.\n",strlen("User successfully created.\n"));
	// cout<<"User successfully created."<<endl;
}

void displayUsers(int id)
{
	cout<<"Users logged in are : "<<endl;
	// write(connfd[id],"Users logged in are : \n",strlen("Users logged in are : \n"));
	map<string,string>::iterator it;
	
	for(it=user.begin(); it!=user.end(); it++)
	{
		string str=it->first+"\n";
		char *s = (char*)str.c_str();
		// write(connfd[id],s,strlen(s));
		cout<<it->first<<endl;
	}
}

void loginUser(string username,string password,int id)
{
	cout<<"Entered login function"<<endl;
	cout<<"Parameters are :"<<username<<" and "<<password<<endl;
	map<string,string>::iterator it;
	
	for(it=user.begin(); it!=user.end(); it++)
	{
		if((it->first==username) && (it->second==password))
		{
			loggedIn[id]=1;
			connIdAndUser[to_string(connfd[id])] = username;
			fflush(stdout);
			write(connfd[id],"User successfully logged in.\n",strlen("User successfully logged in.\n"));
			// cout<<"User successfully logged in"<<endl;
			return ;
		}
	}

	fflush(stdout);
	write(connfd[id],"Login Failed!\n",strlen("Login Failed!\n"));
	// cout<<"Login Failed\n";
}

void error (const char *msg)
{
	perror (msg);
	exit(1);
}

void *readd(void *arg)
{
    int j;
    int *id=(int *)arg;
  	char* msg[50];
	while(1)
	{
		bzero(buffer , 255);
		n = read(connfd[*id],buffer,255);
		
		if(buffer[0]=='\0')
		{
			close(connfd[*id]);
			break;
		}
		
		printf("Client %d: %s\n",*id,buffer);
		
		if(strcmp(buffer,"cu")==0)
		{
			cout<<"Create User called\n";
			char user[255];
			char pass[255];
			read(connfd[*id],user,255);
			read(connfd[*id],pass,255);
			createNewUser(string(user),string(pass),*id);
		}
		else if(strcmp(buffer,"login")==0)
		{
			cout<<"Login Function Called\n";
			char user[255];
			char pass[255];
			
			read(connfd[*id],user,255);
			read(connfd[*id],pass,255);

			loginUser(string(user),string(pass),*id);
		}
		else if(strcmp(buffer,"cg")==0)
		{
			cout<<"Create Group Called\n";
			if(loggedIn[*id]==1)
			{
				char groupOwn[255];
				char groupId[10];
				
				read(connfd[*id],groupId,10);
				read(connfd[*id],groupOwn,255);

				auto it =group.find(string(groupId));
				if(it==group.end())
					{
						noOfGroups++;
						group[string(groupId)] = string(groupOwn);
						write(connfd[*id],"Group successfully created.\n",strlen("Group successfully created.\n"));
						groupUsers.push_back(make_pair(string(groupId),string(groupOwn)));
						int fd=open("groupInfo.txt",O_RDWR|O_CREAT|O_APPEND,0666);
	
						write(fd,groupId,strlen(groupId));
						write(fd,":",1);
						string grOwn(groupOwn);
						groupOwn[grOwn.length()]='\n';
						groupOwn[grOwn.length()+1]='\0';
						
						write(fd,groupOwn,strlen(groupOwn)+1);
						close(fd);
					}
				else
					{
						fflush(stdout);
						write(connfd[*id],"Group already exists.\n",strlen("Group already exists.\n"));			
					}
			}
		}
		else if(strcmp(buffer,"jg")==0)
		{
			// if(loggedIn[*id]==1)
			{
				cout<<"Join group called\n";
				char groupId[10];
				read(connfd[*id],groupId,10);
				cout<<"request for : "<<groupId<<endl;
				string gid(groupId);
				auto it = group.find(gid);
				// map<string,string>::iterator itj;
				if(it!=group.end())
				{
					// for(itj=connIdAndUser.begin(); itj!=connIdAndUser.end(); itj++)
					// 	if(itj->first==to_string(connfd[*id]))
					// 		break;
					char p[100]={0};
					strcpy(p,(it->second).c_str());
					auto it1=connIdAndUser.find(to_string(connfd[*id]));
					requestList.push_back(make_pair(it->first,it1->second));
					cout<<"Request is Successful\n";
					fflush(stdout);	
					write(connfd[*id],"Request is Pending.\n",strlen("Request is Pending.\n"));
				}
				else
				{
					fflush(stdout);
					write(connfd[*id],"Group does not exist.\n",strlen("Group does not exist.\n"));
				}	
			}
			
		}
		else if(strcmp(buffer,"ar")==0)
		{
			if(loggedIn[*id]==1)
			{
				cout<<"Accept Request is called\n";
				char groupId[10];
				char userId[10];

				read(connfd[*id],groupId,10);
				read(connfd[*id],userId,20);
				
				string uId(userId);
				string gId(groupId);

				vector<pair<string,string>>::iterator it;
				for(it=requestList.begin(); it!=requestList.end(); it++)
				{
					if(it->first==gId && it->second==uId)
						break;
				}

				
				map<string,string>::iterator it1;
			
				for(it1=connIdAndUser.begin(); it1!=connIdAndUser.end(); it1++)
				{
					if(it1->second==it->second)
					break;					
				}
				// int conID;
				// if(it1!=connIdAndUser.end())
				// 	conID = stoi(it1->first);
				groupUsers.push_back(make_pair(it->first,it->second));
				// cout<<"Connection id of requseted user is: "<<conID<<endl;
				requestList.erase(it);

				fflush(stdout);
				write(connfd[*id],"Request is accepted.\n",strlen("Request is accepted.\n"));		
			}
		}
		else if(strcmp(buffer,"lg")==0)
		{
			char groupsList[2048];
			cout<<"List groups called\n";
			// string numberOfGroups = to_string(noOfGroups);
			// // cout<<"total Groups in string are :"<<numberOfGroups<<endl;
			// write(connfd[*id],numberOfGroups.c_str(),strlen(numberOfGroups.c_str()));
			string gl="";
			
			for(auto it=group.begin(); it!=group.end(); it++)
			{
				gl=gl+it->first+" "+it->second+"\n";
			}
			bzero(groupsList,2048);
			strcpy(groupsList,gl.c_str());
			fflush(stdout);
			write(connfd[*id],groupsList,strlen(gl.c_str()));

		}
		else if(strcmp(buffer,"leg")==0)
		{
			cout<<"Leave group called\n";
			char groupId[10];

			read(connfd[*id],groupId,10);
			string gId(groupId);
			auto it = connIdAndUser.find(to_string(connfd[*id]));

			vector<pair<string,string>>::iterator it1;

			for(it1=groupUsers.begin(); it1!=groupUsers.end(); it1++)
			{
				if(it1->first==gId && it1->second==it->second)
					break;
			}

			groupUsers.erase(it1);

			write(connfd[*id],"You have successfully left the group.\n",strlen("You have successfully left the group.\n"));

		}
		else if(strcmp(buffer,"uf")==0)
		{
			cout<<"Upload file called\n";
			char fileInfoBuff[1024];
			read(connfd[*id],fileInfoBuff,1024);

			string fl(fileInfoBuff);
			cout<<"Recieved string is "<<fl<<endl;
			fileTokenizer(fl);

			cout<<"Tokens of file : "<<fileInfo[0]<<"-"<<fileInfo[1]<<"-"<<fileInfo[2]<<"-"<<fileInfo[3]<<endl;
			struct file f = newFile();

			f.name=fileInfo[0];
			f.gid=fileInfo[1];
			f.hashValue=fileInfo[2];
			f.fileSize =stoi(fileInfo[3]);
			
			files.push_back(f);

			write(connfd[*id],"File is successfully uploaded.\n",strlen("File is successfully uploaded.\n"));

		}

		
		
		cout<<"Users logged in are : "<<endl;
		map<string,string>::iterator it;
		
		for(it=user.begin(); it!=user.end(); it++)
			cout<<it->first<<endl;

		cout<<"Groups are : "<<endl;
		map<string,string>::iterator it1;
		
		for(auto it1=group.begin(); it1!=group.end(); it1++)
			cout<<it1->first<<" "<<it1->second<<endl;

		cout<<"Request are : "<<endl;
		vector<pair<string,string>>::iterator it2;
		
		for(it2=requestList.begin(); it2!=requestList.end(); it2++)
			cout<<it2->first<<" : "<<it2->second<<endl;

		cout<<"Connection id and users are : "<<endl;
		map<string,string>::iterator it3;
		
		for(it3=connIdAndUser.begin(); it3!=connIdAndUser.end(); it3++)
			cout<<it3->first<<" : "<<it3->second<<endl;

		cout<<"File details : \n";
		vector<struct file>::iterator it4;

		for(it4=files.begin(); it4!=files.end(); it4++)
			cout<<(*it4).name<<endl;

	}
}
int main(int argc, char *argv[])
{
	pthread_attr_t custom1,custom2;
	pthread_attr_init(&custom1);
	pthread_attr_init(&custom2);

	if(argc < 2)
	{
		fprintf(stderr,"Port no. not provided. Program terminated\n");
		exit(1);
	}
	
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
	int i=0;


	while(i<2)
	{
		connfd[i] = accept(sockfd,(struct sockaddr *)&client_address,&clilen);
		cout<<"Connection id of client is"<<connfd[i]<<endl;
		int *id=(int *)malloc(sizeof(int));
		id[0]=i;
	 	pthread_create(&thread[i],&custom1, readd,(void *)id);
	 	i++;
	}

	i=0;
	
	while(i<2)
	{
   	 	pthread_join(thread[i],NULL);
    	close(connfd[i]);	
	}
	close(sockfd);
	return 0;
}