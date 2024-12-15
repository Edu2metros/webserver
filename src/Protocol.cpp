#include "Protocol.h"

Protocol::Protocol(void) : method("GET"), path("/"), type("HTTP/1.1"), connection("keep-alive"), boundary(""), file(""), length(0), header(0){}

Protocol::Protocol(char *data) : method("GET"), path("/"), type("HTTP/1.1"), connection("keep-alive"), boundary(""), file(""), length(0), header(0){
    extract(data);
}

void    Protocol::reset(void) {
    method = "GET";
    path = "";
    type = "";
    connection = "";
    boundary = "";
    file = "";
    length = 0;
    header = 0;
    contentBody = "";
}

string inside(string text, string sub, string stop) {
    size_t  pos;
    if ((pos = text.find(sub)) != string::npos) {
        size_t  start = pos + sub.length();
        size_t  end = text.find(stop, start);
        return text.substr(start, end - start);
    }
    return "";
}


bool    Protocol::extract(const char *data){
    istringstream parse(data);
    size_t  pos;
    if((pos = parse.str().find("Host: ")) != string::npos)
        tmpHost = parse.str().substr(pos + 6, parse.str().find("\n", pos) - pos - 6);
    pos = tmpHost.find(":");
   if(pos != string::npos)
        tmpHost = tmpHost.substr(0, pos);

    parse >> method >> path >> type;
    if ((pos = parse.str().find("\r\n\r\n")) != string::npos) {
        size_t next_pos = parse.str().find("\r\n\r\n", pos + 4);
        if (next_pos != string::npos) {
            header = next_pos + 4;
        } else {
            header = pos + 4;
        }
    }
    connection = inside(parse.str(), "Connection: ", "\n");
    if(boundary == "")
        boundary = inside(parse.str(), "boundary=", "\r\n");
    if(file == "")
        file = inside(parse.str(), "filename=\"","\"");
    if(length == 0)
        length = atoll(inside(parse.str(), "Content-Length: ", "\n").c_str());
    cout << "Method: " << method << " Path: " << path << " Type: " << type << endl;
    if(boundary == "" || file == "" || length == 0)
        return false;
    return true;
}


void    Protocol::setMethod(string value) {
    method = value;
}

method_e    Protocol::isMethod(void) {
    if (method == "GET")
        return GET;
    else if (method == "POST")
        return POST;
    else if (method == "DELETE")
        return DELETE;
    else if(method == "ENTITY_TOO_LARGE")
        return ENTITY_TOO_LARGE;
    else if(method == "INVALID_HOST")
        return INVALID_HOST;
    return INVALID_REQUEST;
}

string  Protocol::getPath(void) {
    return path;
}

string  Protocol::getType(void) {
    return type;
}

string  Protocol::getConnection(void) {
    return connection;
}

string  Protocol::getBoundary(void) {
    return boundary;
}

string  Protocol::getFileName(void) {
    return file;
}

string Protocol::getHost(void){
    return tmpHost;
}

size_t  Protocol::getFileLen(void) {
    return length;
}

size_t  Protocol::getHeaderLen(void) {
    return header;
}

Protocol::~Protocol(){}