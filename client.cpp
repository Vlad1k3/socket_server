#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <chrono>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctime>
#include <iomanip>


class Client{
public:
    Client(const std::string& name, const std::string& serverIp, int port, int period) : name(name), serverIp(serverIp), port(port), period(period){}
    void start();
private:
    std::string name;
    std::string serverIp;
    int port;
    int period;
    std::string getCurrentTime();
};

void Client :: start(){
    while(true){
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0){
            std::cerr << "Ошибка создания сокета" << std::endl;
            return;
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);
        if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
            std::cerr << "Ошибка подключения к серверу" << std::endl;
            close(sock);
            std::this_thread::sleep_for(std::chrono::seconds(period));
            continue;
        }

        std::string message = getCurrentTime() + " " + name;
        send(sock, message.c_str(), message.length(), 0);

        close(sock);
        std::this_thread::sleep_for(std::chrono::seconds(period));
    }
}

std::string Client :: getCurrentTime(){
    std::time_t now = std::time(nullptr);
    std::tm* tm = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(tm, "[%Y-%m-%d %H:%M:%S]");
    return oss.str();
}

int main(int argc, char* argv[]){
    if (argc != 4){
        std::cerr << "Ипользовались наерпавильные флаги" << std::endl;
        return 1;
    }

    std::string name = argv[1];
    int port = std::stoi(argv[2]);
    int period = std::stoi(argv[3]);

    Client client(name, "127.0.0.1", port, period );
    client.start();

    return 0;
}
