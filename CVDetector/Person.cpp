//
//  Person.cpp
//  CVDetector
//
//  Created by Victor Norgren on 16/11/13.
//  Copyright (c) 2013 Victor Norgren. All rights reserved.
//

#include "Person.hpp"

#define TIMER 20;

Person::Person(cv::Rect rectangle, int id, std::string idString) {
    this->visibleFrames = 0;
    this->timer = TIMER;
    this->available = true;
    this->rectangle = rectangle;
    this->idString = idString;
    this->id = id;
};
Person::~Person() {
}

void Person::update(cv::Rect rectangle) {
    if(dead()) {
        return;
    }
    this->timer = TIMER;
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