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
using namespace std;

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
	// penggunaan: ./client <server ip> <nilai n>
	if (argc != 3){
		printf("Pemakaian: ./client <server ip> <nilai n>\n");
	}
	
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
	char message[1000] , server_reply[2000];
	while(1) {
        printf(">");
        gets(message);
        cout << "Pesan yang akan dikirim: " << message << endl;
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
        puts("Server reply :");
        puts(server_reply);
    }
	
	close(sock);
	
	return 0;
}
