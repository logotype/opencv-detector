//
//  Person.cpp
//  Crowd
//
//  Created by Victor Norgren on 8/7/13.
//
//

#include "Person.hpp"

Person::Person() {
    this->visibleFrames = 0;
    this->timer = 10;
    this->available = true;
    this->x = 0;
    this->y = 0;
};
Person::~Person() {
}

void Person::create( int x, int y, int width, int height, cv::Rect rectangle ) {
    if(dead()) {
        return;
    }
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->rectangle = rectangle;
    //std::cout << "Created, x: " << x << ", y: " << y << ", width: " << width << ", height: " << height << "\n";
}

void Person::update( int x, int y, int width, int height, cv::Rect rectangle ) {
    if(dead()) {
        return;
    }
    this->timer = 50;
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->rectangle = rectangle;
}

void Person::countDown() {
    if(dead()) {
        return;
    }
    this->timer--;
}

void Person::kill() {
    this->timer = 0;
}

bool Person::dead() {
    if (this->timer <= 0) return true;
    return false;
}