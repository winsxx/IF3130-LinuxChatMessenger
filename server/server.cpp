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

typedef struct {
	string receiver;
	map<string,vector<string> > sender;
} message_format;

vector<message_format> unread;

string getDateTime() {
	time_t t = time(0);   // get time now
    struct tm * now = localtime(& t );
    stringstream retval ;
   	retval << "[" << (now->tm_year + 1900) << "-" << (now->tm_mon + 1) << "-" << (now->tm_mday) << " " << (now->tm_hour) << ":" << ((now->tm_min) < 10 ? "0" : "" )<< (now->tm_min) << "]";
	return retval.str();
}

void printUnread() {
	for(int i = 0; i < unread.size(); ++i) {
		cout << (i+1) << ". receiver: " << unread[i].receiver << endl;
		for(map<string,vector<string> >::iterator it1 = unread[i].sender.begin(); it1 != unread[i].sender.end(); ++it1) {
			cout << "\tsender_name: " << it1 -> first << endl;
			for (int j = 0 ; j < (it1->second).size();++j) {
				cout << "\tcontent: " << (it1->second)[j] << endl;
			}
		}
	}
}

void inputMessage(string from, string to, string message) {
	cout << "from " << from << " to " << to << " isinya " << message << endl;
	int idx = -1;
	for(int i = 0; i < unread.size(); ++i) {
		if (unread[i].receiver == to) {
			idx = i;
			break;
		}
	}
	cout << "idx " << idx << endl;
	if (idx != -1) {
		// map<string, vector<string> > temp_map = unread[idx].sender;
		map<string, vector<string> >::iterator map_it = unread[idx].sender.find(from);
		if(map_it != unread[idx].sender.end()) { //tambahkan aja ke vector, karena receiver uda ada, sender juga ada
			vector<string> vec_tmp = map_it -> second;
			vec_tmp.push_back(getDateTime() + " " + from + " : " +  message);
			cout << "Isinya\n";
			for (int x = 0; x < vec_tmp.size(); ++x) {
				cout << vec_tmp[x] << endl;
			}
			map_it->second = vec_tmp;
			for (int x = 0; x < (map_it->second).size(); ++x) {
				cout << (map_it->second)[x] << endl;
			}
			// unread[idx].sender = *map_it;
		} else { //ada receiver, tp ga ada sender, makanya perlu tambah sender
			vector<string> vec_tmp;
			vec_tmp.push_back(getDateTime() + " " + from + " : " +  message);
			unread[idx].sender.insert(make_pair(from,vec_tmp));
		}
	} else {
		vector<string> vec_tmp;
		vec_tmp.push_back(getDateTime() + " " + from + " : " +  message);
		
		map<string,vector<string> > map_tmp;
		map_tmp.insert(make_pair(from,vec_tmp));

		message_format mf;
		mf.receiver = to;
		mf.sender = map_tmp;
		unread.push_back(mf);
	}
}

void loadUnreadMessageFromDatabase() {
	ifstream inputfile("databases/unread.txt");
	int num_of_receiver;
	inputfile >> num_of_receiver;
	//cout << "num of receiver: " << num_of_receiver << " " << endl;
	for (int i = 1; i <= num_of_receiver; ++i) {
		message_format temp;
		inputfile >> temp.receiver;
		//cout << "receiver : " << temp.receiver << endl;
		int num_of_sender;
		inputfile >> num_of_sender;
		//cout << "\tnum of sender : " << num_of_sender << endl;
		map<string,vector<string> > temp_map;

		for (int j = 1; j <= num_of_sender; ++j) {
			string sender_name;
			int num_of_message;
			inputfile >> sender_name;
			//cout << "\t\tsender_name " << sender_name << endl;
			inputfile >> num_of_message;
			//cout << "\t\tnum_of_message " << num_of_message << endl;
			vector<string> array_of_msg;
			for (int k = 1; k <= num_of_message; ++k) {
				string message_tmp;
				getline(inputfile, message_tmp);
				getline(inputfile, message_tmp);
				//cout << "\t\t\tmessage_tmp " << message_tmp << endl;
				array_of_msg.push_back(message_tmp);
			}
			temp_map.insert(make_pair(sender_name,array_of_msg));
		}

		temp.sender = temp_map;
		unread.push_back(temp);
	}
}

vector<string> splitchar(char param[2000]) {
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

bool create_group(string group_name) {
	return true;
}

bool is_username_exits(string username) {
	map<string,string> un_pass = getUsernamePassword();
	map<string,string>::iterator it = un_pass.find(username);
	return (it != un_pass.end());
}

void *connection_handler(void *);
map<int,string> logged_in_users;

int main(int argc, char *argv[]){
	loadUnreadMessageFromDatabase();
	printUnread();
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

	while((read_size = recv(clientSocket, client_message, 2000, 0)) > 0 ) {
        char* feedback;
		vector<string> input = splitchar(client_message);
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
					map<int,string>::iterator ite;
					
					ite = logged_in_users.find(clientSocket);
					if(ite != logged_in_users.end()) logged_in_users.erase(ite);
					
					logged_in_users.insert(make_pair(clientSocket,input[1]));
					cout << "User yang sedang login dan socket\n";
					for(ite =  logged_in_users.begin(); ite!=logged_in_users.end(); ++ite) {
						cout << ite->first << " " << ite->second << endl;
					}
				} else {
					feedback = (char*) "Invalid username or password\n" ;
				}
			}
		} else if (input[0] == "create") { //belum jadi
			if (input.size() != 2) {
				feedback = (char*) "Error format create group.\nFormat: create [group_name]\n";
			} else {
				bool create_group_stat = create_group(input[1]);
			}
		} else if (input[0] == "message") {
				map<int,string>::iterator ite;
				ite = logged_in_users.find(clientSocket);
				if (ite == logged_in_users.end()) { //berarti belum log in
					feedback = (char*) "Anda belum login!\n";
				} else {
					cout << "Masuk yang uda log in" << endl;
					if (input.size() != 2) {
						feedback = (char*) "Error format message user/group.\nFormat: message [user_name/group_name]\n";
					} else {
						bool un_exists = is_username_exits(input[1]);
						if(un_exists) {
							if (ite -> second == input[1]) {
								cout << "Kirim ke sendiri" << endl;
								feedback = (char*) "Error tidak boleh kirim pesan ke diri sendiri\n";
							} else {
								string receiver = input[1];
								string sender = ite -> second;
								cout << "Kirim ke orang lain" << endl;
								feedback = (char*) "Message: ";
								write(clientSocket, feedback, strlen(feedback));
								memset(client_message,0,sizeof(client_message));
								read_size = recv(clientSocket, client_message, 2000, 0);
								cout << "Pesannya: " << client_message << endl;
								feedback = (char*) "Message sent.";
								inputMessage(sender,receiver,client_message);
								printUnread();
							}
						} else {
							feedback = (char*) (input[1] + " doesn't exist").c_str();
						}
					}
					//read_size = recv(clientSocket, client_message, 2000, 0);
				}
		}
        write(clientSocket, feedback, strlen(feedback));
        free(message);
        memset(client_message,0,sizeof(client_message));
    }
	
	return 0;
	
}
