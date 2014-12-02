//Ini masih asal-asalan, untuk testing aja
//Harus dihapus dan dibuat ulang

/**
	klien: mengirimkan string ke server dan diblas server dengan string yg sama
*/

#include <cstdio>
#include <sys/types.h>   // tipe data penting untuk sys/socket.h dan netinet/in.h
#include <netinet/in.h>  // fungsi dan struct internet address
#include <sys/socket.h>  // fungsi dan struct socket API
#include <netdb.h>       // lookup domain/DNS hostname
#include <unistd.h>
#include <cstdlib>
#include <errno.h>
#include <cstring>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <ctime>
using namespace std;


typedef struct{
	time_t waktu;
	string dari;
	string isi;
}aMessage;

map<string, vector< aMessage > > message_list;

//Daftar fungsi
void saveMessage();
void loadMessage();
void showMessage(string user);
void addMessage(string user, string from, time_t waktu, string isi);
string getTimeString(time_t waktu);

/*
 * Masukan berupa sebuah baris string yang katanya dipisahkan spasi
 * Keluaran berupa vector yang tiap elemennya berisi kata
 */
vector<string> splitchar(char param[1000]) {
	vector<string> retval;
	string temp;
	stringstream test;
	test << param;
	while(getline(test,temp,' ')) {
		retval.push_back(temp);
	}
	return retval;
}

int main(int argc, char** argv){
	int sock, port, len; char buffer[1000];
	struct sockaddr_in serv_addr;
	struct hostent *server;
	port = 8888;
	
	// buka socket TCP (SOCK_STREAM) dengan alamat IPv4 dan protocol IP
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0){
		close(sock);
		printf("Cannot open socket\n");
		exit(1);
	}
	
	// buat address server
	server = gethostbyname("localhost");
	if (server == NULL) {
		fprintf(stderr,"Host not found\n");
		exit(1);
	}
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(port);
	
	// connect ke server, jika error keluar
	if (connect(sock,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) exit(1);
	char message[1000] , server_reply[2000], message2[1000];
	vector<string> string_token;
	while(1) {
        printf(">");
        gets(message);
        //Kirim pesan ke server
        if(send(sock , message, strlen(message), 0) < 0) {
            puts("Send failed");
            return 1;
        }
		memset(server_reply,0,sizeof(server_reply));
        //Receive a reply from the server
        if(recv(sock, server_reply, 2000, 0) < 0) {
            puts("recv failed");
            break;
        }
        
        //Tampilkan pesan dengan user
        string_token = splitchar(message);
        if(string_token[0].compare("show") == 0 && string_token.size()>1){
			loadMessage();
			showMessage(string_token[1]);
		}
        
        //Jika logout, keluar dari loop dan close sock, recv server akan menerima return 0
        if(string_token[0].compare("logout") == 0){
			break;
		}
		
		//Balasan dari server
        puts(server_reply);
        
        if (strcmp(server_reply,"Message: ")==0) {
        	gets(message2);
        	if(send(sock , message2, strlen(message2), 0) < 0) {
            	puts("Send failed");
	            return 1;
	        } else {
				memset(server_reply,0,sizeof(server_reply));
				//Receive a reply from the server
				if(recv(sock, server_reply, 2000, 0) < 0) {
					puts("recv failed");
					break;
				} else{
					//Balasan dari server
					puts(server_reply);
				}
				string_token = splitchar(message);
				addMessage(string_token[1], "me", time(0), message2);
				saveMessage();
			}
        }
    }
	
	close(sock);
	
	return 0;
}


//Menyimpan message ke database
void saveMessage(){
	ofstream message_file;
	message_file.open("data.txt");
	//Tulis jumlah user
	message_file << message_list.size() <<endl;
	
	//Ulang sebanyak jumlah user
	for(map<string, vector<aMessage> >::iterator it = message_list.begin();it != message_list.end(); it++){
		//Tulis nama user
		message_file << it->first << endl;
		vector<aMessage> message_from = it->second;
		//Tulis jumlah message dengan user
		message_file << message_from.size() <<endl;
		//Tulis semua message dengan user
		for(int j=0; j<message_from.size(); j++){
			aMessage temp = message_from[j];
			message_file << temp.waktu << endl;
			message_file << temp.dari << endl;
			message_file << temp.isi << endl;
		}
	}
	message_file.close();
}


//Mengambil data message dari database
void loadMessage(){
	ifstream message_file;
	message_list.clear();
	message_file.open("data.txt");
	
	string line;
	
	//Baca jumlah user
	getline(message_file, line);
	int user_size;
	sscanf(line.c_str(), "%d", &user_size);
	
	//Ulang sebanyak jumlah user
	for(int i=0; i<user_size; i++){
		//Baca nama "user_name"
		string user_name;
		getline(message_file, user_name);
		
		//Baca jumlah message dengan "user_name"
		int numMessage;
		getline(message_file, line);
		sscanf(line.c_str(), "%d", &numMessage);
		
		//Baca semua message dengan "user_name"
		vector<aMessage> temp_messages;
		for(int j=0; j<numMessage; j++){
			aMessage temp;
			int intTime;
			//waktu
			getline(message_file, line);
			sscanf(line.c_str(), "%d", &intTime);
			temp.waktu = (time_t) intTime;
			//User
			getline(message_file, temp.dari);
			//Isi
			getline(message_file, temp.isi);
			
			temp_messages.push_back(temp);
		}
		message_list[user_name] = temp_messages;
	}
	message_file.close();
}

//Menampilkan message percakapan dengan 'user'
void showMessage(string user){
	map<string, vector<aMessage> >::iterator it = message_list.find(user);
	if(it == message_list.end()){
		cout<< "User message not exist" <<endl;
	} else {
		vector<aMessage> message_content;
		message_content = it->second;
		for(int i=0; i< message_content.size(); i++){
			printf("[%s] %s : %s", getTimeString(message_content[i].waktu).c_str(), 
				message_content[i].dari.c_str(), message_content[i].isi.c_str());
		}
	}
}

//Tambah pesan dari user
void addMessage(string user, string from, time_t waktu, string isi){
	aMessage temp;
	temp.waktu = time(0);
	temp.dari = from;
	temp.isi = isi;
	
	message_list[user].push_back(temp);
}

//Dapatkan tanggal dan waktu dalam format yg sesuai
string getTimeString(time_t waktu){
	struct tm * timeinfo;
	timeinfo = localtime ( &waktu);
	
	int tahun = 1900+timeinfo->tm_year;
	int bulan = 1+timeinfo->tm_mon;
	int tanggal = 1+timeinfo->tm_mday;
	
	int jam = 1+timeinfo->tm_hour;
	int menit = 1+timeinfo->tm_min;
	int detik = 1+timeinfo->tm_sec;
	
	char temp[30];
	sprintf(temp,"%d-%d-%d %d:%d",tahun,bulan,tanggal,jam,menit);
	
	return string(temp);
}

int recvStringFrom(int socketId, char* server_reply){
	String msg="";
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
	server_reply = msg.c_str();
	
	if(code<=0) //kode error atau client tutup
		return code;
	else 
		return cum; //banyaknya karakter
}
