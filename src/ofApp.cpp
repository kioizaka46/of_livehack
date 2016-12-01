#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
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
                TextSymbol ts;
                ts.size = 10;
                ts.start_time = sync_lylic_json["lines"][i]["words"][j]["start"].asDouble();
                ts.end_time = sync_lylic_json["lines"][i]["words"][j]["end"].asDouble();
                ts.text = sync_lylic_json["lines"][i]["words"][j]["string"].asString();
                ts.pos_x = start_point_x;
                ts.pos_y = start_point_y;
                ts.is_draw = true;
                text_symbols.push_back(ts);
            }
        }
    }else{
        ofLogError("ofApp::setup")  << "Failed to parse JSON" << endl;
    }
    music.play();
}
//--------------------------------------------------------------
void ofApp::update(){
    ofSoundUpdate();
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(155,155,155);
    ofSetHexColor(0x000000);
    
    float music_pos = music.getPositionMS();
    for(int i = 0; i< text_symbols.size(); i++){
        text_symbols[i].pos_x = text_symbols[i].end_time - music_pos;
        font.drawString(text_symbols[i].text, text_symbols[i].pos_x, text_symbols[i].pos_y);
    }
    font.drawString("|",drop_point_x,drop_point_y);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
