/*
	This program takes command 4 command line arguments (Space seperated).
	1)	"S" or "G" to specify whether send a store or get request respectively
	2)	address of a location which will specify a file in case you need to store
		the file on the FileMesh, or a folder in case you need to retrieve some file.
	3) 	which node in the mesh to contact
	4)  Location of FileMesh.cfg
*/

/*Libraries needed*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdio.h>
#include "openssl/md5.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <vector>
#include <climits>
#include <map>

using namespace std;

/*Decalration of store(int, int). Definition follows after main*/
bool store(int mf, int node);

/*Decalration of get(int, int). Definition follows after main*/
bool get(int mf, int node);

/*typedef of triple for reading filemesh*/
typedef pair < string, pair <int, string> > triple;

/*Details of config file will be stored in this.*/
vector < triple > config;

/*Map of filename to md5sum will be stored here*/
map<string , int> md5map;

/*GLOBAL variable storing the "address of file (store)" | "address of folder (get)" */
string address;


/*Function to get the md5 sum given file address in filename*/
int md5(const char * filename){
	int max = INT_MAX/2.0;
	unsigned char c[MD5_DIGEST_LENGTH];
	int i;
	FILE *inFile = fopen (filename, "rb");
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];

	if (inFile == NULL) {
		printf ("%s can't be opened.\n", filename);
		return 0;
	}
	MD5_Init (&mdContext);
	while ((bytes = fread (data, 1, 1024, inFile)) != 0)
		MD5_Update (&mdContext, data, bytes);
	MD5_Final (c,&mdContext);
	int md5=0;
	for(i = 0; i < MD5_DIGEST_LENGTH; i++) {
		md5 = (md5+c[i])%max;    	
	}
	/*md5sum returned as an integer*/
	return md5;
}


// Main function taking 4 arguments. For more details, refer to the topmost comment

int main(int argc, char* argv[]){
	string a, b, item;
	/*Restoring any previously sent files along with their md5sum, so that the user
	doesn't have to give the md5sum everytime*/
	ifstream file("filenameToMdsum");
	if(file.is_open()){
		while(file>>a>>b){
			md5map[a]=atoi(b.c_str());
		}
		file.close();
	}

	/*FileMesh Config file is read in the following snippet*/
	//////////////////////////////////////////////////////////////////////////////////
	ifstream file1(argv[4]);
	triple p;
	if(file1.is_open()){
		cout<<"reading fileMesh"<<endl;
		while(file1>>a>>b){
			p.second.second = b;
			stringstream ss(a);
			getline(ss, item, ':');
			p.first=item;
			cout<<item << " "<<p.first<<endl;
			getline(ss, item, ':');
			p.second.first=atoi(item.c_str());
			config.push_back(p);
		}
		
	}
	else{
		cout<<"not reading fileMesh"<<endl;
	}
	//////////////////////////////////////////////////////////////////////////////////

	bool done = false;
	/*Instruction: g for get, s for store*/
	string instr=argv[1];
	address=argv[2];
	int mf;
	string filename;
	int node = atoi(argv[3]);
	while(true){
		/*Check instruction type*/
		if(instr.compare("s")==0 || instr.compare("S") == 0){
		//	STORE
			string item;
			stringstream ss(address);
			while(getline(ss, item, '/'))
				filename = item;

			/*Calculating the md5sum by function declared earlier*/
			mf = md5(address.c_str());

			md5map[filename]=mf;
			cout<<"store"<<endl;
			// cout<<filename<<endl;
			// cout<<md5map.size()<<endl;

			/*Calling the store function with md5code and nodenumber*/
			store(mf, node);

			/*Following code snippet updates the filename to md5sum matching
			in file filenameToMdsum on the disk*/
			////////////////////////////////////////////////////////////////////////
			ofstream outputFile("filenameToMdsum");
			map<string , int>::iterator it;
			for(it = md5map.begin() ; it != md5map.end() ; it++)
				outputFile<<it->first<<" "<<it->second<<endl;
			outputFile.close();
			////////////////////////////////////////////////////////////////////////

			break;
		}
		else if(instr.compare("g")==0 || instr.compare("G") == 0){
		//	GET
			cout<<"get"<<endl;

			/*In case you have the md5sum of the file*/
			cout<<"Do you have the md5sum of the file (y/n)"<<endl;
			cin>>instr;
			while(!done){

				if(instr.compare("y")==0 || instr.compare("Y")==0){
					/*In case he does have the md5sum*/
					cout<<"Then enter it !"<<endl;
					cin>>mf;
					/*Calling get with the md5sum and nodenumber*/
					get(mf, node);
					break;
				}
				else if(instr.compare("n")==0 || instr.compare("N")==0){
					/*In case he doesn't have the md5sum*/
					cout<<"Then enter the filename which you wan't to retrieve"<<endl;
					cout<<"We'll try to retrieve its md5sum from our database"<<endl;
					cout<<"NOTE: ITS POSSIBLE THAT THE FILENAME IS NOT FOUND,"<<endl;
					cout<<"OR THERE ARE MULTIPLE FILES BY THAT NAME, IN WHICH"<<endl;
					cout<<"CASE THE PROGRAM WILL EXIT"<<endl;
					cin>>filename;
					/*Searching in the filename->md5match for filename*/
					map<string, int>::iterator it = md5map.find(filename);
					if(it == md5map.end()){
						/*File not found, no way to go forward with the transaction now*/
						cout<<"FILENAME NOT FOUND!, EXITING . . .";
						return 0;
					}
					else{
						/*File found, retrieving the md5sum and calling get*/
						cout<<"FILENAME FOUND ! GETTING .  .";
						mf = md5map[filename];
					}
					/*Calling get with the md5sum and nodenumber*/
					get(mf, node);
					break;
				}
				else{
					/*Invalid input other than 'y/n'*/
					cout<<"INVALID INPUT, TRY AGAIN"<<endl;
					cout<<"Do you have the md5sum of the file (y/n)"<<endl;
					cin>>instr;

				}
				break;
			}
			break;
		}
		else{
			cout<<"INVALID OPERATION CODE, TRY AGAIN"<<endl;
			cout<<"\"S\" or \"G\" to specify whether send a store or get request respectively"<<endl;
			cout<<"What is the operation desired ?"<<endl;
			cin>>instr;
		}
	}
	return 0;
}

