#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <poll.h>

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
    
    while(serverRunning){
        char buffer[1024] = {0};
        recv(clientSocket, buffer, sizeof(buffer), 0);

        

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
    }
    // Close the socket after handling the request
    close(clientSocket);
}




int main(){
   
    int serverSocket = setUpServer();


    // Launch a separate thread to monitor for "exit" command
    std::thread shutdownThread(commandServer);



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
                //handle http parsing and return to this later for now 
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
