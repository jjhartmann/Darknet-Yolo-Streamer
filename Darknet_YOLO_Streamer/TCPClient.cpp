/////////////////////////////////////////////////////////////////////////////////////////
// TITI: Phone Server
// Author: Jeremy J. Hartmann
// EMAIL: j3hartma@uwaterloo.ca
/////////////////////////////////////////////////////////////////////////////////////////

#include "TCPClient.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>



#define PREVENT_CONNECTION_ERROR(name) if (!mIsConnected)                 \
    {                                                               \
    std::cout << "Error: not connected to server " << name << std::endl;     \
    return;                                                         \
    }                                                               \



TCPClient::~TCPClient() {
    // shutdown the connection since no more data will be sent
    int iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
    }
}




TCPClient::TCPClient(string host, string port, string friendlyName) :
    mHost(host),
    mPort(port),
    mIsConnected(false),
    mServerFriendlyName(friendlyName)
{

    WSADATA wsaData;
    ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
        *ptr = NULL,
        hints;
    char *sendbuf = "this is a test";
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(mHost.c_str(), mPort.c_str(), &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }


    freeaddrinfo(result);


    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return;
    }



    // Set connected flag
    mIsConnected = true;

    // Test
    // int res = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);

}



// Send OpenCV Matrix
void TCPClient::SendMat(cv::Mat in_mat)
{
    PREVENT_CONNECTION_ERROR(mServerFriendlyName)

        if (in_mat.empty()) return;

    // Check contiguous memory
    if (!in_mat.isContinuous()) {
        in_mat = in_mat.clone();
    }

    cv::Size size = in_mat.size();
    int len = in_mat.total() * in_mat.elemSize();
    char *data = in_mat.ptr<char>(0);

    char *dataArray = new char[len + sizeof(int)];
    memcpy(dataArray, &len, sizeof(int));
    memcpy(dataArray + sizeof(int), data, len);

    SendData(dataArray, len + sizeof(int));

    delete dataArray;
}


// Send Message
void TCPClient::SendChar(char *msg) {
    PREVENT_CONNECTION_ERROR(mServerFriendlyName)
        int len = (int)strlen(msg);

    char *dataArray = new char[len + sizeof(int)];
    memcpy(dataArray, &len, sizeof(int));
    memcpy(dataArray + sizeof(int), msg, len);

    SendData(dataArray, len + sizeof(int));

    delete dataArray;

}


/////////////////////////////////////////////////////////////////////
// Send Event to Unity
//
// 1: B_id - Id of the button as described in the proto file. 
// 2: b_val - The val of the event (1-256)
// 3: b_misc - Miscellaneous data or message. (can be null)
//
void TCPClient::SendButtonEvent(InteractionEventType eType, int b_id, int b_val, string b_misc)
{
    PREVENT_CONNECTION_ERROR(mServerFriendlyName)

        int misclen = strlen(b_misc.c_str());
    int headerlen = sizeof(int) * 3;
    int totalLen = misclen + headerlen;

    // Unity TCPNetwork expects a header int witht the size of the message
    int offset = 0;
    char *dataArray = new char[totalLen + sizeof(int)];

    memcpy(dataArray, &totalLen, sizeof(totalLen));
    offset += sizeof(totalLen);

    memcpy(dataArray + offset, &eType, sizeof(InteractionEventType));
    offset += sizeof(InteractionEventType);

    memcpy(dataArray + offset, &b_id, sizeof(b_id));
    offset += sizeof(b_id);

    memcpy(dataArray + offset, &b_val, sizeof(b_val));
    offset += sizeof(b_val);

    memcpy(dataArray + offset, b_misc.c_str(), misclen);

    // Send to Unity
    SendData(dataArray, totalLen + sizeof(int));

    delete dataArray;
}


///////////////////////////////////////////////////////////////////////////////////////////////
//
// Private helper functions
//
///////////////////////////////////////////////////////////////////////////////////////////////
void TCPClient::SendData(char *data, int len) {

    // Send data
    int res = send(ConnectSocket, data, len, 0);


    if (res == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        cout << "Failed." << endl;
        return;
    }

    cout << "Success!!" << endl;
}





