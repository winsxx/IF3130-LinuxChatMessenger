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


//Daftar fungsi
void saveMessage(string user, string content);
string loadMessage(string user);
int recvStringFrom(int socketId, char* server_reply);

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
        message[strlen(message)] ='\0';
        if(send(sock , message, strlen(message)+1, 0) < 0) {
            puts("Send failed");
            return 1;
        }
		memset(server_reply,0,sizeof(server_reply));
        //Receive a reply from the server
        if(recvStringFrom(sock, server_reply) < 0) {
            puts("recv failed");
            break;
        }

        //Tampilkan pesan dengan user
        string_token = splitchar(message);
        if(string_token[0].compare("show") == 0 && string_token.size()>1){
			string content = loadMessage(string_token[1]);
			cout << content;
			if (strcmp(server_reply,"") != 0){
				puts("----- New Message(s) -----");
				content += string(server_reply);
				saveMessage(string_token[1], content);
			}
		}
        
        //Jika logout, keluar dari loop dan close sock, recv server akan menerima return 0
        if(string_token[0].compare("logout") == 0){
			break;
		}
        
        if (strcmp(server_reply,"Message: ")==0) {
        	puts(server_reply);
        	gets(message2);
        	message2[strlen(message2)] ='\0';
        	if(send(sock , message2, strlen(message2)+1, 0) < 0) {
            	puts("Send failed");
	            return 1;
	        } else {
				memset(server_reply,0,sizeof(server_reply));
				//Receive a reply from the server
				if(recvStringFrom(sock, server_reply) < 0) {
					puts("recv failed");
					break;
				} else{
					//Balasan dari server
					string pesan = string(server_reply);
					puts("ok");
					puts(server_reply);
					puts("ok");
					if(pesan[0]=='0'){
						pesan.erase(pesan.begin());
						string content = loadMessage(string_token[1]);
						content.append(pesan);
						saveMessage(string_token[1],content);
						puts(" -------------- Ini berarti pesan yang hrs langsung disimpan ke dalam client");
					}
				}
			}
        } else {
			//Balasan dari server
			puts(server_reply);
		}
        
    }
	
	close(sock);
	
	return 0;
}


//Menyimpan message ke database
void saveMessage(string user, string content){
	ofstream message_file;
	message_file.open((user+".txt").c_str());
	//Tulis jumlah user
	message_file << content;
	
	message_file.close();
}


//Mengambil data message dari database
string loadMessage(string user){
	ifstream message_file;
	message_file.open((user+".txt").c_str());
	
	string content="";
	string line;
	
	//Baca semua isi
	while(getline(message_file, line)){
		content+=line;
		content+="\n";
	}
	
	message_file.close();
	
	return content;
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
	
	if(code<=0) //kode error atau client tutup
		return code;
	else 
		return cum; //banyaknya karakter
}

