
d.serverM: readMembers Function: Reads a file containing usernames and passwords, storing them in an unordered_map. 
communicateWithDepartmentServer Function: Communicates with department servers using UDP to request book statuses. It sends a book code to the department server and receives a response.
receiveBookStatuses Function: Receives book statuses from department servers using UDP.
printHex Function: A debugging utility to print strings in hexadecimal format.
main Function: The core of the program, which includes:
Initializing a UDP socket to receive book lists from department servers.
Creating a TCP socket to handle client connections for user authentication and book code queries.
Authenticating users by comparing received credentials with the stored ones.
If authenticated, the server continuously receives book codes from the client, determines the appropriate department server based on the book code, and uses communicateWithDepartmentServer to get the book status.
Sending the book status back to the client.

serverS, serverH, serverL: readBookData Function: Reads book data (book codes and quantities) from a specified file. The function accounts for possible carriage returns in the data (common in Windows text files) and trims any trailing whitespace.
sendBookStatuses Function: Formats and sends the book statuses (comprising book codes and their quantities) to the main server using UDP.
main Function: The primary function of the program, which includes:
Reading book data from a file.
Creating a UDP socket and binding it to Servers port.
Sending the initial book statuses to the main server.
Continuously listening for incoming requests from the main server.
On receiving a book code from the main server, the server checks its availability and sends back the quantity (or a "not found" message if the book is not in its records).
The server communicates the response using UDP to the main server.

client: encrypt Function: A simple function that encrypts the input string by shifting alphanumeric characters by a fixed number of positions. This is a basic form of encryption, primarily for demonstration purposes.
main Function: The main function of the program, which includes:
Creating a TCP socket and establishing a connection with the main server.
Prompting the user for a username and password, encrypting these credentials, and sending them to the server for authentication.
Receiving an authentication response from the server and handling different scenarios: username not registered, incorrect password, or successful authentication.
If authenticated, entering a loop to allow the user to repeatedly query for book codes:
The user enters a book code, which is sent to the server.
The program waits for a response from the server regarding the status of the requested book.
The response is interpreted as either "not found" or the number of available copies, and appropriate messages are displayed.




