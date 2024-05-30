#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define TCP_PORT 45947  // Replace with the actual TCP port number
#define SERVER_S_PORT 41947 // Replace with actual port numbers for department servers
#define SERVER_L_PORT 42947
#define SERVER_H_PORT 43947
#define BUFFER_SIZE 1024
#define MAIN_SERVER_PORT 44947 

// std::unordered_map<std::string, std::string> readMembers() {
//     std::unordered_map<std::string, std::string> members;
//     std::ifstream file("input_files/member.txt");
//     std::string line, username, password;

//     while (getline(file, line)) {
//         std::size_t pos = line.find(", "); // Find the position of ", "
//         if (pos != std::string::npos) {
//             username = line.substr(0, pos); // Extract the username
//             password = line.substr(pos + 2); // Extract the password
//             members[username] = password;
//         }
//     }

//     std::cout << "Main Server loaded the member list.\n";

//     for (const auto& pair : members) {
//         std::cout << "username: " << pair.first << ", password: " << pair.second << std::endl;
//     }
//     return members;
// }


//after long hours of debugging, i realized the windows txt file always have \r at the end for each line which messes up my algorithm so i have to get rid of it
std::unordered_map<std::string, std::string> readMembers() {
    std::unordered_map<std::string, std::string> members;
    std::ifstream file("input_files/member.txt");
    std::string line, username, password;

    while (getline(file, line)) {
        // Remove carriage return at the end if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        std::size_t pos = line.find(", "); // Find the position of ", "
        if (pos != std::string::npos) {
            username = line.substr(0, pos); // Extract the username
            password = line.substr(pos + 2); // Extract the password
            members[username] = password;
        }
    }

    std::cout << "Main Server loaded the member list.\n";

    for (const auto& pair : members) {
        std::cout << "username: " << pair.first << ", password: " << pair.second << std::endl;
    }
    return members;
}


// Function to communicate with department servers using UDP
std::string communicateWithDepartmentServer(const std::string& bookCode, int serverPort) {
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        std::cerr << "Error in creating UDP socket\n";
        return "";
    }

    struct sockaddr_in dept_server_addr;
    memset(&dept_server_addr, 0, sizeof(dept_server_addr));
    dept_server_addr.sin_family = AF_INET;
    dept_server_addr.sin_port = htons(serverPort);
    dept_server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Assuming localhost, replace with actual IP if different

    sendto(udp_sock, bookCode.c_str(), bookCode.length(), 0, (struct sockaddr *)&dept_server_addr, sizeof(dept_server_addr));
    std::cout << "Main Server sent book request to server " << bookCode[0] << " using UDP over port " << serverPort << ".\n";

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    socklen_t len = sizeof(dept_server_addr);
    recvfrom(udp_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&dept_server_addr, &len);
    std::string response(buffer);

    std::cout << "Raw response from server: " << response << std::endl;  // Debugging line to see the raw response

    // std::size_t countPos = response.find("Count: ");
    // if (countPos != std::string::npos) {
    //     std::string countStr = response.substr(countPos + 7); // Extract count
    //     std::cout << "Number of books " << bookCode << " available is: " << countStr << ".\n";
    // } else {
    //     std::cout << "Number of books " << bookCode << " available is: not found.\n";
    // }

    // std::size_t countPos = response.find("Count: ");
    // std::string countStr = (countPos != std::string::npos) ? response.substr(countPos + 7) : "not found";

    std::cout << "Main Server received from server " << bookCode[0] << " the book status result using UDP over port " << serverPort << ":\n"
              << "Number of books " << bookCode << " available is: " << response << ".\n";

    close(udp_sock);
    return response;
}

std::unordered_map<std::string, int> receiveBookStatuses(int udp_sock) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    struct sockaddr_in source_addr;
    socklen_t len = sizeof(source_addr);

    recvfrom(udp_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&source_addr, &len);
   
    std::unordered_map<std::string, int> bookStatuses;
    std::istringstream iss(buffer);
    std::string bookCode;
    int quantity;

    while (iss >> bookCode >> quantity) {
        bookStatuses[bookCode] = quantity;
    }

    return bookStatuses;
}

// std::string trimNonPrintable(const std::string& str) {
//     std::string result;
//     for (char c : str) {
//         if (isprint(c)) {
//             result += c;
//         }
//     }
//     return result;
// }

#include <iomanip> // For std::setw and std::setfill

// Function to print a string in hex for debugging
void printHex(const std::string& str) {
    std::cout << "Hex: ";
    for (unsigned char c : str) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)c << " ";
    }
    std::cout << std::dec << "\n"; // Switch back to decimal
}


