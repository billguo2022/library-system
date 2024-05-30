#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_L_PORT 42947 // Replace with the actual port number for ServerL
#define MAIN_SERVER_PORT 44947 // Replace with the main server's port number
#define MAIN_SERVER_IP "127.0.0.1" // Replace with the main server's IP address
#define BUFFER_SIZE 1024 

std::unordered_map<std::string, int> readBookData(const std::string& filename) {
    std::unordered_map<std::string, int> bookData;
    std::ifstream file(filename);
    std::string line, bookCode;
    int quantity;

    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        std::istringstream iss(line);
        if (getline(iss, bookCode, ',') && (iss >> quantity)) {
            bookCode.erase(bookCode.find_last_not_of(" \n\r\t")+1);
            bookData[bookCode] = quantity;
        }
    }

    file.close();
    return bookData;
}

void sendBookStatuses(const std::unordered_map<std::string, int>& bookData, int sockfd, const struct sockaddr_in& servaddr) {
    std::string bookStatuses;
    for (const auto& book : bookData) {
        bookStatuses += book.first + ":" + std::to_string(book.second) + ";";
    }

    sendto(sockfd, bookStatuses.c_str(), bookStatuses.length(), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    std::cout << "Server L sent book statuses to the Main Server.\n";
}

int main() {
    std::unordered_map<std::string, int> bookData = readBookData("input_files/literature.txt"); 
    int sockfd;
    struct sockaddr_in servaddr, cliaddr, main_servaddr;
    char buffer[BUFFER_SIZE];
    socklen_t len = sizeof(cliaddr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_L_PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    main_servaddr.sin_family = AF_INET;
    main_servaddr.sin_port = htons(MAIN_SERVER_PORT);
    main_servaddr.sin_addr.s_addr = inet_addr(MAIN_SERVER_IP);

    sendBookStatuses(bookData, sockfd, main_servaddr);

    std::cout << "Server L is up and running using UDP on port " << SERVER_L_PORT << ".\n";

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t message_length = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *) &cliaddr, &len);

        if (message_length < 0) {
            perror("recvfrom error");
            continue;
        }

        buffer[message_length] = '\0';
        std::string bookCode(buffer);
        bookCode.erase(bookCode.find_last_not_of(" \n\r\t")+1);

        std::cout << "Server L received " << bookCode << " code from the Main Server.\n";

        std::string response;
        auto bookEntry = bookData.find(bookCode);
        if (bookEntry != bookData.end()) {
            response = std::to_string(bookEntry->second);
        } else {
            response = "not found";
        }

        sendto(sockfd, response.c_str(), response.length(), 0, (struct sockaddr *)&cliaddr, len);
        std::cout << "Server L finished sending the status of code " << bookCode << " to the Main Server using UDP on port " << SERVER_L_PORT << ".\n";
    }

    close(sockfd);
    return 0;
}
