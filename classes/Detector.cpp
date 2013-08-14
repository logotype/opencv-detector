#include "Detector.hpp"
#include "Util.h"

const char* WINDOW_NAME = "Detector";
using namespace std;

//#define DEBUG 1
#define IMAGE_SCALE 1
#define IMAGE_WIDTH 1280.0
#define IMAGE_HEIGHT 720.0
#define OBJECT_MINSIZE 140
#define OBJECT_MAXSIZE 400
#define OBJECT_SIZE_CHANGE_RANGE 200
#define OBJECT_MOVEMENT_DISTANCE 200
#define OBJECT_MINIMUM_VISIBLE_FRAMES 20

Detector::Detector() {
    
    // Start a thread to run the processing loop
    thread t(bind(&DataHandler::process_messages,&server));
    
    // Run the asio loop in a separate thread
    thread s(bind(&DataHandler::run,&server));
    
    // FPS
    time(&fps_start);
    
    fps_counter = 0;
    captureCount = 0;
    
    initOpenCV();
    runLoop();
}

void Detector::initOpenCV() {
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    assert(mainBundle);
    
#ifdef DEBUG
    capture = cvCaptureFromAVI("vid_trailer.mov"); // full path
    captureFPS = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
#else
    capture = cvCreateCameraCapture(CV_CAP_ANY);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, IMAGE_WIDTH);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, IMAGE_HEIGHT);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FPS, 30);
#endif
    
    // Get Resources URL
    CFURLRef url = CFBundleCopyResourceURL(mainBundle, CFSTR("haarcascade_frontalface_alt2"), CFSTR("xml"), NULL);
    assert(url);
    
    // Get a mutable string and remove localhost
    CFMutableStringRef urlWithLocalhost = CFStringCreateMutableCopy(NULL, 0, CFURLGetString(url));
    CFStringFindAndReplace(urlWithLocalhost, CFSTR("file://localhost"), CFSTR(""),(CFRange) { 0, CFStringGetLength(urlWithLocalhost)}, 0);
    
    char* filePath = CFStringCopyUTF8String(urlWithLocalhost);
    
    if(!cascade.load(filePath)){ printf("Failed to load cascade.\n"); };
    
    delete filePath;
    filePath = NULL;
    CFRelease(url);
    CFRelease(urlWithLocalhost);
    url = NULL;
    urlWithLocalhost = NULL;
    
    storage = cvCreateMemStorage(0);
    assert(storage);
    
    if(!capture)
        abort();
}

void Detector::runLoop() {
    for(;;)
    {
        current_frame = cv::cvarrToMat(cvQueryFrame(capture));
        
        detectAndDisplay();
        int key = cvWaitKey(1);
        if(key == 'q' || key == 'Q')
            break;
    }
}

