
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>

#define SERVER_M_PORT 45947 // Replace with the main server's port number
#define SERVER_M_IP "127.0.0.1" // Replace with the main server's IP address if different
#define BUFFER_SIZE 1024

// Function to encrypt the information before sending to the main server
std::string encrypt(const std::string& input) {
    std::string encrypted;
    for (char c : input) {
        if (isalpha(c) || isdigit(c)) {
            char offset = (isdigit(c) ? '0' : (islower(c) ? 'a' : 'A'));
            int alphaLength = (isdigit(c) ? 10 : 26); // 10 for digits, 26 for alphabet characters
            encrypted += (char)(((c - offset + 5) % alphaLength) + offset);
        } else {
            encrypted += c; // Leave special characters unchanged
        }
    }
    return encrypted;
}

int main() {
    std::cout << "Client is up and running.\n"; // Booting up message

    
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error\n";
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_M_PORT);

    if (inet_pton(AF_INET, SERVER_M_IP, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported\n";
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed\n";
        close(sock);
        return -1;
    }


    std::string username, password, bookCode;
    std::cout << "Please enter the username: ";
    std::cin >> username;
    std::cout << "Please enter the password: ";
    std::cin >> password;

    // Encrypt and send credentials
    std::string encryptedCredentials = encrypt(username) + " " + encrypt(password);
    // std::cout<<encryptedCredentials<<"\n";
    send(sock, encryptedCredentials.c_str(), encryptedCredentials.length(), 0);
    std::cout << username << " sent an authentication request to the Main Server.\n";

    // Wait for authentication response
    memset(buffer, 0, BUFFER_SIZE);
    read(sock, buffer, BUFFER_SIZE - 1);
    std::string authResponse(buffer);

    // Handling authentication response
    if (authResponse == "Username not registered") {
        std::cout << username << " received the result of authentication from Main Server using TCP over port " << SERVER_M_PORT << ". Authentication failed: Username not found.\n";
    } else if (authResponse == "Incorrect password") {
        std::cout << username << " received the result of authentication from Main Server using TCP over port " << SERVER_M_PORT << ". Authentication failed: Password does not match.\n";
    } else {
        std::cout << username << " received the result of authentication from Main Server using TCP over port " << SERVER_M_PORT << ". Authentication is successful.\n";


        // Loop for multiple book code queries
        while (true) {
            std::cout << "Please enter book code to query: ";
            std::cin >> bookCode;

            send(sock, bookCode.c_str(), bookCode.length(), 0);
            std::cout << username << " sent the request to the Main Server.\n";

            // Wait for book status response
            memset(buffer, 0, BUFFER_SIZE);
            read(sock, buffer, BUFFER_SIZE - 1);
            std::string bookStatus(buffer);
            std::cout << "Response received from the Main Server on TCP port: " << SERVER_M_PORT << ".\n";

            // Handling book status response
            // if (bookStatus.find("available") != std::string::npos) {
            //     std::cout << "The requested book " << bookCode << " is available in the library.\n";
            // } else if (bookStatus.find("not found") != std::string::npos) {
            //     std::cout << "Not able to find the book-code " << bookCode << " in the system.\n";
            // } else {
            //     std::cout << "The requested book " << bookCode << " is NOT available in the library.\n";
            // }

            // Assuming bookStatus is the string received from the server
            if (bookStatus == "not found") {
                std::cout << "Not able to find the book-code " << bookCode << " in the system.\n";
            } else {
                int bookCount = std::stoi(bookStatus); // Convert the response to an integer

                if (bookCount > 0) {
                    std::cout << "The requested book " << bookCode << " is available in the library. Total available copies: " << bookCount << ".\n";
                } else {
                    std::cout << "The requested book " << bookCode << " is NOT available in the library.\n";
                }
            }



            std::cout << "--- Start a new query ---\n";
        }
    }

    close(sock);
    return 0;
}