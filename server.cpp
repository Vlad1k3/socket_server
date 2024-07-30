#include <iostream>
#include <fstream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <mutex>
#include <arpa/inet.h>
#include <unistd.h>

std::mutex fileMutex;

class Server{

public:
    Server(int port):port(port), serverSock(-1) {}
    ~Server(){
        if (serverSock != -1){
            close(serverSock);
        }
    }
    bool start();

private:
    int port;
    int serverSock;
    void handleClient(int clientSock);
};

bool Server :: start(){
    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock < 0){
        std::cerr << "Ошибка создания сокета" << std::endl;
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if(bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
        std::cerr << "Ошибка привязки сокета" << std::endl;
        return false;
    }
    if (listen(serverSock, 5) < 0){
        std::cerr << "Ошибка прослушивания" << std::endl;
        return false;
    }

    std::cout << "Сервер запущен на порту " << port << std::endl;

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientSock = accept(serverSock, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSock < 0){
            std::cerr << "Ошибка принятия соединения" << std::endl;
            continue;
        }
        std::thread(&Server::handleClient, this, clientSock).detach();
    }
    return true;
}

void Server::handleClient(int clientSock){
    char buffer[1024] = {0};
    int bytrsRead = read(clientSock, buffer, sizeof(buffer));
    if (bytrsRead > 0){
        std::lock_guard<std::mutex> lock(fileMutex);
        std::ofstream logFile("log.txt", std::ios::app);
        if (logFile.is_open()){
            logFile << buffer << std::endl;
        }
    }
    close(clientSock);
}

int main(int argc, char* argv[]){
    if (argc != 2){
        std::cerr << "Ошибка введения аругментов" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);
    Server server(port);
    if (!server.start()){
        return 1;
    }
    return 0;
}
