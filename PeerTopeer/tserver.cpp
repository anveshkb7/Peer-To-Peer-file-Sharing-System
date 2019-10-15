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
#include <fcntl.h>
#include <vector>

using namespace std;

struct file
{
	string name;
	string gid;
	string hashValue;
	int fileSize;
	string userID;
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
	f.userID = "";
	return f;
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
	// cout<<"Entered createUser function"<<endl;
	// cout<<"Parameters are :"<<username<<" and "<<password<<endl;
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
	// cout<<"Entered login function"<<endl;
	// cout<<"Parameters are :"<<username<<" and "<<password<<endl;
	map<string,string>::iterator it;
	
	for(it=user.begin(); it!=user.end(); it++)
	{
		if((it->first==username) && (it->second==password))
		{
			loggedIn[id]=1;
			connIdAndUser[to_string(connfd[id])] = username;
			fflush(stdout);
			write(connfd[id],"1",2);
			fflush(stdout);
			write(connfd[id],"User successfully logged in.\n",strlen("User successfully logged in.\n"));
			// cout<<"User successfully logged in"<<endl;
			return ;
		}
	}

	fflush(stdout);
	write(connfd[id],"0",2);
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
		fflush(stdout);
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
			// cout<<"Create User called\n";
			char user[255];
			char pass[255];
			read(connfd[*id],user,255);
			read(connfd[*id],pass,255);
			createNewUser(string(user),string(pass),*id);
		}
		else if(strcmp(buffer,"login")==0)
		{
			// cout<<"Login Function Called\n";
			char user[255];
			char pass[255];
			
			read(connfd[*id],user,255);
			read(connfd[*id],pass,255);

			loginUser(string(user),string(pass),*id);
		}
		else if(strcmp(buffer,"cg")==0)
		{
			// cout<<"Create Group Called\n";
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
						string gi="";
						gi+=string(groupId)+"-"+string(groupOwn)+"\n";
						char wr[50];
						strcpy(wr,gi.c_str());
						write(fd,wr,strlen(wr)+1);
						close(fd);

						char t[10];
						strcpy(t,groupOwn);
						int fd1=open(t,O_RDWR|O_CREAT|O_APPEND,0666);
						gi="";
						gi+=string(groupId)+"-"+string(groupOwn)+"\n";
						char wr1[50];
						strcpy(wr1,gi.c_str());
						write(fd1,wr1,strlen(wr1)+1);
						close(fd1);
					}
				else
					{
						fflush(stdout);
						write(connfd[*id],"Group already exists.\n",strlen("Group already exists.\n"));			
					}
			}
			else
			{
				fflush(stdout);
				write(connfd[*id],"You are not logged in.Please login first.\n",strlen("You are not logged in.Please login first.\n"));
				continue;
			}
		}
		else if(strcmp(buffer,"jg")==0)
		{
			if(loggedIn[*id]==1)
			{
				// cout<<"Join group called\n";
				char groupId[10];
				read(connfd[*id],groupId,10);
				// cout<<"request for : "<<groupId<<endl;
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
					// cout<<"Request is Successful\n";
					fflush(stdout);	
					write(connfd[*id],"Request is Pending.\n",strlen("Request is Pending.\n"));
				}
				else
				{
					fflush(stdout);
					write(connfd[*id],"Group does not exist.\n",strlen("Group does not exist.\n"));
				}	
			}
			else
			{
				fflush(stdout);
				write(connfd[*id],"You are not logged in.Please login first.\n",strlen("You are not logged in.Please login first.\n"));
				continue;
			}
			
		}
		else if(strcmp(buffer,"ar")==0)
		{
			if(loggedIn[*id]==1)
			{
				// cout<<"Accept Request is called\n";
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

				auto it4=group.find(to_string(connfd[*id]));
				string s1="";
				char go[50];
				strcpy(go,it4->second.c_str());
				s1+=string(groupId)+"-"+string(userId)+"\n";
				int fd=open(go,O_RDWR|O_CREAT|O_APPEND,0666);
				char wr1[50];
				strcpy(wr1,s1.c_str());
				write(fd,wr1,strlen(wr1)+1);
				fflush(stdout);
				write(connfd[*id],"Request is accepted.\n",strlen("Request is accepted.\n"));		
			}
			else
			{
				fflush(stdout);
				write(connfd[*id],"You are not logged in.Please login first.\n",strlen("You are not logged in.Please login first.\n"));
				continue;
			}
		}
		else if(strcmp(buffer,"lg")==0)
		{
			if(loggedIn[*id]==1)
			{
				char groupsList[2048];
				// cout<<"List groups called\n";
				// string numberOfGroups = to_string(noOfGroups);
				// // cout<<"total Groups in string are :"<<numberOfGroups<<endl;
				// write(connfd[*id],numberOfGroups.c_str(),strlen(numberOfGroups.c_str()));
				string gl="Following Groups are created \n";
				int flag=0;
				for(auto it=group.begin(); it!=group.end(); it++)
				{
					gl=gl+it->first+" "+it->second+"\n";
					flag=1;
				}
				if(flag==1)
				{
					bzero(groupsList,2048);
					strcpy(groupsList,gl.c_str());
					fflush(stdout);
					write(connfd[*id],groupsList,strlen(gl.c_str()));
				}
				else
				{
					fflush(stdout);
					write(connfd[*id],"No groups are created till now.\n",strlen("No groups are created till now.\n"));
				}
			}
			else
			{
				fflush(stdout);
				write(connfd[*id],"You are not logged in.Please login first.\n",strlen("You are not logged in.Please login first.\n"));
				continue;
			}
		}
		else if(strcmp(buffer,"lir")==0)
		{
			if(loggedIn[*id]==1)
			{	
				char requestListBuff[2048];
				char groupId[10];
				// cout<<"List request called\n";
				read(connfd[*id],groupId,10);
				// string numberOfGroups = to_string(noOfGroups);
				// // cout<<"total Groups in string are :"<<numberOfGroups<<endl;
				// write(connfd[*id],numberOfGroups.c_str(),strlen(numberOfGroups.c_str()));
				string gId(groupId);
				string rl="following users requseted for your group \n";

				int flag=0;
				for(auto it=requestList.begin(); it!=requestList.end(); it++)
				{
					if(it->first==gId)
					{
						rl=rl+it->second+"\n";
						flag=1;
					}
				}
				if(flag==1)
				{	
					bzero(requestListBuff,2048);
					strcpy(requestListBuff,rl.c_str());
					fflush(stdout);
					write(connfd[*id],requestListBuff,strlen(rl.c_str()));
				}
				else
				{
					fflush(stdout);
					write(connfd[*id],"There are no request present for your group.\n",strlen("There are no request present for your group.\n"));
				}
			}
			else
			{
				fflush(stdout);
				write(connfd[*id],"You are not logged in.Please login first.\n",strlen("You are not logged in.Please login first.\n"));
				continue;
			}
		}
		else if(strcmp(buffer,"leg")==0)
		{
			if(loggedIn[*id]==1)
			{
				// cout<<"Leave group called\n";
				char groupId[10];

				read(connfd[*id],groupId,10);
				string gId(groupId);
				auto it = connIdAndUser.find(to_string(connfd[*id]));

				vector<pair<string,string>>::iterator it1;
				int flag=0;
				for(it1=groupUsers.begin(); it1!=groupUsers.end(); it1++)
				{
					if(it1->first==gId && it1->second==it->second)
					{
						flag=1;
						break;
					}
				}
				if(flag==1)
				{
					groupUsers.erase(it1);
					fflush(stdout);
					write(connfd[*id],"You have successfully left the group.\n",strlen("You have successfully left the group.\n"));
				}
				else
				{
					fflush(stdout);
					write(connfd[*id],"You have not joined this group.\n",strlen("You have not joined this group.\n"));
				}
			}
			else
			{
				fflush(stdout);
				write(connfd[*id],"You are not logged in.Please login first.\n",strlen("You are not logged in.Please login first.\n"));
				continue;
			}

		}
		else if(strcmp(buffer,"uf")==0)
		{
			// cout<<"Upload file called\n";
			char fileInfoBuff[1024];
			read(connfd[*id],fileInfoBuff,1024);

			string fl(fileInfoBuff);
			// cout<<"Recieved string is "<<fl<<endl;
			fileTokenizer(fl);

			// cout<<"Tokens of file : "<<fileInfo[0]<<"-"<<fileInfo[1]<<"-"<<fileInfo[2]<<"-"<<fileInfo[3]<<endl;
			struct file f = newFile();
			// cout<<"struct object created";
			auto it = connIdAndUser.find(to_string(connfd[*id]));
			
			f.name=fileInfo[0];
			f.gid=fileInfo[1];
			f.hashValue=fileInfo[2];
			f.fileSize =stoi(fileInfo[3]);
			f.userID=it->second;
			
			files.push_back(f);
			// cout<<"File pushed successfully.\n";
			write(connfd[*id],"File is successfully uploaded.\n",strlen("File is successfully uploaded.\n"));

		}
		else if(strcmp(buffer,"df")==0)
		{
			char fileINFO[500];
			char gId[10];
			char fileName[100];
			char destPath[50];
			string fInfo = "";
			
			read(connfd[*id],gId,10);
			read(connfd[*id],fileName,100);
			
			string grid(gId);
			string fname(fileName);

			// Find userID of requesting user from connection id
			auto it = connIdAndUser.find(to_string(connfd[*id]));

			vector<pair<string,string>>::iterator it1;
			int flag=0;
			cout<<"Request for file :"<<fileName<<endl;
			
			// Check Whether requesting user belongs to that group
			for(it1=groupUsers.begin(); it1!=groupUsers.end(); it1++)
			{
				if(it1->first==grid && it1->second==it->second)
				{
					flag=1;
					break;
				}			
			}

			cout<<"file is requested by user "<<it->first<<endl;

			// if(flag==1)
			{
				vector<struct file>::iterator it4;
				// Find userID of user having the requested file
				for(it4=files.begin(); it4!=files.end(); it4++)
				{
					if((*it4).name==fname && (*it4).gid==grid)
						break;
				}
				fflush(stdout);
				// write(connfd[*id],"1",2);
				string fi ="";
				fi+= (*it4).userID;
				write(connfd[*id],fi.c_str(),strlen(fi.c_str()));
				// cout<<"File Owner is "<<(*it4).userID<<endl;
				// char fo[10]={0};
				// strcpy(fo,((*it4).userID).c_str());
				// cout<<"File Owner "<<fo<<endl;
				
				// // fflush(stdout);
				
				// write(connfd[*id],fo,strlen(fo));
				fflush(stdout);
				string fs="";
				fs=to_string((*it4).fileSize);
				
				
				write(connfd[*id],fs.c_str(),strlen(fs.c_str()));
				fflush(stdout);
				write(connfd[*id],((*it4).hashValue).c_str(),strlen(((*it4).hashValue).c_str()));
			}
			// else 
			// {
			// 	fflush(stdout);
			// 	write(connfd[*id],"0",2);
			// 	write(connfd[*id],"You are not part of the group.\n",strlen("You are not part of the group.\n"));
			// }
		}
		
		
		cout<<"Users logged in are : "<<endl;
		map<string,string>::iterator it;
		
		for(it=user.begin(); it!=user.end(); it++)
			cout<<it->first<<endl;

		// cout<<"Groups are : "<<endl;
		// map<string,string>::iterator it1;
		
		// for(auto it1=group.begin(); it1!=group.end(); it1++)
		// 	cout<<it1->first<<" "<<it1->second<<endl;

		cout<<"Request are : "<<endl;
		vector<pair<string,string>>::iterator it2;
		
		for(it2=requestList.begin(); it2!=requestList.end(); it2++)
			cout<<it2->first<<" : "<<it2->second<<endl;

		// cout<<"Connection id and users are : "<<endl;
		// map<string,string>::iterator it3;
		
		// for(it3=connIdAndUser.begin(); it3!=connIdAndUser.end(); it3++)
		// 	cout<<it3->first<<" : "<<it3->second<<endl;

		// cout<<"Groups and Users are : "<<endl;
		// vector<pair<string,string>>::iterator it5;
		// for(it5=groupUsers.begin(); it5!=groupUsers.end(); it5++)
		// 	cout<<it5->first<<" : "<<it5->second<<endl;

		cout<<"File details : \n";
		vector<struct file>::iterator it4;

		for(it4=files.begin(); it4!=files.end(); it4++)
			cout<<(*it4).name<<" "<<(*it4).userID<<endl;

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


	while(1)
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