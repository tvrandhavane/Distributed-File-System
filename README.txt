Computer Networks - LAB-09
Socket Programming Lab: Distributed File System

Tanmay Randhavane 	110050010
Alok Yadav			110050043
Ved Ratn Dixit 		110050044

* List of relevant files:
1)	Node.cpp		- For the nodes in the file mesh
2) 	User.cpp		- For the end user of the file mesh
3) 	FileMesh.cfg 	- Configuration of the mesh used by the nodes

* Compilation instructions:
1)	To compile on the end user's system
g++ User.cpp -o User -lssl -lcrypto

(This needs openssl library to run, to install openssl - sudo apt-get install libssl-dev)

2) To compile on the Node's system
g++ Node.cpp -o Node

* Configuration file(s):
	Format of the configuration file:
	IP_Address:PortNumber Folder_path

	Example configuration file (FileMesh.cfg) is included.

* Running instructions:
1) On the User side: 
./User {S/G} {file_address/folder_address} node_id {config_file_address}

This executable runs the client side of the application, client can store or get the file using it.

This program takes 3 command line arguments
	a)	"S" or "G" to specify whether send a store or get request respectively
	b)	address of a location which will specify a file in case you need to store
	the file on the FileMesh, or a folder in case you need to retrieve some file.
	c) 	which node in the mesh to contact
	d)	address of conifiguration file of the nodes in the file mesh

2) On the node side:
./Node

This executable runs the nodes in the file mesh, when a client request arrives, it stores  a file or sends the file or forwards the request to the correct node according to the type of the requst.