int main() {
    auto members = readMembers();

    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    // ... Error checking ...

    struct sockaddr_in udp_server_addr;
    memset(&udp_server_addr, 0, sizeof(udp_server_addr));
    udp_server_addr.sin_family = AF_INET;
    udp_server_addr.sin_addr.s_addr = INADDR_ANY;
    udp_server_addr.sin_port = htons(MAIN_SERVER_PORT);

    if (bind(udp_sock, (struct sockaddr *)&udp_server_addr, sizeof(udp_server_addr)) < 0) {
        std::cerr << "Error in binding UDP socket for book lists\n";
        close(udp_sock);
        exit(EXIT_FAILURE);
    }

    // Receive book lists from department servers
    auto bookStatusesS = receiveBookStatuses(udp_sock);
    std::cout << "Main Server received a book code list from server S using UDP over port " << SERVER_S_PORT << ".\n";
    auto bookStatusesL = receiveBookStatuses(udp_sock);
    std::cout << "Main Server received a book code list from server L using UDP over port " << SERVER_L_PORT << ".\n";
    auto bookStatusesH = receiveBookStatuses(udp_sock);
    std::cout << "Main Server received a book code list from server H using UDP over port " << SERVER_H_PORT << ".\n";
    // Close UDP socket after receiving book lists
    close(udp_sock);
    
   

    int tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sock < 0) {
        std::cerr << "Error in TCP socket creation\n";
        return -1;
    }

    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(TCP_PORT);

    if (bind(tcp_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error in binding TCP socket\n";
        close(tcp_sock);
        return -1;
    }

    listen(tcp_sock, 5);
    std::cout << "Main Server is up and running.\n";

    while (true) {
        socklen_t client_len = sizeof(client_addr);
        int new_sock = accept(tcp_sock, (struct sockaddr *)&client_addr, &client_len);
        if (new_sock < 0) {
            std::cerr << "Error in accepting client connection\n";
            continue;
        }

        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        recv(new_sock, buffer, BUFFER_SIZE, 0);
        std::string credentials(buffer);
        
        
        std::istringstream iss(credentials);
        std::string encryptedUsername, encryptedPassword;
        iss >> encryptedUsername >> encryptedPassword;
        std::cout<< encryptedUsername<<"\n";
        std::cout<< encryptedPassword <<"\n";

      

        std::cout << "Main Server received the username and password from the client using TCP over port " << TCP_PORT << ".\n";
        
        
        // std::cout << "Received Username Length: " << encryptedUsername.length() << "\n";
        // std::cout << "Received Password Length: " << encryptedPassword.length() << "\n";
        // printHex(encryptedUsername);
        // printHex(encryptedPassword);

        // if (members.find(encryptedUsername) != members.end()) {
        //     std::cout << "Stored Password Length: " << members[encryptedUsername].length() << "\n";
        //     printHex(members[encryptedUsername]);
        // }

        if (members.find(encryptedUsername) == members.end()) {
            std::cout << encryptedUsername << " is not registered. Send a reply to the client.\n";
            const char* response = "Username not registered";
            send(new_sock, response, strlen(response), 0);
        } else if (members[encryptedUsername] != encryptedPassword) {
            std::cout << "Password does not match the username. Send a reply to the client.\n";
            const char* response = "Incorrect password";
            send(new_sock, response, strlen(response), 0);
        // if (members.find(encryptedUsername) == members.end()) {
        //     std::cout << encryptedUsername << " is not registered. Send a reply to the client.\n";

        //     const char* response = "Username not registered";
        //     send(new_sock, response, strlen(response), 0);
        // } else if (members[encryptedUsername] != encryptedPassword) {
        //     std::cout << "Password does not match the username. Send a reply to the client.\n";
        //     std::cout << members[encryptedUsername]<<"\n";
        //     const char* response = "Incorrect password";
        //     send(new_sock, response, strlen(response), 0);
        // Trim non-printable characters from received username and password
        // std::string cleanEncryptedUsername = trimNonPrintable(encryptedUsername);
        // std::string cleanEncryptedPassword = trimNonPrintable(encryptedPassword);
        // std::cout << "Cleaned Username: " << cleanEncryptedUsername << "\n";
        // std::cout << "Cleaned Password: " << cleanEncryptedPassword << "\n";

        // if (members.find(cleanEncryptedUsername) == members.end()) {
        //     std::cout << cleanEncryptedUsername << " is not registered. Send a reply to the client.\n";
        //     const char* response = "Username not registered";
        //     send(new_sock, response, strlen(response), 0);
        // } else if (members[cleanEncryptedUsername] != cleanEncryptedPassword) {
        //     std::cout << "Password does not match the username. Send a reply to the client.\n";
        //     const char* response = "Incorrect password";
        //     send(new_sock, response, strlen(response), 0);
        } else {

            std::cout << "Password " << encryptedPassword << " matches the username. Send a reply to the client.\n"; // Message if password matches
            std::string encryptedBookCode1;
            
            const char* authSuccessMsg = "Authentication successful";
            send(new_sock, authSuccessMsg, strlen(authSuccessMsg), 0);

             while (true) { // Loop for continuous book code queries
                memset(buffer, 0, BUFFER_SIZE);
                int readBytes = recv(new_sock, buffer, BUFFER_SIZE, 0);
                if (readBytes <= 0) {
                    break; // Break the loop if the connection is closed or an error occurs
                }

                std::string encryptedBookCode(buffer);
                
                std::string serverResponse;
                if (encryptedBookCode[0] == 'S') {
                    std::cout << "Found " << encryptedBookCode << " located at Server S. Send to Server S.\n"; // Message if book code found in S
                    serverResponse = communicateWithDepartmentServer(encryptedBookCode, SERVER_S_PORT);
                } else if (encryptedBookCode[0] == 'L') {
                    std::cout << "Found " << encryptedBookCode << " located at Server L. Send to Server L.\n"; // Message if book code found in L
                    serverResponse = communicateWithDepartmentServer(encryptedBookCode, SERVER_L_PORT);
                } else if (encryptedBookCode[0] == 'H') {
                    std::cout << "Found " << encryptedBookCode << " located at Server H. Send to Server H.\n"; // Message if book code found in H
                    serverResponse = communicateWithDepartmentServer(encryptedBookCode, SERVER_H_PORT);
                } else {
                    std::cout << "Did not find " << encryptedBookCode << " in the book code list.\n"; // Message if book code not found
                    serverResponse = "Book code not found";
                }
                send(new_sock, serverResponse.c_str(), serverResponse.length(), 0);
                std::cout << "Main Server sent the book status to the client.\n"; // Message after sending book status to client
            }

            close(new_sock);
        }

        close(tcp_sock);
        return 0;
    }
}



