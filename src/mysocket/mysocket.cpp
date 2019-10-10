#include "mysocket.h"

MySocket::MySocket(int port)
{
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
}

MySocket::MySocket(const char *addr, int port)
{
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_aton(addr, &address.sin_addr);
}

void MySocket::createSocket()
{
    create_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (create_socket == -1)
    {
        socketError("create socket error");
    }
}

void MySocket::connectSocket()
{
    if (connect(create_socket, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        socketError("connect error");
    }
    cout << "Connection with server " << inet_ntoa(address.sin_addr) << " established" << endl;
}

void MySocket::bindSocket()
{
    if (bind(create_socket, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        socketError("bind error");
    }
}

void MySocket::listenSocket()
{
    if (listen(create_socket, 5) == -1)
    {
        socketError("listen error");
    }
    addrlen = sizeof(struct sockaddr_in);
}

int MySocket::acceptNewConnection()
{
    new_socket = accept(create_socket, (struct sockaddr *)&cliaddress, &addrlen);
    if (new_socket == -1)
    {
        socketError("accept error");
    }
    cout << "Client connected from " << inet_ntoa(cliaddress.sin_addr) << ":" << ntohs(cliaddress.sin_port) << endl;
    strcpy(buffer, "Welcome to myserver, Please enter your command:\n\0");
    if (send(new_socket, buffer, strlen(buffer), 0) == -1)
    {
        socketError("send error");
    }
    return new_socket;
}

const char *MySocket::recvMessage()
{
    size = recv(create_socket, buffer, BUF - 1, 0);
    if (size == -1)
    {
        socketError("recv error");
    }
    else if (size == 0)
    {
        return "QUIT\n\0";
    }
    buffer[size] = '\0';
    return buffer;
}

const char *MySocket::recvMessageFromClient(int socket)
{
    size = recv(socket, buffer, BUF - 1, 0);

    if (size == -1)
    {
        socketError("recv error");
    }
    else if (size == 0)
    {
        return "QUIT\n\0";
    }
    buffer[size] = '\0';
    return buffer;
}

void MySocket::sendMessage(const char *message)
{
    if (send(create_socket, message, strlen(message), 0) == -1)
    {
        socketError("send error");
    }
}

void MySocket::sendMessageToClient(const char *message, int socket)
{
    if (send(socket, message, strlen(message), 0) == -1)
    {
        socketError("send error");
    }
}

void MySocket::socketError(const char *errorMessage)
{
    perror(errorMessage);
    exit(EXIT_FAILURE);
}

MySocket::~MySocket()
{
    close(create_socket);
}
