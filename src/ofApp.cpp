#include "ofApp.h"

void ofApp::setup() {
    // load images
    ofDirectory dir;
    ofDisableArbTex();
    int n = dir.listDir("popcones");
    for (int i=0; i<n; i++) {
        images.push_back(ofImage(dir.getPath(i)));
    }
    
    // physic setting
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
    
    // draw setting
    drop_point_x = 100.0;
    drop_point_y = 350.0;
    start_point_x = 1024.0;
    start_point_y = 350.0;
    
    // load font
    font.loadFont("SmartFontUI.ttf", 50, true, true);
    std::string file = "sync_koi_hoshinogen.json";
    
    
    // parse json
    bool parsingSuccessful = sync_lylic_json.open(file);
    int count = 0;
    if (parsingSuccessful){
        for(int i=0; i < sync_lylic_json["lines"].size(); i++){
            for(int j=0; j < sync_lylic_json["lines"][i]["words"].size(); j++){
                TextSymbol ts;
                ts.size = 10;
                ts.start_time = sync_lylic_json["lines"][i]["time"].asDouble();
                ts.pos_x = start_point_x;
                ts.pos_y = start_point_y;
                ts.is_draw = false;
                ts.text = sync_lylic_json["lines"][i]["words"][j]["string"].asString();
                tmp_line.push_back(shared_ptr<CustomParticle>(new CustomParticle(images, ts.text, ts.start_time)));
                count++;
//                ofLogNotice() << count;
                text_symbols.push_back(ts);
            }
            lylic.push_back(tmp_line);
            if (count > 100) {
                break;
            }
        }
        for(int i = 0; i< lylic.size(); i++){
            for(int j = 0; j < lylic[i].size(); j++){
                lylic[i][j]->setup(box2d.getWorld(), 100+j*50, 200, 15);
            }
        }
    }else{
        ofLogError("ofApp::setup")  << "Failed to parse JSON" << endl;
    }
    
    // music load and play
    music.load("koi_hoshinogen.mp3");
    music.setMultiPlay(true);
    music.play();
    music.setPositionMS(10000);

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
    // ポップコーンの表示
    for(int i=0; i<customParticles.size(); i++) {
        customParticles[i].get()->draw();
        ofLogNotice() << customParticles[i].get()->getRotation();
    }
    ofFill();
    
    // 歌詞の表示
    string tmp_str = "";
    float music_pos = music.getPositionMS();
//    for(int i = 0; i< text_symbols.size(); i++){
//        if (music_pos > text_symbols[i].start_time) {
//            tmp_str = text_symbols[i].text;
//        }
//    }
    for(int i = 0; i< lylic.size(); i++){
        for(int j = 0; j < lylic[i].size(); j++){
            if (music_pos > lylic[i][j]->start_time) {
                lylic[i][j]->draw();
            }
        }
    }
    font.drawString(tmp_str, ofGetWidth()/2 - 300, 400); //表示場所は後で考えます
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if(key == 'c') {
        float r = ofRandom(min_popcone_size, max_popcone_size);
        customParticles.push_back(shared_ptr<CustomParticle>(new CustomParticle(images, "あ", 0)));
        CustomParticle * p = customParticles.back().get();
        
//        p->setPhysics(dencity, bounce, friction);
        p->setup(box2d.getWorld(), mouseX, mouseY, r);
    }
    if (key == 'a') {
        for(int i = 0; i < customParticles.size(); i++){
            float vec_x = customParticles[i].get()->getPosition().x;
            float vec_y = customParticles[i].get()->getPosition().y;
            customParticles[i].get()->addRepulsionForce(vec_x, vec_y + 50, pop_power);
        }
    }
    if (key == 'b') {
        for (int i = 0; lylic[0].size(); i++) {
            ofLogNotice() << lylic[0][i]->text;
            lylic[0][i].get()->setPhysics(dencity, bounce, friction);
            lylic[0][i].get()->setup(box2d.getWorld(), 100+1*50, 200, 15);
        }
//        for(int i = 0; i < lylic.size(); i++){
//            for(int j = 0; j < lylic[i].size(); j++){
//                lylic[i][j]->setPhysics(dencity, bounce, friction);
//            }
//        }
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
