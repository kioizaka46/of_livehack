#include "ServoControllerThread.h"
#include "ofConstants.h"

ServoControllerThread::ServoControllerThread()
:newFrame(true){
    execute_flag = true;
    next_execute_time = 0;

    startThread();
}

ServoControllerThread::~ServoControllerThread(){
    toSending.close();
    sended.close();
    waitForThread(true);
}

void ServoControllerThread::sending(int rotate_deg){
    toSending.send(rotate_deg);
}

void ServoControllerThread::update(int deg, int interval){
    // toggle flag
    if(ofGetElapsedTimeMillis() >= next_execute_time) {
        execute_flag = true;
    }
    if(execute_flag){
        sending(deg);
        
        next_execute_time += interval;
        execute_flag = false;
    }
}

void ServoControllerThread::draw(){
    
}

void ServoControllerThread::threadedFunction(){
    while (toSending.receive(rotate_deg)) {
        string command = "ssh -i /Users/tmk-mac/Desktop/livehack_raspberrypi pi@169.254.184.112 'sudo python3 /home/pi/servo_controller.py " + ofToString(rotate_deg) + "'";
        system(command.c_str());
    }
}
