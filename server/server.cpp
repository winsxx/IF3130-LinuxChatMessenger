/*
 * Tugas Besar 2 Jarkom
 * Compile: g++ server.cpp -pthread -o server
 */
 
#define SERVER_PORT 8888
#define MAX_CONNECTION_QUEUE 10
 
#include <cstdio>
#include <cstdlib>
 //Library socket
#include <sys/types.h>
#include <sys/socket.h>
//Library address
#include <netinet/in.h>
//Library string for bzero
#include <cstring>
//Library for close() so that it no longer refers to any file
#include <unistd.h>
//Library threading
#include <pthread.h>
#include <cstring>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
using namespace std;

vector<string> splitchar(char param[2000]) {
	cout << "masuk\n";
	vector<string> retval;
	string temp;
	stringstream test;
	test << param;
	while(getline(test,temp,' ')) {
		retval.push_back(temp);
	}
	return retval;
}

map<string,string> getUsernamePassword() {
	ifstream inputfile("databases/users.txt");
	map<string,string> un_pass;
	string line;
	while(getline(inputfile,line)) {
		char param[1000];
		strcpy(param,line.c_str());
		vector<string> temp = splitchar(param);
		un_pass.insert(make_pair(temp[0],temp[1]));
	}
	inputfile.close();
	return un_pass;
}

bool signup (string username, string password) {
	//kemungkinan: berhasil signup, gagal karena uda ada 
	map<string,string> un_pass = getUsernamePassword();

	map<string,string>::iterator it = un_pass.find(username);
	if(it == un_pass.end()) { //berarti username belum ada
		ofstream outputfile;
		outputfile.open("databases/users.txt");
		for(it = un_pass.begin();it!=un_pass.end();++it) {
			outputfile << it->first << " " << it->second << endl;
		}
		outputfile << username << " " << password << endl;
		outputfile.close();
		return true;
	} else {
		return false;
	}
}

bool login (string username, string password) {
	//kemungkinan: berhasil login, username ga ada, password salah.
	map<string,string> un_pass = getUsernamePassword();
	map<string,string>::iterator it = un_pass.find(username);
	if (it == un_pass.end()) { //berarti username belum ada
		return false;
	} else { //cek password
		return (it->second == password);
	}
}

void *connection_handler(void *);
map<string,int> logged_in_users;

int main(int argc, char *argv[]){
	
	/* Create socket
	 * int socket(int domain, int type, int protocol)
	 * AF_INET : domain untuk IPv4 Internet Protocol
	 * SOCK_STREAM: reliable, two-way
	 * IPPRORO_IP: mengembalikan protokol yang cocok TCP/UDP
	 * mengembalikan -1 jika error
	 */
	int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (serverSocket == -1){
		close(serverSocket);
		printf("Could not create socket\n");
		exit(1);
	}
	printf("Socket created\n");
	
	/* Create server address
	 * At port SERVER_PORT
	 */
	struct sockaddr_in serverAddress;
	bzero( (char *) &serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(SERVER_PORT);
	
	/* Set socket reuseable
	 * Allows other sockets to bind() to this port, unless there is an active listening socket bound to the port already.
	 */
	int option = 1;
	if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1){
		printf("Unable to set socket option to reuseable\n");
		exit(1);
	}
	printf("Done set socket reuseable\n");
	
	/* Binding
	 */
	if(bind(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) == -1){
		close(serverSocket);
		printf("Unable to bind address to socket\n");
		exit(1);
	}
	printf("Server address binded to socket\n");
	
	/* Listen at socket
	 * Maximum connection waiting at queue is MAX_CONNECTION_QUEUE
	 */
	if(listen(serverSocket, MAX_CONNECTION_QUEUE) == -1){
		printf("Fail to listen\n");
		exit(1);
	}
	printf("Server socket listening\n");
	
	
	printf("Waiting for incoming connection...\n");
	
	/* Accept connection
	 * int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
	 */
	int addrlen = sizeof(struct sockaddr_in);
	struct sockaddr_in clientAddress;
	pthread_t thread_id;
	while(true){
		int clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, (socklen_t*) &addrlen);
		if(clientSocket != -1){
			printf("Connection accepted with client socket %d\n",clientSocket);
			
			if(pthread_create(&thread_id, NULL, connection_handler, (void*) &clientSocket) == -1){
				printf("Could not create thread for %d\n",clientSocket);
				exit(1);
			}
			
		}
	}
	
	close(serverSocket);
	
	return 0;
 }

void *connection_handler(void *connectionSocket){
	int clientSocket = *(int*)connectionSocket;
	
	/*Mulai dari sini adalah contoh untuk dan akan dihapus 
	 *==================================================== 
	 * 
	 */
	int read_size;
	char *message, client_message[2000];

	while((read_size = recv(clientSocket, client_message , 2000 , 0)) > 0 ) {
        char* feedback;
		vector<string> input = splitchar(client_message);
		cout << input[0] << endl;
		if(input[0] == "signup") {
			if (input.size() != 3) {
				feedback = (char*) "Error format penulisan signup.\nFormat: signup [username] [password]\n";
			} else {
				bool signup_stat = signup(input[1], input[2]);
				if(signup_stat) {
					feedback = (char*) "Signup success!\n";
				} else {
					feedback = (char*) "Signup gagal karena username sudah ada\n";
				}
			}
		} else if (input[0] == "login") {
			if (input.size() != 3) {
				feedback = (char*) "Error format penulisan login.\nFormat: login [username] [password]\n";
			} else {
				bool login_stat = login(input[1], input[2]);
				if(login_stat) {
					feedback = (char*) "Login success\n";
					logged_in_users.insert(make_pair(input[1],clientSocket));
				} else {
					feedback = (char*) "Invalid username or password\n" ;
				}
			}
		} else if (input[0] == "message") {
			
		}
        write(clientSocket, feedback, strlen(feedback));
        free(message);
        memset(client_message,0,sizeof(client_message));
    }
	
	return 0;
	
}
