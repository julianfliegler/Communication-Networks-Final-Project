/*
===============================================
Title:   server.cpp
Authors: Julian Fliegler, Allison Rose Johnson
Date:    2 May 2022
===============================================
*/

// works, but want to add md5 checksum 
        
#define WIN32_LEAN_AND_MEAN     // exclude rarely used Windows headers
#define _WIN32_WINNT 0x0501     // version at least Windows XP

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <iostream>
#include <limits.h>             // INT_MAX
//#include <process.h>
#include <time.h> // _strtime

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 5050
#define DEFAULT_ADDR "127.0.0.1"

// prototypes
int CalcChecksum(char buffer[]);
DWORD WINAPI MyRecv(LPVOID lpParameter);

using namespace std;

int main()
{
    /* ref: https://www.youtube.com/watch?v=TP5Q0cs6uNo&t=1228s */
    cout << "\t\t-------- TCP SERVER --------" << endl;
    cout << endl;

    // WSAStartup vars
    WORD wVersionRequested = MAKEWORD(2, 2); // Winsock version 2.2
    WSADATA wsaData;
    int iWsaCleanup;

    // socket vars
    SOCKET TCPServerSocket;
    int iCloseSocket;
    
    // address vars
    struct sockaddr_in TCPServerAdd;
    struct sockaddr_in TCPClientAdd;
    int iTCPClientAdd = sizeof(TCPClientAdd);

    // bind, listen, accept, recv vars
    int iBind;
    int iListen;
    SOCKET sAcceptSocket;
    int iRecv;
    char RecvBuffer[DEFAULT_BUFLEN];
    int iRecvBuffer = sizeof(RecvBuffer);

    // Step 1: WSA Startup
    int err = WSAStartup(wVersionRequested, &wsaData);
    if(err != 0){
        cout << "WSAStartup failed with error " << err << endl;
        return 1;
    }
    cout << "WSAStartup successful." << endl;

    // Step 2: Fill socket struct
    TCPServerAdd.sin_family = AF_INET;                      // IPv4 address family
    TCPServerAdd.sin_addr.s_addr = inet_addr(DEFAULT_ADDR); // set server address
    TCPServerAdd.sin_port = htons(DEFAULT_PORT);            // bind to port, convert from host byte order to network byte order

    // Step 3: Create socket
    TCPServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(TCPServerSocket == INVALID_SOCKET){
        cout << "TCP server socket creation failed with error " << WSAGetLastError() << endl;
        return 1;
    }
    cout << "TCP server socket creation successful." << endl;

    // Step 4: Bind 
    iBind = bind(TCPServerSocket, (sockaddr*)&TCPServerAdd, sizeof(TCPServerAdd)); // associate local address with socket
    if(iBind == SOCKET_ERROR){
        cout << "Binding failed with error " << WSAGetLastError() << endl;
        return 1;
    }
    cout << "Binding successful." << endl;

    // Step 5: Listen for incoming connections
    iListen = listen(TCPServerSocket, INT_MAX);
        // backlog = INT_MAX = number of connections to create
    if(iListen == SOCKET_ERROR){
        cout << "Listen failed with error " << WSAGetLastError() << endl;
        return 1;
    }
    cout << "Listen successful." << endl;

    // Step 6: Accept connection
    // sAcceptSocket = accept(TCPServerSocket, (sockaddr*)&TCPClientAdd, &iTCPClientAdd);
    // if(sAcceptSocket == SOCKET_ERROR){
    //     cout << "Accept failed with error " << WSAGetLastError() << endl;
    //     return 1;
    // }
    // int i = 1;
    // cout << "-- SOCKET" << i <<" connection accepted. --" << endl;
    // cout << endl;

    // /* make acceptor socket non-blocking */
    // u_long iMode = 1; // non-blocking
    // int iResult = ioctlsocket(TCPServerSocket, FIONBIO, &iMode);
    // if (iResult != NO_ERROR)
    // cout << "ioctlsocket failed with error " << iResult << endl;

    // Step 7: Receive data from client
    /* ref: https://www.cplusplus.com/forum/windows/59255/ */
    /* ref: https://stackoverflow.com/questions/15185380/tcp-winsock-accept-multiple-connections-clients */
    int sockNum = 1;
    while (1)
    {
        sAcceptSocket = SOCKET_ERROR;
        while(sAcceptSocket == SOCKET_ERROR)
        {
            sAcceptSocket = accept(TCPServerSocket, (sockaddr*)&TCPClientAdd, &iTCPClientAdd);
        }
        cout << "SOCKET " << sockNum++ << " connected." << endl;
        DWORD dwThreadId;
        CreateThread(NULL, 0, MyRecv, (LPVOID) sAcceptSocket, 0, &dwThreadId);
    }

    // Step 8: Close socket
    // iCloseSocket = closesocket(TCPServerSocket);
    // if(iCloseSocket == SOCKET_ERROR){
    //     cout << "Closing socket failed with error " << WSAGetLastError() << endl;
    //     return 1;
    // }
    // cout << "Socket closed successfully." << endl;

    // Step 9: Cleanup Winsock DLL
    iWsaCleanup = WSACleanup();
    if(iWsaCleanup == SOCKET_ERROR){
        cout << "Cleanup failed with error " << WSAGetLastError() << endl;
        return 1;
    }
    cout << "Cleanup successful." << endl;

    return 0;
}

DWORD WINAPI MyRecv(LPVOID lpParameter){
    SOCKET acceptSocket = (SOCKET) lpParameter;
    
    // Receive data.
    int iRecv = SOCKET_ERROR;
    char RecvBuffer[1024] = "";
    char TimeBuffer[128];

    while(1)
    {
        _strtime(TimeBuffer);
        ZeroMemory(RecvBuffer, sizeof(RecvBuffer));

        iRecv = recv(acceptSocket, RecvBuffer, sizeof(RecvBuffer), 0);
        cout << TimeBuffer << " Client sent: " << RecvBuffer << endl;
        CalcChecksum(RecvBuffer);
        cout << endl;

        if (iRecv == 0){
            cout << "Connection closed." << endl;
            closesocket(acceptSocket);
            exit(0);
        }    
        else if (iRecv == -1)
        {
            // if(WSAGetLastError() == WSAEMSGSIZE) // client has more data to send 
            // {
            //     cout << "continue" << endl;
            //     continue; // iterate again to get more data
            // } 
            // else{
                // some other error
                cout << "Server receive failed with error " << WSAGetLastError() << endl;
                return 0;
            //}
        }
    }
}
        
int CalcChecksum(char buffer[DEFAULT_BUFLEN]){
    // variables
    int count;
    int Sum = 0;
    // checks if the count is less than te default buffer 
    // adds the total sum
    //  switches the sums's sign 
    for (count = 0; count < DEFAULT_BUFLEN; count++)
        Sum = Sum + buffer[count];
  
    cout << "checksum = " << Sum << endl;
    // returns sum 
    return (Sum );
}