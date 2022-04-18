#undef UNICODE

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x501

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <limits.h>

#define DEFAULT_BUFLEN 8096
#define DEFAULT_PORT 5050
#define DEFAULT_ADDR "127.0.0.1"

using namespace std;

int main()
{
    cout << "\t\t-------- TCP SERVER --------" << endl;
    cout << endl;

    // local vars
    WORD wVersionRequested = MAKEWORD(2, 2); //MAKEWORD(lowbyte, highbyte)
    WSADATA wsaData;
    int iWsaCleanup;
    int err;

    SOCKET TCPServerSocket;
    int iCloseSocket;

    struct sockaddr_in TCPServerAdd;
    struct sockaddr_in TCPClientAdd;
    int iTCPClientAdd = sizeof(TCPClientAdd);

    int iBind;
    int iListen;
    SOCKET sAcceptSocket = INVALID_SOCKET;

    // int iSend;
    // char SenderBuffer[DEFAULT_BUFLEN] = "Hello from Server!";
    // int iSenderBuffer = strlen(SenderBuffer) + 1;
    
    int iRecv;
    char RecvBuffer[DEFAULT_BUFLEN];
    int iRecvBuffer = DEFAULT_BUFLEN;// + 1;

    // Step 1: WSA Startup
    err = WSAStartup(wVersionRequested, &wsaData);
    if(err != 0){
        cout << "WSAStartup failed with error " << err << endl;
        return 1;
    }
    cout << "WSAStartup was successful." << endl;

    // Step 2: Fill socket struct
    TCPServerAdd.sin_family = AF_INET; // IPv4 address family
    TCPServerAdd.sin_addr.s_addr = inet_addr(DEFAULT_ADDR); 
    TCPServerAdd.sin_port = htons(DEFAULT_PORT); // bind to port, convert from host byte order to ntk byte order

    // Step 3: Create socket
    TCPServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(TCPServerSocket == INVALID_SOCKET){
        cout << "TCP server socket creation failed with error " << WSAGetLastError() << endl;
        return 1;
    }
    cout << "TCP server socket creation successful." << endl;

    // Step 4: Bind 
    // associate local address with socket
    iBind = bind(TCPServerSocket, (sockaddr*)&TCPServerAdd, sizeof(TCPServerAdd));
    if(iBind == SOCKET_ERROR){
        cout << "Binding failed with error " << WSAGetLastError() << endl;
        return 1;
    }
    cout << "Binding successful." << endl;

    // Step 5: Listen for incoming connections
    iListen = listen(TCPServerSocket, INT_MAX); // TODO: is using INT_MAX efficient
        // backlog = INT_MAX = number of connections to create
    if(iListen == SOCKET_ERROR){
        cout << "Listen failed with error " << WSAGetLastError() << endl;
        return 1;
    }
    cout << "Listen successful." << endl;

    // Step 6: Accept connection
    sAcceptSocket = accept(TCPServerSocket, (sockaddr*)&TCPClientAdd, &iTCPClientAdd);
    if(sAcceptSocket == SOCKET_ERROR){
        cout << "Accept failed with error " << WSAGetLastError() << endl;
        return 1;
    }
    cout << "Connection accepted." << endl;

    // Step 8: Receive data from client
    do{
        iRecv = recv(sAcceptSocket, RecvBuffer, iRecvBuffer, 0);
        if ( iRecv > 0 ){
            printf("Bytes received: %d\n", iRecv);
            cout << RecvBuffer << endl; // debugging
        }
        else if ( iRecv == 0 )
            printf("Connection closed\n");
        else
            cout << "Server receive failed with error " << WSAGetLastError() << endl;
            return 1;
    } while(iRecv > 0);
    cout << "Data received successfully." << endl;

    // Step 9: Close socket
    iCloseSocket = closesocket(TCPServerSocket);
    if(iCloseSocket == SOCKET_ERROR){
        cout << "Closing socket failed with error " << WSAGetLastError() << endl;
        return 1;
    }
    cout << "Socket closed successfully." << endl;

    // Step 10: Cleanup Winsock DLL
    iWsaCleanup = WSACleanup();
    if(iWsaCleanup == SOCKET_ERROR){
        cout << "Cleanup failed with error " << WSAGetLastError() << endl;
        return 1;
    }
    cout << "Cleanup successful." << endl;

    system("PAUSE");
    return 0;
}