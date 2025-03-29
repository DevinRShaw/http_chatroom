#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <poll.h>

#define PORT_NUMBER 8080


std::atomic<bool> serverRunning(true);

int setUpServer(){
    // creating socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    //this allows us to restart the server quickly without failiure to bind error
    int optval =  1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

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


void monitorShutdown(){
    std::string input;
    while (serverRunning) {
        std::getline(std::cin, input);
        if (input == "quit") {
            std::cout << "Shutting down the server..." << std::endl;
            serverRunning = false;
        }
        else {
            std::cout << input << " not recognized as command" << std::endl;
        }
    }
}




void handleClient(int clientSocket){ 
     // HTTP response
     const char* httpResponse = 
     "HTTP/1.1 200 OK\r\n"
     "Content-Type: text/html\r\n"
     "Connection: close\r\n\r\n"
     "<html><body><p>Hello, World!</p></body></html>";

    // sending HTTP response
    send(clientSocket, httpResponse, std::strlen(httpResponse), 0);


    //TODO FIX
        //disconnecting causes infinite messages
    while(serverRunning){

        char buffer[1024] = {0};
        recv(clientSocket, buffer, sizeof(buffer), 0);
    
        std::cout << "Message from client: " << buffer << std::endl;

        std::memset(buffer, 0, sizeof(buffer));  // Clears the buffer
        
        //handle the broadcast logic later, maybe keep a mutex style list of client sockets? 
        
    }
 

    // closing the client socket 
    close(clientSocket);
}




int main(){
   
    int serverSocket = setUpServer();


    // Launch a separate thread to monitor for "exit" command
    std::thread shutdownThread(monitorShutdown);



    // Configure pollfd structure for monitoring the server socket
    struct pollfd fds[1];
    fds[0].fd = serverSocket;
    fds[0].events = POLLIN;  // We're interested in the "read" event (new connections).



    while (serverRunning) {

        // Use poll() with a timeout of 0 to make it non-blocking (instant return)
        int ret = poll(fds, 1, 1);  // Timeout of 0ms means no blocking, instant return

        if (ret == -1) {
            std::cerr << "Error with poll()" << std::endl;
            break;
        }

        // Check if the server socket is ready to accept a connection
        if (fds[0].revents & POLLIN) {

            // The socket is ready to accept a connection
            int clientSocket = accept(serverSocket, nullptr, nullptr);
            
            //TODO FIX
                //this is printing multiple times per client
            std::cout << "connection made" << std::endl;

            if (clientSocket == -1) {
                std::cerr << "Failed to accept connection" << std::endl;
                continue;
            }

            // Handle the client connection in a separate thread
            //TODO FEATURE 
                //this could be handled better with poll() or async with epoll instead of threading at all 
            std::thread clientThread(handleClient, clientSocket);
            clientThread.detach();  // Detach the thread to handle it asynchronously
        }

    }

    // Closing the server socket after the server has stopped
    close(serverSocket);

    // Wait for the shutdown thread to finish
    shutdownThread.join();

    std::cout << "Server shutdown successfully." << std::endl;

    return 0;
}
