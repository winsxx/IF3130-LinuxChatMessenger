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

/*
 * Daftar mutex
 */
pthread_mutex_t mutex_user_data;
pthread_mutex_t mutex_group_data;
pthread_mutex_t mutex_unread;

typedef struct {
	string receiver;
	map<string,vector<string> > sender;
} message_format;

vector<message_format> unread;

bool is_username_exists(string username);

string getNotifications(string username) {
	//Lock
	pthread_mutex_lock (&mutex_unread);
			
	string retval = "";
	vector<string> messageList;

	int num_of_receiver = unread.size();
	for (int i = 1; i <= num_of_receiver; ++i) {
		message_format temp = unread[i-1];
		if (temp.receiver == username) {
			int num_of_sender = temp.sender.size();
			for (map<string,vector<string> >::iterator it1 = temp.sender.begin(); it1 != temp.sender.end(); ++it1) {
				retval += ("New message(s) from " + it1 -> first + "\n");
			}
			break;
		}
	}
	//UnLock
	pthread_mutex_unlock (&mutex_unread);
			
	return retval;
}

void saveUnreadMessageToDatabase();

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

vector<string> readMessage(string from, string to) {
	//Lock
	pthread_mutex_lock (&mutex_unread);
	
	vector<string> retval;
	int idx = -1;
	for(int i = 0; i < unread.size(); ++i) {
		if (unread[i].receiver == to) {
			idx = i;
			break;
		}
	}

	if (idx == -1){
		//UnLock
		pthread_mutex_unlock (&mutex_unread);
		return retval; //berarti ga ada pesan yang masuk ke dia
	}
	else {
		map<string, vector<string> >::iterator map_it = unread[idx].sender.find(from);
		if (map_it == unread[idx].sender.end()) { //berarti ga ada pesan yang dari sender ke dia
			//UnLock
			pthread_mutex_unlock (&mutex_unread);
			return retval;
		} else {
			retval = map_it -> second;
			//setelah itu hapus dari database
			unread[idx].sender.erase(map_it);
			if (unread[idx].sender.size() == 0) { //buang diri sendiri yang receiver tanpa sender itu loh
				unread.erase(unread.begin()+idx);
			}
			saveUnreadMessageToDatabase();
			//UnLock
			pthread_mutex_unlock (&mutex_unread);
			return retval;
		}
	}
	
}

bool findSenderReceiverConversationInUnread(string sender, string receiver) {
	//Lock
	pthread_mutex_lock (&mutex_unread);
	
	int idx = -1;
	for(int i = 0; i < unread.size(); ++i) {
		if (unread[i].receiver == receiver) {
			idx = i;
			break;
		}
	}
	if (idx == -1) {
		//UnLock
		pthread_mutex_unlock (&mutex_unread);
		return false;
	} else {
		map<string, vector<string> >::iterator map_it = unread[idx].sender.find(sender);
		if (map_it == unread[idx].sender.end()){ //kalau ketemu, return true
			//UnLock
			pthread_mutex_unlock (&mutex_unread);
			
			return false;
		} else {
			//UnLock
			pthread_mutex_unlock (&mutex_unread);
			
			return ( (map_it->second).size() > 0);
		}
	}	
}

