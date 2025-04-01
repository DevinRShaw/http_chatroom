#ifndef HTTP_REQUEST_PARSER_H
#define HTTP_REQUEST_PARSER_H

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
    std::string raw_body; 
    std::map<std::string, std::string> parsed_body;
};


// possible optimization here in the parameter as a reference? Less copying?
std::map<std::string, std::string> parseRawBody(HttpRequest httpRequest){
        enum body_state {
            KEY,
            VALUE
        };

        body_state map_state  = KEY;
        //parse the body into a map of key value pairs 
        std::string key = "";
        std::string value = "";
        int i = 0;
        for(auto var : httpRequest.raw_body)
        {
            
            
            //can use a DFA style approach here 
                //state 1 = key , = seen -> state 2 
                //state 2 = value , if /n seen and end of string we are done, if & seen append the <key, value> pair to map -> state 1
            

            if (map_state == KEY && var != '='){
                key += var;
            }

            else if (map_state == KEY && var == '='){
                map_state = VALUE;
                
            }

            
            else if (map_state == VALUE && var != '&'){
                value += var; 

            }

            else if (map_state == VALUE && var == '&' ){
                
                httpRequest.parsed_body[key] = value;
                key = "";
                value = "";
                map_state = KEY;
            }
            

        }

        return httpRequest.parsed_body;

}

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
        bodyStream << line << "&";
    }
    
    httpRequest.raw_body = bodyStream.str();

    /*
    if (!httpRequest.raw_body.empty() && httpRequest.raw_body.back() == '\n') {
        httpRequest.raw_body.pop_back();  // Remove trailing '\n'
    }
    */

    

    if (!httpRequest.raw_body.empty()){
        httpRequest.parsed_body = parseRawBody(httpRequest);
    }

    return httpRequest;
}


#endif // HTTP_REQUEST_PARSER_H
