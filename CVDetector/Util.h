//
//  Util.h
//  CVDetector
//
//  Created by Victor Norgren on 16/11/13.
//  Copyright (c) 2013 Victor Norgren. All rights reserved.
//

#ifndef CVDetector_Util_h
#define CVDetector_Util_h

#include <CoreFoundation/CoreFoundation.h>

inline float dist(int x1, int y1, int x2, int y2) {
    return sqrt(((x2-x1)*(x2-x1)) + (y2-y1)*(y2-y1)); // (euclidian distance) sqrt((x1*x2) + (y1*y2))
}

inline bool in_range(int input, int min, int max) {
    if (input <= 0)
        return true;
    while (input < min) {
        min /= 10;
        max /= 10;
    }
    return input <= max;
}

inline double map(double x, double in_min, double in_max, double out_min, double out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

char* CFStringCopyUTF8String(CFStringRef aString) {
    if(aString == NULL) {
        return NULL;
    }
    
    CFIndex length = CFStringGetLength(aString);
    CFIndex maxSize =
    CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8);
    char* buffer = (char*)malloc(maxSize);
    if(CFStringGetCString(aString, buffer, maxSize, kCFStringEncodingUTF8)) {
        return buffer;
    }
    free( buffer );
    buffer = NULL;
    return NULL;
}

static const char alphanum[] = "0123456789"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

int stringLength = sizeof(alphanum) - 1;
char genRandom() {
    return alphanum[rand() % stringLength];
}

std::string randomString() {
    std::string rnd;
    for(unsigned int i = 0; i < 5; ++i) {
        rnd += genRandom();
    }
    return rnd;
}

#endif
