#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <poll.h>
#include <sys/epoll.h>
#include <fcntl.h>

#include "http_parsing.h"

#define PORT_NUMBER 8080


std::atomic<bool> serverRunning(true);

//this is fine just sets up socket to take connections 
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


void commandServer(){
    std::string input;
    while (serverRunning) {
        std::getline(std::cin, input);
        if (input == "quit" || input == "exit") {
            std::cout << "Shutting down the server..." << std::endl;
            serverRunning = false;
        } 
        else {
            std::cout << input << " not recognized as command" << std::endl;
        }
    }
}



//this is a threaded method, will be easier to handle username aspect with this, can not yet see how to track user with polling type connection
void handleClient(int clientSocket) {
    
    
    char buffer[1024] = {0};

    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
    
    if (bytesReceived == -1) {
        perror("recv failed");  // Print error if recv() fails
    } else if (bytesReceived == 0) {
        std::cout << "Client disconnected: " << clientSocket << std::endl;
        //how to remove from the poll? 
    }

    

    HttpRequest clientResponse = parseHttpRequest(buffer);

    // Check if it's a GET or POST request
    if (clientResponse.method == "GET") {
        std::cout << "Message from client: " << buffer << std::endl;
        // Respond with the HTML form
        const char* httpResponse = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: keep-alive\r\n\r\n"
            "<html>"
                "<body>"
                    "<form action=\"/submit\" method=\"post\">"
                        "<label for=\"name\">Enter your name:</label>"
                            "<input type=\"text\" id=\"name\" name=\"name\" required>"
                        "<button type=\"submit\">Submit</button>"
                    "</form>"
                "</body>"
            "</html>";

        send(clientSocket, httpResponse, std::strlen(httpResponse), 0);
    } 
    
    else if (clientResponse.method == "POST") {
        std::cout << "Message from client: " << buffer << std::endl;
        // Handle the form submission
        std::string username = clientResponse.parsed_body["name"];

        // Respond with the greeting
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/html\r\n";
        response += "Connection: keep-alive\r\n\r\n";
        response += "<html><body><h1>Hello, " + username + "!</h1></body></html>";
        // Add the form after the greeting
        response += "<form action=\"/submit\" method=\"post\">"
                        "<label for=\"name\">Enter your name:</label>"
                            "<input type=\"text\" id=\"name\" name=\"name\" required>"
                        "<button type=\"submit\">Submit</button>"
                    "</form>";

        send(clientSocket, response.c_str(), std::strlen(response.c_str()), 0);
    }
    
    // Close the socket after handling the request
    close(clientSocket);
}


int setnonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl - F_GETFL");
        return -1;
    }

    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl - F_SETFL O_NONBLOCK");
        return -1;
    }

    return 0; // Success
}



int main(){
   
    int serverSocket = setUpServer();

    setnonblocking(serverSocket);


    // Launch a separate thread to monitor for "exit" command
    std::thread shutdownThread(commandServer);


    // Configure pollfd structure for monitoring the server socket
    #define MAX_EVENTS 10
    struct epoll_event ev, events[MAX_EVENTS];
    struct pollfd fds[1];
    fds[0].fd = serverSocket;
    fds[0].events = POLLIN;  // We're interested in the "read" event (new connections).

    int conn_sock, nfds, epollfd;



    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = serverSocket;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, serverSocket, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    while(serverRunning) {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, 0);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == serverSocket) {
                conn_sock = accept(serverSocket,nullptr,nullptr);
                if (conn_sock == -1) {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                setnonblocking(conn_sock);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
                            &ev) == -1) {
                    perror("epoll_ctl: conn_sock");
                    exit(EXIT_FAILURE);
                }
            } else {

                handleClient(events[n].data.fd);
            }
        
        }

    }

    

    // Closing the server socket after the server has stopped
    close(serverSocket);

    // Wait for the shutdown thread to finish
    shutdownThread.join();

    std::cout << "Server shutdown successfully." << std::endl;

    return 0;
}
