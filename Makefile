all:
	g++ -o server server.cpp
	g++ -o client client.cpp
	
runserver:
	./server
	
runclient:
	./client
