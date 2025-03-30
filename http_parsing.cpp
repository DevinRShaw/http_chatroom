#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <cstring>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body; //change this to a map of <field, value> pairs 
};

// Function to parse HTTP request
HttpRequest parseHttpRequest(const std::string& request) {
    HttpRequest httpRequest;
    std::istringstream requestStream(request);
    std::string line;
    
    // Parse Request Line
    if (std::getline(requestStream, line)) {
        std::istringstream lineStream(line);
        lineStream >> httpRequest.method >> httpRequest.path >> httpRequest.version;
    }

    // Parse Headers
    while (std::getline(requestStream, line) && line != "\r") {
        size_t colonPos = line.find(": ");
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 2); //possibly wrong substring since it is only +2
            //std::string value = line
            if (!value.empty() && value.back() == '\r') value.pop_back();  // Remove '\r'
            httpRequest.headers[key] = value;
        }
    }

    // Parse Body (if present)
    std::ostringstream bodyStream;
    while (std::getline(requestStream, line)) {
        bodyStream << line << "\n";
    }
    
    httpRequest.body = bodyStream.str();
    if (!httpRequest.body.empty() && httpRequest.body.back() == '\n') {
        httpRequest.body.pop_back();  // Remove trailing '\n'
    }

    return httpRequest;
}




// Test function
void printParsedHttp(std::string rawRequest) {
    

    HttpRequest parsedRequest = parseHttpRequest(rawRequest);

    // Print Parsed Data
    std::cout << "Method: " << parsedRequest.method << std::endl;
    std::cout << "Path: " << parsedRequest.path << std::endl;
    std::cout << "Version: " << parsedRequest.version << std::endl;
    
    std::cout << "\nHeaders:\n";
    for (const auto& header : parsedRequest.headers) {
        std::cout << header.first << ": " << header.second << std::endl;
    }

    std::cout << "\nBody:\n" << parsedRequest.body << std::endl;
}



// Main function for testing
int main() {
    std::string rawRequest =
        "GET / HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Connection: keep-alive\r\n"
        "sec-ch-ua: \"Chromium\";v=\"134\", \"Not:A-Brand\";v=\"24\", \"Google Chrome\";v=\"134\"\r\n"
        "sec-ch-ua-mobile: ?0\r\n"
        "sec-ch-ua-platform: \"Windows\"\r\n"
        "Upgrade-Insecure-Requests: 1\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.0.0 Safari/537.36\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\r\n"
        "Sec-Fetch-Site: none\r\n"
        "Sec-Fetch-Mode: navigate\r\n"
        "Sec-Fetch-User: ?1\r\n"
        "Sec-Fetch-Dest: document\r\n"
        "Accept-Encoding: gzip, deflate, br, zstd\r\n"
        "Accept-Language: en-US,en;q=0.9\r\n"
        "\r\n";  // End of headers

    std::string rawRequest2 = 
            "POST /submit HTTP/1.1\r\n"
            "Host: localhost:8080\r\n"
            "Connection: keep-alive\r\n"
            "Content-Length: 9\r\n"
            "Cache-Control: max-age=0\r\n"
            "sec-ch-ua: \"Chromium\";v=\"134\", \"Not:A-Brand\";v=\"24\", \"Google Chrome\";v=\"134\"\r\n"
            "sec-ch-ua-mobile: ?0\r\n"
            "sec-ch-ua-platform: \"Windows\"\r\n"
            "Origin: http://localhost:8080\r\n"
            "Content-Type: application/x-www-form-urlencoded\r\n"
            "Upgrade-Insecure-Requests: 1\r\n"
            "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.0.0 Safari/537.36\r\n"
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\r\n"
            "Sec-Fetch-Site: same-origin\r\n"
            "Sec-Fetch-Mode: navigate\r\n"
            "Sec-Fetch-User: ?1\r\n"
            "Sec-Fetch-Dest: document\r\n"
            "Referer: http://localhost:8080/\r\n"
            "Accept-Encoding: gzip, deflate, br, zstd\r\n"
            "Accept-Language: en-US,en;q=0.9\r\n"
            "\r\n"
            "name=test";

    printParsedHttp(rawRequest);
    printParsedHttp(rawRequest2);
    return 0;
}