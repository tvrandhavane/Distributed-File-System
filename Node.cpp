#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <stdlib.h>    
#include <cstring>

using namespace std;

vector<string> IPAddress;
vector<int>	port;
vector<string> folder;
int id;


bool storeFile(string md5sum, string userIP, int userPort){
	int sockfd;			//File descriptor of the TCP socket
	struct sockaddr_in dest_addr;	//This struct will store the address of the user
	int connectStatus;
	char buf[2048];

	//Create TCP socket
	sockfd = socket (PF_INET, SOCK_STREAM, 0);	//create UDP socket	
	if(sockfd == -1){							//check if udp socket is created or not.
		cout << "TCP socket could not be created." << endl;
		return -1;
	}

	//Populate the user address struct
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(userPort); // host byte order
	dest_addr.sin_addr.s_addr = inet_addr(userIP.c_str());	// network byte order
	memset(&(dest_addr.sin_zero), '\0', 8); // zero the rest of the struct

	//Connect to the user
	connectStatus = connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));
	if(connectStatus == -1){
		cout << "Connection to the user failed." << endl;
		close(sockfd);
		return -1;
	}
	//Sending File
	int recvStatus=4048;
	ofstream recvFile;
	//path of file where  recieve file is stored with name md5sum
	string path;
	path=folder[id];
	path+="/";
	path+=md5sum;
	//opening file of name md5sum to write the recieved data
	recvFile.open(path.c_str(),ios::out | ios::binary);
	while( (recvStatus=recv(sockfd,buf,2048,0))>0){
		recvFile.write(buf,recvStatus);
	}
	//closing file and socket
	recvFile.close();
	close(sockfd);
	return true;

}

bool retrieveFile(string md5sum, string userIP, int userPort){
	int sockfd;			//File descriptor of the TCP socket
	struct sockaddr_in dest_addr;	//This struct will store the address of the user
	int connectStatus;

	//Create TCP socket
	sockfd = socket (PF_INET, SOCK_STREAM, 0);	//create UDP socket	
	if(sockfd == -1){							//check if udp socket is created or not.
		cout << "TCP socket could not be created." << endl;
		return -1;
	}

	//Populate the user address struct
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(userPort); // host byte order
	dest_addr.sin_addr.s_addr = inet_addr(userIP.c_str());	// network byte order
	memset(&(dest_addr.sin_zero), '\0', 8); // zero the rest of the struct

	//Connect to the user
	connectStatus = connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));
	if(connectStatus == -1){
		cout << "Connection to the user failed." << endl;
		close(sockfd);
		return -1;
	}

	//This node sends the file to the user now
	//path of file to send named with md5sum
	string path="";
	path+=folder[id];
	path+="/";
	path+=md5sum;
	//open file 
  	std::ifstream file(path.c_str(), std::ifstream::binary);
			file.seekg(0, std::ifstream::beg);
			int n;
			while(file.tellg() != -1)
			{
			    char *p = new char[1024];

			    bzero(p, 1024);
			    file.read(p, 1024);
			    int long size=file.gcount();
			    printf("%ld\n", size);
			    //cout<<p<<endl;
			    n = send(sockfd, p, size, 0);
			    if (n < 0) {
			         cout<<"ERROR writing to socket"<<endl;
			    } else {
			         printf("---------%d\n", n);
			    }

			    delete p;
			}
			close(sockfd);
			file.close();

	return true;


}

