// Darknet_YOLO_Streamer.cpp : Defines the entry point for the console application.
//
#pragma once
#include <iostream>
#include <iomanip> 
#include <string>
#include <vector>
#include <fstream>
#include "stdafx.h"

#define OPENCV

// OpenCV
#include <opencv2/opencv.hpp>			// C++
#include "opencv2/highgui/highgui_c.h"	// C
#include "opencv2/imgproc/imgproc_c.h"	// C

// Yolo
#include "yolo_v2_class.hpp"	// imported functions from DLL

// Load OpenCV Libs
#pragma comment(lib, "opencv_core2413.lib")
#pragma comment(lib, "opencv_imgproc2413.lib")
#pragma comment(lib, "opencv_highgui2413.lib")

// TCPClient
#include "TCPClient.h"
#define UNITY_YOLO_STREAM_IP "192.168.137.1"
#define UNITY_YOLO_STREAM_PORT "10045"

// YOLO Stream to Unity
TCPClient *UnityYoloStream;

#ifdef _WIN32
#include <windows.h>

void sleep(unsigned milliseconds)
{
    Sleep(milliseconds);
}
#else
#include <unistd.h>

void sleep(unsigned milliseconds)
{
    usleep(milliseconds * 1000); // takes microseconds
}
#endif

void draw_boxes(cv::Mat mat_img, std::vector<bbox_t> result_vec, std::vector<std::string> obj_names, unsigned int wait_msec = 0) {
    int iter = 0;
    for (auto &i : result_vec) {
        cv::Scalar color(60 + (iter * 10), 160 + (iter * 5), 260 - (iter * 20));
        cv::rectangle(mat_img, cv::Rect(i.x, i.y, i.w, i.h), color, 3);
        if (obj_names.size() > i.obj_id)
            putText(mat_img, obj_names[i.obj_id], cv::Point2f(i.x, i.y - 10), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, color);
        if (i.track_id > 0)
            putText(mat_img, std::to_string(i.track_id), cv::Point2f(i.x + 5, i.y + 15), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, color);
        iter++;
    }
    cv::imshow("window name", mat_img);
    //cv::waitKey(wait_msec);
}


void show_result(std::vector<bbox_t> const result_vec, std::vector<std::string> const obj_names) {
    for (auto &i : result_vec) {
        if (obj_names.size() > i.obj_id) std::cout << obj_names[i.obj_id] << " - ";
        std::cout << "obj_id = " << i.obj_id << ",  x = " << i.x << ", y = " << i.y
            << ", w = " << i.w << ", h = " << i.h
            << std::setprecision(3) << ", prob = " << i.prob << std::endl;
    }
}

std::vector<std::string> objects_names_from_file(std::string const filename) {
    std::ifstream file(filename);
    std::vector<std::string> file_lines;
    if (!file.is_open()) return file_lines;
    for (std::string line; file >> line;) file_lines.push_back(line);
    std::cout << "object names loaded \n";
    return file_lines;
}

int main(int argc, const char* argv[])
{
    std::string config = "";
    std::string weights = "";
    std::string cocodataset = "";
    if (argc < 3)
    {
        config = "yolo-voc.cfg";
        weights = "yolo-voc.weights";
        cocodataset = "voc.names";
        std::cout << "Not enough arguments" << std::endl;
    }
    else
    {
        cocodataset = argv[3];
        config = argv[1];
        weights = argv[2];
    }

    // Init TCP Cleint
    UnityYoloStream = new TCPClient(UNITY_YOLO_STREAM_IP, UNITY_YOLO_STREAM_PORT, "unity_yolo_stream");

    
    // Setup 
    std::cout << "Before Detector" << std::endl;
    Detector detector(config, weights);
    std::cout << "After Detector" << std::endl;

    auto obj_names = objects_names_from_file(cocodataset);

    cv::VideoCapture cap;
    cap.open(0);
    //cap.set(CV_CAP_PROP_FPS, 20);

    cv::Mat frame;

    while (cap.isOpened() && cap.read(frame))
    {
        //std::cout << "input image or video filename: ";

        //std::string filename = "D:\\GIT_DIR\\ANDROID_EVERYWHERE_PROJECTS\\Darknet_YOLO_Streamer\\config\\data\\dog.jpg";
        //std::cin >> filename;
        //if (filename.size() == 0) break;


        //try {
        //    std::string const file_ext = filename.substr(filename.find_last_of(".") + 1);
        //    if (file_ext == "avi" || file_ext == "mp4" || file_ext == "mjpg" || file_ext == "mov") {	// video file
        //        cv::Mat frame;
        //        detector.nms = 0.02;	// comment it - if track_id is not required
        //        for (cv::VideoCapture cap(filename); cap >> frame, cap.isOpened();) {
        //            std::vector<bbox_t> result_vec = detector.detect(frame, 0.2);
        //            result_vec = detector.tracking(result_vec);	// comment it - if track_id is not required

        //            draw_boxes(frame, result_vec, obj_names, 3);
        //            show_result(result_vec, obj_names);
        //        }
        //    }
        //    else {	// image file
        //        std::cout << filename << std::endl;
        //        cv::Mat mat_img = cv::imread(filename);
        //        std::vector<bbox_t> result_vec = detector.detect(mat_img);
        //        draw_boxes(mat_img, result_vec, obj_names);
        //        show_result(result_vec, obj_names);
        //    }
        //
        //}
        //catch (std::exception &e) { std::cerr << "exception: " << e.what() << "\n"; getchar(); }
        //catch (...) { std::cerr << "unknown exception \n"; getchar(); }


        
     



        try {
            detector.nms = 0.4;// 0.02;	// comment it - if track_id is not required
            std::vector<bbox_t> result_vec = detector.detect(frame, 0.2);
            result_vec = detector.tracking(result_vec);	// comment it - if track_id is not required

            // Send data to Unity
            if (UnityYoloStream->IsConnected())
            {
                UnityYoloStream->SendBoundingBoxDataArray(result_vec);
            }

            draw_boxes(frame, result_vec, obj_names, 0);
            show_result(result_vec, obj_names);

            
        }
        catch (std::exception &e) { std::cerr << "exception: " << e.what() << "\n"; getchar(); }
        catch (...) { std::cerr << "unknown exception \n"; getchar(); }
    

        if (cv::waitKey(1) == 'q')
        {
            break;
        }

        // Articifially Slow down stream
        
        sleep(100);
    }

    // Delete content
    delete UnityYoloStream;

    return 0;
}
