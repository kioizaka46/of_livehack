#pragma once
#include "ofMain.h"

class ServoControllerThread: public ofThread {
public:
    ServoControllerThread();
    ~ServoControllerThread();
    void sending(int rotate_deg);
    void update(int deg, int interval);
    bool isFrameNew();
    void draw();
    
private:
    void threadedFunction();
    ofThreadChannel<int> toSending;
    ofThreadChannel<int> sended;
    int rotate_deg;
    bool newFrame;
    bool received;
};