int main(){

	int totalNodes;		//Number of total nodes in the mesh
	int sockfd;			//UDP socket file descriptor of this node
	int bindStatus;		//variable to check of binding of UDP socket to IP address and port was successful or not
	struct sockaddr_in my_addr;		//object to reference the elements of socket address

	//unsigned char buf[2048];
	int bufsize=2048;
	//Vectors to store information about nodes


	//Get the ID of this node
	cout << "Enter the node ID: " << endl;
	cin >> id;	

	//Read the configuration file
	string configFileName;
	cout << "Enter the name of configuration file: " << endl;
	cin >> configFileName;
	cout << "Reading configuration..." <<configFileName <<endl;

	ifstream myfile (configFileName.c_str(),ifstream::in);
	if (myfile.is_open()){

    	string line;
    	int newid = 0;
    	//getline(myfile, line);
    	while(getline(myfile, line)){
    		string newIPAddress = "";
    		string newPort = "";
    		string newFolder = "";

    		//read the IP address
    		cout<<line<<endl;
    		int i = 0;
    		while(line[i] != ':'){
    			newIPAddress+=line[i];
    			i++;
    		}

    		IPAddress.push_back(newIPAddress);
    		//read the port
    		i = i + 1;
    		while(line[i] != ' '){
    			newPort+=line[i];
    			i++;
    		}
    		port.push_back(atoi(newPort.c_str()));
    		//read the folder name
    		i = i + 1;
    		while(i<line.size()){
    			newFolder+=line[i];
    			i++;
    		}
    		
    		cout<<newid<<newPort<<endl;
    		folder.push_back(newFolder);
    		newid++;
    		//getline(myfile, line);
    	}
    	totalNodes = newid;
    }
    else{
    	cout<<"No Configuration file of name " << configFileName << " found" <<endl; 
    }
    myfile.close();
    cout << "Configuration successfully loaded." << endl;

    //Create a UDP port to listen on
    cout << "Creating an UDP socket..." << endl;
	sockfd = socket (PF_INET, SOCK_DGRAM, 0);	//create UDP socket	
	if(sockfd == -1){							//check if udp socket is created or not.
		cout << "UDP socket could not be created." << endl;
		return -1;
	}
	else{
		cout << "UDP socket successfully created." << endl;
	}

	//Bind the socket to IP address and port
	cout << "Binding the UDP socket to IP address and port..." << endl;
	
	my_addr.sin_family = AF_INET;	// host byte order
	my_addr.sin_port = htons(port[id]);// short, network byte order
	my_addr.sin_addr.s_addr = inet_addr(IPAddress[id].c_str());	//IP address of this host
	memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

	bindStatus = bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));

	if(bindStatus == -1){					//check if binding is successful or not.
		cout << "Binding failed." << endl;
		return -1;
	}
	else{
		cout << "Binding successful." << endl;
	}

	//Listen for a message on this UDP socket
	while(true){
		unsigned char buf[bufsize];	//buffer to read the information from
		struct sockaddr_in from;	//Store the address of the sender in this variable
		

		cout<<"In Recieving"<<endl;
		socklen_t fromlen = sizeof(from);	//length of the address stored in from

		int recvStatus = recvfrom(sockfd, buf, bufsize, 0,(struct sockaddr *)&from, &fromlen);	//Receive a message

		if(recvStatus == -1){						//if received message had an error
			cout << "Message error" << endl;
			return -1;
		}
		else{										//Message received successfully.
			cout << "Received " << recvStatus << " Bytes from " << inet_ntoa(from.sin_addr) << endl;
			string userIP = "";
			string userPort = "";
			string type = "";
			string md5sum = "";

			//read request md5sum

			int i = 0;
			
			while(buf[i] != ':'){
					type+=buf[i];
					i++;
			}
			i = i + 1;	
			while(buf[i] != ':'){
				md5sum+=(buf[i]);
				i++;
			}

			//Compute the id of the node which stores this file
			int md5int=atoi(md5sum.c_str()) ;
			int targetNode = md5int % totalNodes;

			if(targetNode == id){	//If current node is the target node then perform 
									//the required task according to the type of the request
				//read file type
				cout<<"id "<<id<<"targetNode " <<targetNode<<"Total "<<totalNodes<<endl;

				//read IP address
				i = i + 1;
				while(buf[i] != ':'){
					userIP+=buf[i];
					i++;
				}

				//read port
				i = i + 1;
				while(buf[i] != '\n'){
					userPort+=buf[i];
					i++;
				}

				if(type == "s"){

					storeFile(md5sum, userIP, atoi(userPort.c_str()));
				}
				else if(type == "g"){
					retrieveFile(md5sum, userIP, atoi(userPort.c_str()));
				}


			}			
			else{
				//forward this message to the target node
				struct sockaddr_in to;		//object to reference the elements of target node address
				socklen_t tolen = sizeof(to);
				to.sin_family = AF_INET;	// host byte order
				to.sin_port = htons(port[targetNode]);// short, network byte order
				to.sin_addr.s_addr = inet_addr(IPAddress[targetNode].c_str());	//IP address of the target node
				memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

				int sendStatus = sendto(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&to, tolen);
				if(sendStatus == -1){						//if sent message had an error
					cout << "Message error" << endl;
					return -1;
				}
				else{										//Message sent successfully.
					cout << "Sent " << sendStatus << " Bytes." << endl;
				}
			}
		}
	}
	return -1;
}