void inputMessage(string from, string to, string message, bool himself, string header_jam, string another_because_of_group = "") {
	//Lock
	pthread_mutex_lock (&mutex_unread);
	
	int idx = -1;
	for(int i = 0; i < unread.size(); ++i) {
		if (unread[i].receiver == to) {
			idx = i;
			break;
		}
	}
	if (idx != -1) {
		// map<string, vector<string> > temp_map = unread[idx].sender;
		map<string, vector<string> >::iterator map_it = unread[idx].sender.find(from);
		if(map_it != unread[idx].sender.end()) { //tambahkan aja ke vector, karena receiver uda ada, sender juga ada
			vector<string> vec_tmp = map_it -> second;
			vec_tmp.push_back(header_jam + " " + (another_because_of_group != "" ? another_because_of_group : (himself ? to : from)) + " : " +  message);
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
			vec_tmp.push_back(header_jam + " " + (another_because_of_group != "" ? another_because_of_group : (himself ? to : from)) + " : " +  message);
			unread[idx].sender.insert(make_pair(from,vec_tmp));
		}
	} else {
		vector<string> vec_tmp;
		vec_tmp.push_back(header_jam + " " + (another_because_of_group != "" ? another_because_of_group : (himself ? to : from)) + " : " +  message);
		
		map<string,vector<string> > map_tmp;
		map_tmp.insert(make_pair(from,vec_tmp));

		message_format mf;
		mf.receiver = to;
		mf.sender = map_tmp;
		unread.push_back(mf);
	}
	saveUnreadMessageToDatabase();
	//UnLock
	pthread_mutex_unlock (&mutex_unread);
}

