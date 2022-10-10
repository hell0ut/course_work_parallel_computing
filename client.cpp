#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <string>

#define prot IPPROTO_TCP;


int shutdown_services(ADDRINFO* addrResult, SOCKET* ConnectSocket, const std::string& message, int result) {
    std::cout << message << " " << result << std::endl;
    if (ConnectSocket != nullptr) {
        closesocket(*ConnectSocket);
        *ConnectSocket = INVALID_SOCKET;
    }
    freeaddrinfo(addrResult);
    WSACleanup();
    return 1;
}


int main()
{
    WSADATA wsaData;
    int result;
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    ADDRINFO hints;
    ADDRINFO* addrResult = nullptr;

    if (result != 0) return shutdown_services(addrResult, nullptr, "WSAStartup failed, result = ", result);

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = prot;

    result = getaddrinfo("localhost", "666", &hints, &addrResult);
    if (result != 0) return shutdown_services(addrResult, nullptr, "getaddrinfo failed, result = ", result);

    SOCKET ConnectSocket = INVALID_SOCKET;

    ConnectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) return shutdown_services(addrResult, &ConnectSocket, "Socket creation failed, result = ", result);

    result = connect(ConnectSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) return shutdown_services(addrResult, &ConnectSocket, "Unable to connect to server, result = ", result);

    std::cout << "Input your request:\n"<<
              "arg 1 word, arg 2 indexedView_bool (hello 1);\n use 0 instead of word to finish\n";

    while (true) {
        std::string request;
        std::getline(std::cin, request);

        result = send(ConnectSocket, request.c_str(), (int)strlen(request.c_str()), 0);
        if (result == SOCKET_ERROR) return shutdown_services(addrResult, &ConnectSocket, "Send failed, result = ", result);

        std::cout << "Bytes sent: " << result << " bytes" << std::endl;
        char recvBuffer[4096];
        ZeroMemory(recvBuffer, 4096);
        result = recv(ConnectSocket, recvBuffer, 4096, 0);
        if (result > 0) {
            std::cout << "Receieved data:\n" << recvBuffer << std::endl;
        }
        else if (result == 0) {
            std::cout << "Connection closed" << std::endl;
            break;
        }
        else {
            std::cout << "Recv failed with error: " << result << std::endl;
            break;
        }
    }

    shutdown_services(addrResult, &ConnectSocket, "Returned ", result);
    return 0;


}
