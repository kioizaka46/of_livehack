#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup() {
    ofSetVerticalSync(true);
    ofBackgroundHex(0xfdefc2);
    ofSetLogLevel(OF_LOG_NOTICE);
    
    bMouseForce = false;
    
    box2d.init();
    box2d.setGravity(0, 30);
    box2d.createGround();
    box2d.setFPS(100.0);
    box2d.registerGrabbing();
    box2d.createBounds(0, 0, ofGetWidth(), ofGetHeight());

}
//--------------------------------------------------------------
void ofApp::update() {
    
    box2d.update();
    
    if(bMouseForce) {
        float strength = 1.0f;
        float damping  = 1.0f;
        for(int i=0; i<customParticles.size(); i++) {
            customParticles[i].get()->addAttractionPoint(mouseX, mouseY, strength);
            customParticles[i].get()->setDamping(damping, damping);
        }
    }
}


//--------------------------------------------------------------
void ofApp::draw() {
    for(int i=0; i<customParticles.size(); i++) {
        customParticles[i].get()->draw();
    }
    ofFill();
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    
    if(key == 'c') {
        customParticles.push_back(shared_ptr<CustomParticle>(new CustomParticle));
        CustomParticle * p = customParticles.back().get();
        float r = ofRandom(50, 51);
        p->setPhysics(0.1, 0.9, 0.5);
        p->setup(box2d.getWorld(), mouseX, mouseY, r);
        //p->color.r = ofRandom(20, 100);
        //p->color.g = 0;
        //p->color.b = ofRandom(150, 255);
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
    
}

//--------------------------------------------------------------
void ofApp::resized(int w, int h){
    
}
