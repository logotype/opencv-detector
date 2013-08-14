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

#endif /* defined(__Crowd__Person__) */