/*Store function*/
bool store(int mf, int node){
	int myport = 5000;
	int bufsize = 2048;
	int destPort = 5050;
	/*CHANGE THIS IP TO THE USER'S MACHINES IP ADDRESS*/
	string myIpAddr = "10.13.50.144";
	int myTcpPort = 5002;
	//Create socket
	int sockfd,sockfdtcp ;	//ID of the udp socket and tcp socket
	sockfd = socket (PF_INET, SOCK_DGRAM, 0);	//create UDP socket	
	if(sockfd == -1){							//check if udp socket is created or not.
		cout << "UDP socket could not be created." << endl;
		return -1;
	}

	//Bind the socket to IP address and port
	int bindStatus;	//variable to check of binding was successful or not

	struct sockaddr_in my_addr;		//object to reference the elements of socket address
	my_addr.sin_family = AF_INET;	// host byte order
	my_addr.sin_port = htons(myport);// short, network byte order
	my_addr.sin_addr.s_addr = inet_addr(myIpAddr.c_str());	//IP address of this host
	memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

	bindStatus = bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));

	if(bindStatus == -1){					//check if binding is successful or not.
		cout << "Binding failed." << endl;
		close(sockfd);
		return -1;
	}

	//create a TCP socket to store/receive the file
	sockfdtcp = socket (PF_INET, SOCK_STREAM, 0);	//create UDP socket	
	if(sockfdtcp == -1){							//check if udp socket is created or not.
		cout << "TCP socket could not be created." << endl;
		return -1;
	}

	// modifying port number of the TCP socket
	/*ASSIGNING THE TCP SOCKET RANDOM PORT NUMBER*/
	my_addr.sin_port = htons(0);// short, network byte order 
	//Bind the socket to IP address and port
	bindStatus = bind(sockfdtcp, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));

	if(bindStatus == -1){					//check if binding is successful or not.
		cout << "TCP Binding failed." << endl;
		close(sockfdtcp);
		return -1;
	}
	else{
		cout << "TCP Binding successful." << endl;
	}
	/*RETRIEVING THE RANDOMLY ASSIGNED PORT NUMBER TO BE SENT TO THE NODE*/
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	getsockname(sockfdtcp, (struct sockaddr *)&sin, &len);
	myTcpPort = ntohs(sin.sin_port);





	//Send a message on THE UDP socket
	char msg[50];
	//cout<<"in store1"<<endl;
	sprintf(msg, "s:%d:%s:%d",mf, myIpAddr.c_str(), myTcpPort);
	//cout<<"in store2"<<endl;

	struct sockaddr_in to;		//object to reference the elements of destination socket address
	socklen_t tolen = sizeof(to);
	to.sin_family = AF_INET;	// host byte order
	to.sin_port = htons(config[node].second.first);// short, network byte order
	to.sin_addr.s_addr = inet_addr(config[node].first.c_str());	//IP address of the destination
	memset(&(to.sin_zero), '\0', 8); // zero the rest of the struct

	cout<<config[node].second.first << " " <<config[node].first.c_str() <<endl;
	int sendStatus = sendto(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&to, tolen);
	if(sendStatus == -1){						//if received message had an error
		cout << "Message error" << endl;
		return -1;
	}
	else{										//Message received successfully.
		cout << "Sent " << sendStatus << " Bytes." << endl;
	}
	int close_tatus=close(sockfd);


	//Listen for the TCP packet
	listen(sockfdtcp, 10);		//Maximum 10 connections
	struct sockaddr_in their_addr;	//Connector's address
	socklen_t sin_size = sizeof(struct sockaddr_in);
	int new_fd = accept(sockfdtcp, (struct sockaddr *)&their_addr, &sin_size);		//Accept the connection

	if(new_fd == -1){
		cout << "Connection to the node failed. Closing..." << endl;
		return -1;
	}
	std::ifstream file(address.c_str(), std::ifstream::binary);

	file.seekg(0, std::ifstream::beg);
	int n;
	while(file.tellg() != -1)
	{
		char *p = new char[1024];

		bzero(p, 1024);
		file.read(p, 1024);
		int long size=file.gcount();
		printf("%ld\n", size);

		n = send(new_fd, p, size, 0);
		if (n < 0) {
			cout<<"ERROR writing to socket"<<endl;
		} else {
			printf("---------%d\n", n);
		}

		delete p;
	}
	close(new_fd);
	close(sockfdtcp);
	file.close();

	return true;
}

