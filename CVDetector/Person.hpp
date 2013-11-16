//
//  Person.h
//  CVDetector
//
//  Created by Victor Norgren on 16/11/13.
//  Copyright (c) 2013 Victor Norgren. All rights reserved.
//

#ifndef __CVDetector__Person__
#define __CVDetector__Person__

#include <opencv2/objdetect/objdetect.hpp>
#include <iostream>

class Person
{
public:
    Person(cv::Rect rectangle, int id, std::string idString);
    ~Person();
    
    int visibleFrames;
    int timer;
    bool available;
    std::string idString;
    int id;
    cv::Rect rectangle;
    
    void update(cv::Rect rectangle);
    void countDown();
    void kill();
    bool dead();
};

#endif /* defined(__CVDetector__Person__) */
