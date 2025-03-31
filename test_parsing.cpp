#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <cstring>
#include"http_parsing.h"





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

    std::cout << "\nBody:\n" << parsedRequest.raw_body << std::endl;

    std::cout << "\nParsed Body:\n";
    for (const auto& pair : parsedRequest.parsed_body) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
}



// Main function for testing
int main() {
  

    std::string rawRequest = 
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
            "name=test&email=test@example.com&age=25"; //need to parse this based on the use of & delimiter and the = seperator 

    printParsedHttp(rawRequest);
    printParsedHttp(rawRequest);
    


    return 0;
}