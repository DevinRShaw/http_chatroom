#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <atomic>

#define PORT_NUMBER 8080


std::atomic<bool> serverRunning(true);

int setUpServer(void){
    // creating socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    // specifying the address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT_NUMBER); // you can change this port if needed
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // binding socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Failed to bind socket" << std::endl;
        return 1;
    }

    // listening to the assigned socket
    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Failed to listen on socket" << std::endl;
        return 1;
    }
    
    std::cout << "Server is running on port 8080" << std::endl;
    std::cout << "http://localhost:8080" << std::endl;

    return serverSocket;

}


//maybe keep this to work around the blocking issue with accept 
void monitorShutdown(){
    std::string input;
    while (serverRunning) {
        std::getline(std::cin, input);
        if (input == "quit") {
            std::cout << "Shutting down the server..." << std::endl;
            serverRunning = false;
        }
    }
}




void handleClient(int clientSocket){ 
    while(serverRunning){

        // HTTP response
        const char* httpResponse = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n\r\n"
            "<html><body><h1>Hello, World!</h1></body></html>";

        // sending HTTP response
        send(clientSocket, httpResponse, std::strlen(httpResponse), 0);

        // receiving data
        char buffer[1024] = {0};
        recv(clientSocket, buffer, sizeof(buffer), 0);
    
        std::cout << "Message from client: " << buffer << std::endl;

        std::memset(buffer, 0, sizeof(buffer));  // Clears the buffer
        

        //handle the broadcast logic later, maybe keep a atomic list of client sockets? 
    }
    

    // closing the client socket
    close(clientSocket);
}








int main(){
   
    int serverSocket = setUpServer();


    // Launch a separate thread to monitor for "exit" command
    std::thread shutdownThread(monitorShutdown);

    while (serverRunning) {
        // accepting connection request
        int clientSocket = accept(serverSocket, nullptr, nullptr);

        //accept blocks here, meaning when the server is shutdown via command line, another client must connect to proceed and shutdown 
        //fixable via multiplexing? 

        if (clientSocket == -1) {
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }

        // Handle the client connection in a separate thread
        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach();  // Detach the thread to handle it asynchronously
    }



    // Closing the server socket after the server has stopped
    close(serverSocket);

    // Wait for the shutdown thread to finish
    shutdownThread.join();

    std::cout << "Server shutdown successfully." << std::endl;

    return 0;
}
