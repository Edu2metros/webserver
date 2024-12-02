#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define PORT 1234
#define MAX_BUFFER_SIZE 65535

using namespace std;

void createUploadDirectory() {
    struct stat st = {0};
    if (stat("upload", &st) == -1) {
        if (mkdir("upload", 0777) == -1) {
            cerr << "Erro ao criar diretório 'upload'.\n";
            exit(1);
        }
    }
}

void handleClient(int clientSocket) {
    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytesRead;
    string path = "upload/received_file";
    ofstream outFile;
    
    outFile.open(path, ios::out | ios::binary);
    if (!outFile.is_open()) {
        cerr << "Erro ao abrir o arquivo para escrita.\n";
        close(clientSocket);
        return;
    }

    cout << "Aguardando dados...\n";

	int count = 0;
    while ((bytesRead = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0)) > 0) {
		outFile.write(buffer, bytesRead);
		count++;
		std::cout << "Pacote " << count << " Recebido: " << buffer << std::endl << std::endl;
	}

    if (bytesRead == 0) {
        cout << "Transferência concluída.\n";
    } else if (bytesRead < 0) {
        cerr << "Erro ao receber dados.\n";
    }

    outFile.close();
    close(clientSocket);
}

int main() {
    int serverSocket, clientSocket;
    sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Criação do socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cerr << "Erro ao criar o socket.\n";
        return 1;
    }

    // Definição do endereço do servidor
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind do socket
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Erro ao fazer bind.\n";
        return 1;
    }

    // Inicia o servidor para ouvir por conexões
    if (listen(serverSocket, 5) < 0) {
        cerr << "Erro ao iniciar o servidor.\n";
        return 1;
    }

    cout << "Servidor escutando na porta " << PORT << "...\n";
    
    // Criação do diretório de upload, se não existir
    createUploadDirectory();

    // Loop de aceitação de conexões
    while (true) {
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket < 0) {
            cerr << "Erro ao aceitar conexão.\n";
            continue;
        }

        cout << "Cliente conectado, processando...\n";

        handleClient(clientSocket);
    }

    // Fechar o socket do servidor
    close(serverSocket);

    return 0;
}