void Detector::detectAndDisplay() {
    
    std::vector<cv::Rect> faces;
    cv::Mat frame_gray;
    
    cvtColor(current_frame, frame_gray, CV_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);
    cv::resize(frame_gray, small_image, cv::Size(current_frame.size().width / IMAGE_SCALE, current_frame.size().height / IMAGE_SCALE));
    cv::flip(small_image, small_image, 1);
    
    cascade.detectMultiScale(small_image, faces, 1.1, 2, 0, cvSize(OBJECT_MINSIZE / IMAGE_SCALE, OBJECT_MINSIZE / IMAGE_SCALE), cvSize(OBJECT_MAXSIZE / IMAGE_SCALE, OBJECT_MAXSIZE / IMAGE_SCALE));
    
    draw_image = cv::Mat(current_frame);
    
    if(personList.empty()) {
        for(int i = 0; i < faces.size(); i++) {
            Person person = Person(faces[i], captureCount++, randomString());
            personList.push_front(person);
        }
        
    } else if(personList.size() <= faces.size()) {
        
        bool used[ faces.size() ];
        for(int i = 0; i < faces.size(); i++)
            used[i] = false;
        
        float record = 5000;
        list<Person>::iterator iterator;
        for(iterator = personList.begin(); iterator!= personList.end(); iterator++)
        {
            // find faces[index] that is closest to face f
            // set used[index] to true so that it can't be used twice
            bool doUpdate = false;
            int index = -1;
            
            for(int i = 0; i < faces.size(); i++) {
                float d = dist(faces[i].x, faces[i].y, iterator->rectangle.x, iterator->rectangle.y);
                //cout << "ADist: " << round(d) << ", Currently: " << personList.size() << " people tracked (" << faces.size() << "), ID: " << personList.begin()->id << ", x: " << personList.begin()->x << ", y: " << personList.begin()->y << "\n";
                
                if(d < record && d < OBJECT_MOVEMENT_DISTANCE && !used[i] && in_range(faces[i].width, iterator->rectangle.width - OBJECT_SIZE_CHANGE_RANGE, iterator->rectangle.width + OBJECT_SIZE_CHANGE_RANGE) && in_range(faces[i].height, iterator->rectangle.height - OBJECT_SIZE_CHANGE_RANGE, iterator->rectangle.height + OBJECT_SIZE_CHANGE_RANGE)) {
                    record = d;
                    index = i;
                    doUpdate = true;
                }
            }
            
            if(doUpdate) {
                // update Person object location
                used[index] = true;
                iterator->update(faces[index]);
            }
        }
        
        // add any unused faces
        for(int i = 0; i < faces.size(); i++) {
            if(!used[i]) {
                
                bool didOverlap = false;
                list<Person>::iterator iterator;
                for(iterator = personList.begin(); iterator!= personList.end(); iterator++) {
                    if (faces[i].x < (iterator->rectangle.x + iterator->rectangle.width) && (faces[i].x + faces[i].width) > iterator->rectangle.x &&
                        faces[i].y < (iterator->rectangle.y + iterator->rectangle.height) && (faces[i].y + faces[i].height) > iterator->rectangle.y) {
                        didOverlap = true;
                        break;
                    }
                }
                
                if(!didOverlap) {
                    // Add person in map
                    Person person = Person(faces[i], captureCount++, randomString());
                    personList.push_front(person);
                }
            }
        }
    } else {
        
        // all Person objects start out as available
        list<Person>::iterator iterator;
        for(iterator = personList.begin(); iterator!= personList.end(); iterator++)
        {
            iterator->available = true;
        }
        
        // match pos with a Person object
        float record = 5000;
        for(int i = 0; i < faces.size(); i++) {
            // find face object closest to faces[i]
            // set available to false
            int index = -1;
            int j = 0;
            
            bool doUpdate = false;
            list<Person>::iterator iterator;
            for(iterator = personList.begin(); iterator!= personList.end(); iterator++)
            {
                float d = dist(faces[i].x, faces[i].y, iterator->rectangle.x, iterator->rectangle.y);
                //cout << "BDist: " << round(d) << ", Currently: " << personList.size() << " people tracked (" << faces.size() << "), ID: " << personList.begin()->id << ", x: " << personList.begin()->x << ", y: " << personList.begin()->y << "\n";
                
                if(d < record && d < OBJECT_MOVEMENT_DISTANCE && in_range(faces[i].width, iterator->rectangle.width - OBJECT_SIZE_CHANGE_RANGE, iterator->rectangle.width + OBJECT_SIZE_CHANGE_RANGE) && in_range(faces[i].height, iterator->rectangle.height - OBJECT_SIZE_CHANGE_RANGE, iterator->rectangle.height + OBJECT_SIZE_CHANGE_RANGE)) {
                    record = d;
                    index = j;
                    doUpdate = true;
                }
                j++;
            }
            
            // update Person object location
            if(doUpdate) {
                list<Person>::iterator updatePerson = personList.begin();
                advance(updatePerson, index);
                
                updatePerson->available = false;
                updatePerson->update(faces[i]);
            }
        }
        // Check for overlaps, delete the least visible frame
        list<Person>::iterator deleteIterator = personList.begin();
        while (deleteIterator != personList.end())
        {
            bool didDelete = false;
            list<Person>::iterator iterator;
            for(iterator = personList.begin(); iterator!= personList.end(); iterator++)
            {
                if (deleteIterator->dead() || (deleteIterator != iterator && deleteIterator->visibleFrames < iterator->visibleFrames &&
                                               deleteIterator->rectangle.x < (iterator->rectangle.x + iterator->rectangle.width) && (deleteIterator->rectangle.x + deleteIterator->rectangle.width) > iterator->rectangle.x &&
                                               deleteIterator->rectangle.y < (iterator->rectangle.y + iterator->rectangle.height) && (deleteIterator->rectangle.y + deleteIterator->rectangle.height) > iterator->rectangle.y)) {
                    personList.erase(deleteIterator++);
                    break;
                    cout << "killed overlap" << endl;
                    captureCount--;
                    if(captureCount<0) {
                        captureCount = 0;
                    }
                    didDelete = true;
                } else {
                    deleteIterator->countDown();
                }
            }
            if(!didDelete) {
                ++deleteIterator;
            }
        }
    }
    
    if(personList.size() > 0) {
        
        stringstream iWidth;
        iWidth << IMAGE_WIDTH;
        
        stringstream iHeight;
        iHeight << IMAGE_HEIGHT;
        
        stringstream iScale;
        iScale << IMAGE_SCALE;
        
        string JSON = "{\n    \"frame\": ";
        JSON += "{\n"
        "        \"width\": " + iWidth.str() + ",\n"
        "        \"height\": " + iHeight.str() + ",\n"
        "        \"scale\": " + iScale.str() + "\n"
        "    },\n";
        
        JSON += "    \"tracking\": [\n";
        
        list<Person>::iterator lastElement = personList.end();
        --lastElement;
        
        list<Person>::iterator iterator;
        for(iterator = personList.begin(); iterator!= personList.end(); iterator++) {
            
            if(iterator->visibleFrames < OBJECT_MINIMUM_VISIBLE_FRAMES)
                continue;
            
            stringstream pID;
            pID << iterator->id;
            
            stringstream pIDString;
            pIDString << iterator->idString;
            
            stringstream pX;
            pX << fixed << setprecision(4) << ((small_image.size().width - iterator->rectangle.x) * IMAGE_SCALE) / IMAGE_WIDTH;
            
            stringstream pY;
            pY << fixed << setprecision(4) << (iterator->rectangle.y * IMAGE_SCALE) / IMAGE_HEIGHT;
            
            stringstream pW;
            pW << iterator->rectangle.width;
            
            stringstream pH;
            pH << iterator->rectangle.height;
            
            stringstream pV;
            pV << iterator->visibleFrames;
            
            JSON += "    {\n"
            "        \"id\": \"" + pID.str() + "\",\n"
            "        \"idString\": " + pIDString.str() + ",\n"
            "        \"x\": " + pX.str() + ",\n"
            "        \"y\": " + pY.str() + ",\n"
            "        \"width\": " + pW.str() + ",\n"
            "        \"height\": " + pH.str() + ",\n"
            "        \"visibleFrames\": " + pV.str() + "\n"
            "    }";
            
            if (iterator != lastElement) {
                JSON += ",\n";
            } else {
                JSON += "\n";
            }
            
            // draw from personList
            CvPoint p1;
            p1.x = (small_image.size().width - iterator->rectangle.x) * IMAGE_SCALE;
            p1.y = iterator->rectangle.y * IMAGE_SCALE;
            
            CvPoint p2;
            p2.x = (small_image.size().width - (iterator->rectangle.x + iterator->rectangle.width)) * IMAGE_SCALE;
            p2.y = (iterator->rectangle.y + iterator->rectangle.height) * IMAGE_SCALE;
            cv::rectangle(draw_image, p1, p2, CV_RGB(0, 255, 0), 1, 8, 0);
            
            ostringstream sstream;
            sstream << iterator->id;
            string idString = sstream.str();
            
            CvFont font;
            double hScale = .5;
            double vScale = .5;
            int lineWidth = 1;
            cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC, hScale, vScale, 0, lineWidth);
            
            CvPoint p3;
            p3.x = (small_image.size().width - iterator->rectangle.x - iterator->rectangle.width + 2) * IMAGE_SCALE;
            p3.y = (iterator->rectangle.y + 13) * IMAGE_SCALE;
            
            cv::putText(draw_image, idString.c_str(), p3, 1, 1, cvScalar(0, 255, 0));
        }
        
        JSON += "    ]\n}\n";
        
        //cout << "------------------------------------------" << endl << JSON;
        
        server.send_message(JSON);
    }
    
    // update visibleTime
    list<Person>::iterator iterator;
    for(iterator = personList.begin(); iterator!= personList.end(); iterator++) {
        iterator->visibleFrames++;
    }
    
    // draw from face db
    for(int i = 0; i < faces.size(); i++)
    {
        CvPoint center;
        int radius;
        center.x = cvRound((small_image.size().width - faces[i].width * 0.5 - faces[i].x) * IMAGE_SCALE);
        center.y = cvRound((faces[i].y + faces[i].height * 0.5) * IMAGE_SCALE);
        radius = cvRound((faces[i].width + faces[i].height) * 0.25 * IMAGE_SCALE);
    cv:circle(draw_image, center, radius, CV_RGB(255, 255, 255), 1, 8, 0);
    }
    
    displayFPS();
    //if(personList.size() > 0)
    //    cout << "Currently: " << personList.size() << " people tracked (" << faces.size() << "), ID: " << personList.begin()->id << ", x: " << personList.begin()->x << ", y: " << personList.begin()->y << "\n";
    
    imshow(WINDOW_NAME, draw_image);
}

void Detector::displayFPS() {
    
    // stop the clock and show FPS
    time(&fps_end);
    ++fps_counter;
    double sec = difftime(fps_end, fps_start);
    double fps = round(fps_counter / sec);
    
    ostringstream sstream;
    sstream << "FPS " << fps;
    string fpsString = sstream.str();
    
    CvFont font;
    double scale = 1.0;
    int lineWidth = 1;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC, scale, scale, 0, lineWidth);
    cv::putText(draw_image, fpsString.c_str(), cvPoint(10, 30), 1, 1, cvScalar(255, 255, 0));
}

Detector::~Detector() {
    cout << "destructor\n";
}