void loadUnreadMessageFromDatabase() {
	ifstream inputfile("databases/unread.txt");
	int num_of_receiver;
	unread.clear();
	inputfile >> num_of_receiver;
	for (int i = 1; i <= num_of_receiver; ++i) {
		message_format temp;
		inputfile >> temp.receiver;
		int num_of_sender;
		inputfile >> num_of_sender;
		map<string,vector<string> > temp_map;

		for (int j = 1; j <= num_of_sender; ++j) {
			string sender_name;
			int num_of_message;
			inputfile >> sender_name;
			inputfile >> num_of_message;
			vector<string> array_of_msg;
			string message_tmp;
			getline(inputfile, message_tmp); //simpan enter
			for (int k = 1; k <= num_of_message; ++k) {
				message_tmp="";
				getline(inputfile, message_tmp);
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
	//Lock
	pthread_mutex_lock (&mutex_user_data);
	
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
	
	//Unlock
	pthread_mutex_unlock (&mutex_user_data);
	
	return un_pass;
}

bool iscontain (vector<string> data, string test){
	for (int i = 0 ; i < data.size(); ++i){
		if(data.at(i)==test)
			return true;
	}
	return false;
}

vector<string> getGroupName() {
	//Lock 
	pthread_mutex_lock(&mutex_group_data);
	
	ifstream inputfile("databases/group.txt");
	vector<string> groupname;
	string line;
	while(getline(inputfile, line)) {
		char param[1000];
		strcpy(param,line.c_str());
		vector<string> temp = splitchar(param);
		 if(!iscontain(groupname, temp[0]))
			groupname.push_back(temp[0]);
	}
	inputfile.close();
	
	//Unlock 
	pthread_mutex_unlock(&mutex_group_data);
	
	return groupname;
}

bool isGroupMember(string group_name, string username) {
	//Lock 
	pthread_mutex_lock(&mutex_group_data);
	
	ifstream inputfile("databases/group.txt");
	string line;
	while(getline(inputfile, line)) {
		char param[1000];
		strcpy(param, line.c_str());
		vector<string> temp = splitchar(param);
		if(group_name==temp[0] && username==temp[1]){
			//Unlock 
			inputfile.close();
			pthread_mutex_unlock(&mutex_group_data);
			return true;
		}
	}
	
	//Unlock 
	inputfile.close();
	pthread_mutex_unlock(&mutex_group_data);
	
	return false;
}

void log(string log_content) {
	//Input file
	ifstream inputfile("databases/log.txt");
	string line;
	vector<string> temp;
	while(getline(inputfile, line)) {
		temp.push_back(line);
	}
	//Output file
	ofstream outputfile;
	outputfile.open("databases/log.txt");
	for (int i = 0 ; i < temp.size(); ++i) {
		outputfile << temp.at(i) << endl;
	}
	outputfile << log_content << endl;
	outputfile.close();
}

bool isGroupExist(string group_name)
{
	//Lock 
	pthread_mutex_lock(&mutex_group_data);
	
	ifstream inputfile("databases/group.txt");
	string line;
	while(getline(inputfile, line)) {
		char param[1000];
		strcpy(param, line.c_str());
		vector<string> temp = splitchar(param);
		if(group_name==temp[0]){
			//Unlock 
			inputfile.close();
			pthread_mutex_unlock(&mutex_group_data);
			return true;
		}
	}
	//Unlock 
	inputfile.close();
	pthread_mutex_unlock(&mutex_group_data);
	return false;
}


vector<string> getGroupMember(string group_name) {
	vector<string> v;
	if(!isGroupExist(group_name)) {

	} else {
		ifstream inputfile("databases/group.txt");
		string line;
		while(getline(inputfile, line)) {
			char param[1000];
			strcpy(param, line.c_str());
			vector<string> temp = splitchar(param);
			if(group_name==temp[0]){
				v.push_back(temp[1]);
			}
		}
		inputfile.close();
	}
	return v;
}

void inputMessageGroup(string from, string to, string message, bool himself, string header_jam){
	vector<string> recipient = getGroupMember(to);
	for(vector<string>::iterator itv = recipient.begin(); itv != recipient.end(); ++itv) {
		if(*itv != from) {
			inputMessage(to, *itv, message, false, header_jam, from);
		}
	}
}

bool create_group(string group_name, string nama)
{
	/*
	 * Yang ini gw masih bingung karena masih mungkin terjadi race condition
	 */


	//kemungkinan: berhasil bikin, gagal karena udah ada
	if(isGroupExist(group_name) || is_username_exists(group_name)) //group udah ada
		return false;
	else //group belum ada
	{
		//Lock 
		pthread_mutex_lock(&mutex_group_data);
		//Input file
		ifstream inputfile("databases/group.txt");
		string line;
		vector<string> temp;
		while(getline(inputfile, line)) {
			temp.push_back(line);
		}
		//Output file
		ofstream outputfile;
		outputfile.open("databases/group.txt");
		for (int i = 0 ; i < temp.size(); ++i)
    		{
			outputfile << temp.at(i) << endl;
		}
		outputfile << group_name << " " << nama;
		outputfile.close();
		
		//Unock 
		pthread_mutex_unlock(&mutex_group_data);
		return true;
	}
	
}

bool join_group(string group_name, string nama)
{
	if(!iscontain(getGroupName(), group_name) || isGroupMember(group_name,nama))
	{
		return false;
	}
	else
	{
		//Lock 
		pthread_mutex_lock(&mutex_group_data);
		
		ifstream inputfile("databases/group.txt");
		string line;
		vector<string> temp;
		while(getline(inputfile, line))
			temp.push_back(line);
		inputfile.close();

		ofstream outputfile;
		outputfile.open("databases/group.txt");
		 for (int i = 0 ; i < temp.size(); ++i)
  		{
		 	outputfile << temp.at(i) << endl;
		}
		outputfile << group_name << " " << nama;
		outputfile.close();
		//Unock 
		pthread_mutex_unlock(&mutex_group_data);
		return true;
	}
}

bool leave_group(string group_name, string nama)
{
	if(!isGroupMember(group_name, nama))
		return false;
	else
	{
		//Lock 
		pthread_mutex_lock(&mutex_group_data);
		
		ifstream inputfile("databases/group.txt");
		string line;
		vector<string> ret;
		while(getline(inputfile, line))
		{
			char param[1000];
			strcpy(param,line.c_str());
			vector<string> temp = splitchar(param);
			if(!(temp[0]==group_name && temp[1]==nama))
				ret.push_back(line);
		}
		inputfile.close();
		
		ofstream outputfile;
		outputfile.open("databases/group.txt");
		 for (int i = 0 ; i < ret.size(); ++i)
  		{
		 	outputfile << ret[i] << endl;
		}
		outputfile.close();
		//Unlock 
		pthread_mutex_unlock(&mutex_group_data);
		return true;
	}
}

bool signup (string username, string password) {
	//kemungkinan: berhasil signup, gagal karena uda ada 
	map<string,string> un_pass = getUsernamePassword();

	map<string,string>::iterator it = un_pass.find(username);
	if(it == un_pass.end()) { //berarti username belum ada
		ofstream outputfile;
		
		//Lock
		pthread_mutex_lock (&mutex_user_data);
		
		outputfile.open("databases/users.txt");
		for(it = un_pass.begin();it!=un_pass.end();++it) {
			outputfile << it->first << " " << it->second << endl;
		}
		outputfile << username << " " << password << endl;
		outputfile.close();
		//Unlock
		pthread_mutex_unlock (&mutex_user_data);
		
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

int recvStringFrom(int socketId, char* server_reply){
	string msg="";
	int code;
	int cum = 0;
	char from_server[2];
	
	//Baca dari server per byte sampai ketemu null character
	while(code = recv(socketId, from_server, 1, 0) > 0){
		if(from_server[0] == '\0'){
			break;
		}else{
			msg += from_server[0];
			++cum;
		}
	}
	
	//Masukkan sebuah string message dari server atau string setengah jadi
	server_reply = strcpy(server_reply,msg.c_str());
	server_reply[msg.length()] = '\0';
	
	if(code<=0) //kode error atau client tutup
		return code;
	else 
		return cum; //banyaknya karakter
}

void *connection_handler(void *);
map<int,string> logged_in_users;

int main(int argc, char *argv[]){
	loadUnreadMessageFromDatabase();
	printUnread();
	saveUnreadMessageToDatabase();
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
	
	/*
	 * Initialize mutex
	 */
	pthread_mutex_init(&mutex_user_data, NULL);
	pthread_mutex_init(&mutex_group_data, NULL);
	pthread_mutex_init(&mutex_unread, NULL);
	
	/* Accept connection
	 * int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
	 */
	int addrlen = sizeof(struct sockaddr_in);
	struct sockaddr_in clientAddress;
	pthread_t thread_id;
	log(getDateTime() + " Server starts");
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
	/*
	 * Destroy mutex
	 */
	pthread_mutex_destroy(&mutex_user_data); 
	pthread_mutex_destroy(&mutex_group_data); 
	pthread_mutex_destroy(&mutex_unread); 
	
	return 0;
 }

void *connection_handler(void *connectionSocket){
	int clientSocket = *(int*)connectionSocket;

	int read_size;
	char *message, client_message[2000];

	while((read_size = recvStringFrom(clientSocket, client_message)) > 0 ) {
        char* feedback;
		vector<string> input = splitchar(client_message);

		if(input[0] == "signup") {
			if (input.size() != 3) {
				feedback = (char*) "Error format penulisan signup.\nFormat: signup [username] [password]\n";
			} else {
				bool signup_stat = signup(input[1], input[2]);
				if(signup_stat) {
					feedback = (char*) "Signup success!\n";
					log(getDateTime() + " " + input[1] + " signed up");
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
					log(getDateTime() + " " + input[1] + " logged in");
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
		} else if(input[0] == "logout") {
			map<int,string>::iterator ite;
			ite = logged_in_users.find(clientSocket);
			if (ite == logged_in_users.end()) { //berarti belum log in
				feedback = (char*) "Anda belum login!\n";
			} else if (input.size() != 1){
				feedback = (char*) "Error format penulisan logout.\nFormat: logout\n";
			} else {
				log(getDateTime() + " " + ite->second + " logged out");
				logged_in_users.erase(ite);
				feedback = (char*) "Anda berhasil logout\n";
			}
			
		} else if (input[0] == "create") {
			map<int,string>::iterator ite;
			ite = logged_in_users.find(clientSocket);
			if (ite == logged_in_users.end()) { //berarti belum log in
				feedback = (char*) "Anda belum login!\n";
			} else {
				if(input.size() != 2) {
					feedback = (char*) "Error format penulisan login.\nFormat: create [nama group]\n";
				}
				else{
					string group_name = input[1];
					if(create_group(group_name, ite -> second)) {
						log(getDateTime() + " " + ite->second + " created group " + group_name);
						feedback = (char*) ("Group " + group_name + " created\n").c_str();
					}	
					else
						feedback = (char*) ("Cannot create " + group_name +"\n").c_str();
				}
			}

		} else if(input[0] == "join") {
			map<int,string>::iterator ite;
			ite = logged_in_users.find(clientSocket);
			if (ite == logged_in_users.end()) { //berarti belum log in
				feedback = (char*) "Anda belum login!\n";
			} else {
				if(input.size() != 2) {
					feedback = (char*) "Error format penulisan login.\nFormat: join [nama group]\n";
				}
				else {
					if(join_group(input[1], ite -> second)) {
						log(getDateTime() + " " + ite->second + " joined group " + input[1]);
						feedback = (char*) ("Joining group " + input[1] +"\n").c_str();
					}
					else
						feedback = (char*) ("Can't join this group\n");
				}
			}

		} else if(input[0] == "leave") {
			map<int,string>::iterator ite;
			ite = logged_in_users.find(clientSocket);
			if (ite == logged_in_users.end()) { //berarti belum log in
				feedback = (char*) "Anda belum login!\n";
			} else {
				if(input.size() != 2) {
					feedback = (char*) "Error format penulisan login.\nFormat: leave [nama group]\n";
				}
				else {
					if(leave_group(input[1], ite -> second)){
						log(getDateTime() + " " + ite->second + " left group " + input[1]);
						feedback = (char*) ("Leaving group " + input[1] +"\n").c_str();
					}
					else
						feedback = (char*)("Failed to leave group " + input[1] +"\n").c_str();
				}
			}
		} else if (input[0] == "message") {
			map<int,string>::iterator ite;
			ite = logged_in_users.find(clientSocket);
			if (ite == logged_in_users.end()) { //berarti belum log in
				feedback = (char*) "Anda belum login!\n";
			} else {
				if (input.size() != 2) {
					feedback = (char*) "Error format message user/group.\nFormat: message [user_name/group_name]\n";
				} else {
					bool un_exists = is_username_exists(input[1]);
					bool group_exists = isGroupExist(input[1]);
					if(un_exists) {
						if (ite -> second == input[1]) {
							feedback = (char*) "Error tidak boleh kirim pesan ke diri sendiri\n";
						} else {
							string receiver = input[1];
							string sender = ite -> second;
							feedback = (char*) "Message: ";
							write(clientSocket, feedback, strlen(feedback)+1);
							memset(client_message,0,sizeof(client_message));
							read_size = recvStringFrom(clientSocket, client_message);
							feedback = (char *) "";
							string header_jam = getDateTime();
							inputMessage(sender,receiver,client_message,false, header_jam);
							if(findSenderReceiverConversationInUnread(receiver, sender)) { //dibalik apakah kita menerima pesan dari orang tersebut
								inputMessage(receiver,sender,client_message,true, header_jam); //masukkan ke diri sendiri
								feedback = (char *) "1";
							} else { //surun simpan di client
								string pesan = "0" + getDateTime() +" "+ ite -> second + " : " + string(client_message) + "\n";
								char temp[1500];
								strcpy(temp, pesan.c_str());
								feedback = temp;
							}
							write(clientSocket, feedback, strlen(feedback)+1);
							feedback = (char*) "";
							log(getDateTime() + " " + ite->second + " messaged " + input[1]);
							printUnread();
						}
					} else if (group_exists) {
						string receiver = input[1];
						string sender = ite -> second;
						feedback = (char*) "Message: ";
						write(clientSocket, feedback, strlen(feedback)+1);
						memset(client_message,0,sizeof(client_message));
						read_size = recvStringFrom(clientSocket, client_message);
						feedback = NULL;
						string header_jam = getDateTime();
						inputMessageGroup(sender,receiver,client_message,false,header_jam);
						if(findSenderReceiverConversationInUnread(receiver, sender)) {
							inputMessage(receiver,sender,client_message,true,header_jam); //masukkan ke diri sendiri
							feedback = (char*) "1";
						} else { //surun simpan di client
							string pesan = "0" + getDateTime() +" "+ ite -> second + " : " + string(client_message) + "\n";
							char temp[1500];
							strcpy(temp, pesan.c_str());
							feedback = temp;
						}
						write(clientSocket, feedback, strlen(feedback)+1);
						feedback = (char*) "";
						log(getDateTime() + " " + ite->second + " messaged " + input[1]);
					} else {
						feedback = (char*) (input[1] + " doesn't exist\n").c_str();
					}
				}
				
			}
		} else if (input[0] == "show") {
			map<int,string>::iterator ite;
			ite = logged_in_users.find(clientSocket);
			if (ite == logged_in_users.end()) { //berarti belum log in
				feedback = (char*) "Anda belum login!\n";
			} else {
				if (input.size() != 2) {
					feedback = (char*) "Error format show conversation.\nFormat: show [user_name/group_name]\n";
				} else {
					bool un_exists = is_username_exists(input[1]);
					bool group_exists = isGroupExist(input[1]);
					if(un_exists || group_exists) {
						if (ite -> second == input[1]) {
							feedback = (char*) "Error tidak boleh lihat pesan ke diri sendiri\n";
						} else {
							string sender = input[1];
							string receiver = ite -> second;
							printUnread();
							vector<string> msg =  readMessage(sender,receiver); //from, to
							string pesan = "1";
							for(int p = 0; p < msg.size(); ++p) { //vector ke string
 								pesan += (msg[p] + "\n");
							}
							char temp[1500];
							strcpy(temp,pesan.c_str());
							printUnread();
							feedback = temp;
							write(clientSocket, feedback, strlen(feedback)+1);
							feedback = (char*) "";
						}
					} else {
						feedback = (char*) ("Account "+ input[1] + " doesn't exist\n").c_str();
					}
				}
			}
		} else {
			feedback = (char* ) "command not exists";
		}
		
		//Membuat notifikasi
		map<int,string>::iterator ite;
		ite = logged_in_users.find(clientSocket);
		if (ite != logged_in_users.end()) { //berarti belum log in
			string fb = string(feedback);
			string notif =  getNotifications(ite -> second);
			string res = fb+notif;
			char temp[1900];
			strcpy(temp, res.c_str());
			feedback = temp;
		}
        write(clientSocket, feedback, strlen(feedback)+1);
        free(message);
        memset(client_message,0,sizeof(client_message));
    }
    if(read_size == 0){
		map<int,string>::iterator ite;
		ite = logged_in_users.find(clientSocket);
		if(ite != logged_in_users.end()) {
			log(getDateTime() + " " + ite->second + " logged out");
			logged_in_users.erase(ite);
		}
		cout << "\nUser yang sedang login dan socket\n";
		for(ite =  logged_in_users.begin(); ite!=logged_in_users.end(); ++ite) {
			cout << ite->first << " " << ite->second << endl;
		}
	}
	return 0;
}

bool is_username_exists(string username) {
	map<string,string> un_pass = getUsernamePassword();
	map<string,string>::iterator it = un_pass.find(username);
	return (it != un_pass.end());
}

void saveUnreadMessageToDatabase(){
	ofstream outputfile;
	outputfile.open("databases/unread.txt");
	int num_of_receiver = unread.size();
	outputfile << num_of_receiver << endl;
	for (int i = 1; i <= num_of_receiver; ++i) {
		message_format temp = unread[i-1];
		outputfile << temp.receiver << endl;
		int num_of_sender = temp.sender.size();
		outputfile << num_of_sender<<endl;

		for (map<string,vector<string> >::iterator it1 = temp.sender.begin(); it1 != temp.sender.end(); ++it1) {
			string sender_name = it1 -> first;
			int num_of_message = (it1 -> second).size();
			outputfile << sender_name << endl;
			outputfile << num_of_message << endl;
			vector<string> array_of_msg;
			for (int k = 1; k <= num_of_message; ++k) {
				string message_tmp = (it1 -> second)[k-1];
				outputfile << message_tmp << endl;
			}
		}
	}
	outputfile.close();
}
