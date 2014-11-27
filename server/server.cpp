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
 
 void *connection_handler(void *);
 
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
	int clientSocket;
	pthread_t thread_id;
	while(true){
		clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, (socklen_t*) &addrlen);
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
	
	//Send message to client
	message = (char*)"Selamat datang ke Messenger\n";
	write(clientSocket, message, strlen(message));
	
	message = (char*)"Tulis sesuatu dan saya akan mengulanginya\n";
	write(clientSocket, message, strlen(message));
	
	//Recieve message from client
	read_size = recv(clientSocket, client_message, 2000, 0);
	//end of string marker
	client_message[read_size] = '\0';
	
	//Logging
	printf("Klien tulis: %s\n", client_message);
	
	//send the message back to client
	write(clientSocket, client_message, strlen(client_message));
	
	//hapus pesan dari buffer
	memset(client_message, 0, 2000);
	
	return 0;
	
}
