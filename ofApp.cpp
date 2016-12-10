#include "ofApp.h"

CustomCircle::CustomCircle() {
    counter = 0;
    //phase = ofRandom(0, PI*2);
    lifeTime = 3;
    //生死の判定
    dead = false;
}

void CustomCircle::update()
{
    if (counter > lifeTime) {
        dead = true;
    }
    //年齢を追加
    counter++;
    printf("counter = %f", counter);
}

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
    gravity     = 10;
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
    
    // write song
    music.load("keyaki_silent_majority.mp3");
    music.setMultiPlay(true);
    
    font.loadFont("SmartFontUI.ttf", 100, true, true);
    std::string file = "sync_lylic_silent_majority.json";
    
    drop_point_x = 100.0;
    drop_point_y = 350.0;
    start_point_x = 1024.0;
    start_point_y = 350.0;
    
    bool parsingSuccessful = sync_lylic_json.open(file);
    
    if (parsingSuccessful){
        for(int i=0; i < sync_lylic_json["lines"].size(); i++){
            for(int j=0; j < sync_lylic_json["lines"][i]["words"].size(); j++){
                for(int k=0; k < sync_lylic_json["lines"][i]["words"].size(); k++) {
                TextSymbol ts;
                ts.size = 10;
                ts.start_time = sync_lylic_json["lines"][i]["time"].asDouble();
                ts.text = sync_lylic_json["lines"][i]["words"][k]["string"].asString();
                ts.pos_x = start_point_x;
                ts.pos_y = start_point_y;
                ts.is_draw = true;
                text_symbols.push_back(ts);
                }}
        }
    }else{
        ofLogError("ofApp::setup")  << "Failed to parse JSON" << endl;
    }
    music.play();
    music.setPositionMS(20000);

}
//--------------------------------------------------------------
void ofApp::update() {
    box2d.update();
    ofSoundUpdate();
    
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
    
    float music_pos = music.getPositionMS();
    for(int i = 0; i< text_symbols.size(); i++){
        if (music_pos > text_symbols[i].start_time) {
            font.drawString(text_symbols[i].text, font.stringWidth("test")/4, 400);
        }
    }
    //font.drawString("(",drop_point_x,drop_point_y);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if(key == 'c') {
        float r = ofRandom(min_popcone_size, max_popcone_size);
        customParticles.push_back(shared_ptr<CustomParticle>(new CustomParticle(images, "")));
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
