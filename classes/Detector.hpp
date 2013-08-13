//
//  Detector.hpp
//  Detector
//
//  Created by Victor Norgren on 8/11/13.
//
//

#ifndef Detector_Detector_hpp
#define Detector_Detector_hpp

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <cassert>
#include <iostream>

#include "DataHandler.hpp"
#include "Person.hpp"

class Detector
{
public:
    Detector();
    ~Detector();
private:

    // Websocket
    DataHandler server;
    
    // OpenCV
    CvCapture *capture;
    cv::CascadeClassifier cascade;
    CvMemStorage *storage;
    cv::Mat current_frame;
    cv::Mat draw_image;
    cv::Mat small_image;
    int captureFPS;
    int captureCount;
    std::list<Person> personList;

    // FPS meter
    time_t fps_start;
    time_t fps_end;
    int fps_counter;

    // Utility functions
    float dist(int x1, int y1, int x2, int y2);
    bool in_range(int input, int min, int max);
    double map(double x, double in_min, double in_max, double out_min, double out_max);

    // Class methods
    void initOpenCV();
    void runLoop();
    void detectAndDisplay();
    void displayFPS();
};

#endif
