#include "Stream.h"
#include "Server.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <filesystem>

string Server::createPacket(int client)
{
	fd_set          read_fd;
    struct timeval  timeout;
    bool            packetCreated = false;
    bool            creating = true;
    char            buffer[65535];
    size_t          currentSize = 0;
    size_t          writtenByte = 0;
    size_t          dataLen;
    size_t          offset = 0;
    int             piece;
    string          path("");
    ofstream        out; 

	master.reset();

	while(creating){
		FD_ZERO(&read_fd);
		FD_SET(client, &read_fd);
		if(master.isMethod() != GET && master.isMethod() != POST && master.isMethod() != DELETE)
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
		int receiving = select(client + 1, &read_fd, NULL, NULL, &timeout);
		if(receiving < 0){
			creating = false;
			cerr << "Error on select\n";
		}
		else if(receiving == 0)
			break;
		else{
			if(FD_ISSET(client, &read_fd)){
				piece = recv(client, buffer, sizeof(buffer), 0);
				if(piece > 0){
					currentSize += piece;
					master.extract(buffer);
					if(master.getFileName() != "" && !out.is_open() && master.isMethod() == POST){
						// check accepted methods
						// check upload directory
						// check file length
						path = "upload/" + master.getFileName();
						out.open(path.c_str(), ios::out | ios::binary);
					}
					if(!out.is_open())
						continue;
					offset = (size_t)master.getHeaderLen();
				}
			}
			dataLen = piece - offset;
			size_t remainingLen = size_t(master.getFileLen()) - writtenByte;

			if(remainingLen <= dataLen)
				dataLen = remainingLen;
			if(dataLen > 0){
				char *sub = strstr(buffer + offset, master.getBoundary().c_str());
				if(sub)
					dataLen -= master.getBoundary().length() + 6;
				cout << "Tamanho offset: " << offset << endl;
				cout << "Tamanho dataLen: " << dataLen << endl;
				cout << "Imprimido: " << string(buffer + offset, dataLen) << endl;
				out.write(buffer + offset, dataLen);
				writtenByte += dataLen;
				// if(writtenByte > maxBodySize){
					// error max body size
					// break;
				// }
				if(master.isMethod() == POST)
					cout << "uploaded " << writtenByte << " of " << master.getFileLen() << endl;
				if(writtenByte >= master.getFileLen()){
					cout << "transfer done\n";
					break;
				}
				offset = 0;
		}
		else if(piece == 0 || writtenByte >= master.getFileLen()){
			cout << "Received entire file\n";
			creating = false;
			break;
		}
		else{
			creating = false;
			cout << "*uploaded " << writtenByte << " of " << master.getFileLen() << endl;
			break;
		}
	}
	out.close();
}