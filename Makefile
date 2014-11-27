all:
	g++ -pthread -o server_app server/server.cpp
	g++ -o client_app client/client.cpp
	
runserver:
	./server
	
runclient:
	./client
