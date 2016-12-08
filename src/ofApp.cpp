#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup() {
    // load images
    ofDirectory dir;
    ofDisableArbTex();
    int n = dir.listDir("popcones");
    for (int i=0; i<n; i++) {
        images.push_back(ofImage(dir.getPath(i)));
	}
    
    dencity     = 0.1;
    bounce      = 0.3;
    friction    = 0.3;
    gravity     = 60;
    pop_power   = 200;
    
    ofSetVerticalSync(true);
    ofBackgroundHex(0xfdefc2);
    ofSetLogLevel(OF_LOG_NOTICE);
    
    bMouseForce = false;
    
    box2d.init();
    box2d.setGravity(0, gravity);
    box2d.createGround();
    box2d.setFPS(60.0);
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
        float r = ofRandom(min_popcone_size, max_popcone_size);
        customParticles.push_back(shared_ptr<CustomParticle>(new CustomParticle(images, "あ")));
        CustomParticle * p = customParticles.back().get();
        
        p->setPhysics(dencity, bounce, friction);
        p->setup(box2d.getWorld(), mouseX, mouseY, r);
    }
    if (key == 'a') {
        for(int i = 0; i < customParticles.size(); i++){
            float vec_x = customParticles[i].get()->getPosition().x;
            float vec_y = customParticles[i].get()->getPosition().y;
            customParticles[i].get()->addRepulsionForce(vec_x, vec_y + 50, pop_power);
        }
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
