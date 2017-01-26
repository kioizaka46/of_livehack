#include "ServoControllerThread.h"
#include "ofConstants.h"

ServoControllerThread::ServoControllerThread()
:newFrame(true){
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
    
}

void ServoControllerThread::draw(){
    
}

void ServoControllerThread::threadedFunction(){
    while (toSending.receive(rotate_deg)) {
        //UEKUSA
        //        string command = "ssh -i /Users/tmk-mac/Desktop/livehack_raspberrypi pi@169.254.184.112 'sudo python3 /home/pi/servo_controller.py " + ofToString(rotate_deg) + "'";
        //string command = "ssh -i /Users/tuekusa/.ssh/id_rsa pi@169.254.184.112 'sudo python3 /home/pi/servo_controller.py " + ofToString(rotate_deg) + "'";
        //HIGASHI
        string command = "ssh -i /Users/yhigashi/Desktop/livehack_raspberrypi_yhigashi pi@169.254.184.112 'sudo python3 /home/pi/servo_controller.py " + ofToString(rotate_deg) + "'";

        system(command.c_str());
    }
}
