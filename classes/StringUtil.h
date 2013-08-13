//
//  StringUtil.h
//  Crowd
//
//  Created by Victor Norgren on 8/8/13.
//
//

#ifndef Crowd_StringUtil_h
#define Crowd_StringUtil_h


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

#endif