bool get(int mf, int node){
	int myport = 5000;
	int bufsize = 2048;
	int destPort = 5050;
	string myIpAddr = "10.13.50.144";
	int myTcpPort = 5002;
	
	//Create socket
	int sockfd,sockfdtcp;	//ID of the udp socket
	sockfd = socket (PF_INET, SOCK_DGRAM, 0);	//create UDP socket	
	if(sockfd == -1){							//check if udp socket is created or not.
		cout << "UDP socket could not be created." << endl;
		return -1;
	}

	//Bind the socket to IP address and port
	int bindStatus;	//variable to check of binding was successful or not

	struct sockaddr_in my_addr;		//object to reference the elements of socket address
	my_addr.sin_family = AF_INET;	// host byte order
	my_addr.sin_port = htons(myport);// short, network byte order
	my_addr.sin_addr.s_addr = inet_addr(myIpAddr.c_str());	//IP address of this host
	memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

	bindStatus = bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));

	if(bindStatus == -1){					//check if binding is successful or not.
		cout << "Binding failed." << endl;
		close(sockfd);
		return -1;
	}

	//create a TCP socket to store/receive the file
	sockfdtcp = socket (PF_INET, SOCK_STREAM, 0);	//create UDP socket	
	if(sockfdtcp == -1){							//check if udp socket is created or not.
		cout << "TCP socket could not be created." << endl;
		return -1;
	}

	/*ASSIGNING A RANDOM TCP PORT TO THE SOCKET*/
	my_addr.sin_port = htons(0);// short, network byte order
	//Bind the socket to IP address and port
	bindStatus = bind(sockfdtcp, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));

	if(bindStatus == -1){					//check if binding is successful or not.
		cout << "TCP Binding failed." << endl;
		close(sockfdtcp);
		return -1;
	}
	else{
		cout << "TCP Binding successful." << endl;
	}

	/*retrieving the randomly assigned port number*/
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	getsockname(sockfdtcp, (struct sockaddr *)&sin, &len);
	myTcpPort = ntohs(sin.sin_port);
	//Send a message on this UDP socket



	//Send a message on this UDP socket
	char msg[50];
	sprintf(msg, "g:%d:%s:%d", mf, myIpAddr.c_str(), myTcpPort);
	struct sockaddr_in to;		//object to reference the elements of destination socket address
	socklen_t tolen = sizeof(to);
	to.sin_family = AF_INET;	// host byte order
	to.sin_port = htons(config[node].second.first);// short, network byte order
	to.sin_addr.s_addr = inet_addr(config[node].first.c_str());	//IP address of the destination
	memset(&(to.sin_zero), '\0', 8); // zero the rest of the struct


	int sendStatus = sendto(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&to, tolen);
	if(sendStatus == -1){						//if received message had an error
		cout << "Message error" << endl;
		return -1;
	}
	else{										//Message received successfully.
		cout << "Sent " << sendStatus << " Bytes." << endl;
	}
	// close the UDP socket
	close(sockfd);

	//Listen for the TCP packet
	listen(sockfdtcp, 10);		//Maximum 10 connections

	struct sockaddr_in their_addr;	//Connector's address
	socklen_t sin_size = sizeof(struct sockaddr_in);
	int new_fd = accept(sockfdtcp, (struct sockaddr *)&their_addr, &sin_size);		//Accept the connection

	if(new_fd == -1){
		cout << "Connection to the node failed. Closing..." << endl;
		return -1;
	}
	/*Now retrieve the file from the node connected*/
	
	char buf[2048];
	int recvStatus=4048;
	ofstream recvFile;
	//getting path of file where  recieve file is stored with name md5sum
	string path="";
	path+=address;
	path+="/";
	char str[100];
	sprintf(str, "%i", (int)mf);
	path+=string(str);
	cout<<path<<endl;

	//opening file of name md5sum to write the recieved data
	recvFile.open(path.c_str(),ios::out | ios::binary);
	while( (recvStatus=recv(new_fd,buf,2048,0))>0){
		recvFile.write(buf,recvStatus);
	}
	//closing file and socket
	close(sockfdtcp);
	close(new_fd);
	recvFile.close();

	return true;;
}
