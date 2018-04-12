/////////////////////////////////////////////////////////////////////////////////////////
// TITI: Phone Server
// Author: Jeremy J. Hartmann
// EMAIL: j3hartma@uwaterloo.ca
/////////////////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////
#define WIN32_LEAN_AND_MEAN
// Simple client
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "45697"
/////////////////////////////

// Forward delclaration for YOLO Bounding Box
struct bbox_t;

// Forward Declarations
namespace cv {
    class Mat;
}


// Namespaces
using namespace std;

enum InteractionEventType
{
    ButtonEvent = 0,
    ConintuousTouchEvent = 1,
    DoubleTapEvent = 2,
    SingleTapEvent = 3,
    FlickEvent = 4,
    PinchEvent = 5,
    RotateEvent = 6
};

class TCPClient {
public:
    TCPClient(string host, string port, string friendlyName);
    ~TCPClient();

    bool IsConnected();

    void SendChar(char *msg);
    void SendMat(cv::Mat in_mat);
    void SendButtonEvent(InteractionEventType eType, int b_id, int b_val, string b_misc);
    void SendBoundingBoxData(bbox_t bbox);
    void TCPClient::SendBoundingBoxDataArray(vector<bbox_t> bbox_vec);

private:
    // Connection info
    string mServerFriendlyName;
    string mPort;
    string mHost;
    SOCKET ConnectSocket;

    // Connection Status. 
    bool mIsConnected;

    // Private methods. 
    void SendData(char *data, int len);

};

