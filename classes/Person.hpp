//
//  Person.h
//  Crowd
//
//  Created by Victor Norgren on 8/7/13.
//
//

#ifndef __Crowd__Person__
#define __Crowd__Person__

#include <opencv2/objdetect/objdetect.hpp>
#include <iostream>

class Person
{
public:
    Person();
    ~Person();

    int visibleFrames;
    int timer;
    bool available;
    std::string id;
    int idNumber;
    double x;
    double y;
    double width;
    double height;
    cv::Rect rectangle;
    
    void create( int x, int y, int width, int height, cv::Rect rectangle );
    void update( int x, int y, int width, int height, cv::Rect rectangle );
    void countDown();
    void kill();
    bool dead();
};

#endif /* defined(__Crowd__Person__) */
