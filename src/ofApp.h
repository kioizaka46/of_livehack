#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxTrueTypeFontUC.h"
#include "ofxJSON.h"

class CustomLylic;

struct TextSymbol {
    std::string text;
    double start_time;
    double end_time;
    int size;
    double pos_x;
    double pos_y;
    bool is_draw;
};

class CustomParticle : public ofxBox2dCircle {
    
public:
    // param
    ofColor color;
    vector<ofImage> imgs;
    ofxTrueTypeFontUC font;
    float bake_level = 0.0; // 0.0~1.0
    int image_count;
    string text;
    float start_time;
        
    CustomParticle(vector<ofImage> images, string txt, float st_time) {
        imgs = images;
        text = txt;
        font.load("SmartFontUI.ttf", 50, true, true);
        //        bake_level = ofRandomuf();
        bake_level = 0;
        image_count = images.size();
        start_time = st_time;
    }
void draw() {
        float radius = getRadius();
        
        glPushMatrix();
        glTranslatef(getPosition().x, getPosition().y, 0);
        
        int tmp_img_num = (int)(image_count + 1) * bake_level;
        if (tmp_img_num == 0){
            ofSetColor(255,255,255);
            font.drawString(text, 0, 0);
        } else if (tmp_img_num == 1) {
            ofSetColor(255,255,255);
            imgs[tmp_img_num - 1].draw(-radius/2,-radius/2, radius*2, radius*2);
        } else {
            ofSetColor(255,255,255);
            imgs[tmp_img_num - 1].draw(-radius/2,-radius/2, radius*2, radius*2);
        }
        
        glPopMatrix();
    }
};


// ------------------------------------------------- a simple extended box2d circle

// -------------------------------------------------
class ofApp : public ofBaseApp {
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void resized(int w, int h);
    
    bool bMouseForce;
    
    // sounds
    ofSoundPlayer music;
    float synthPosition;
    
    // io
    ofxJSONElement sync_lylic_json;
    ofxTrueTypeFontUC font;
    
    // lyrics
    std::vector<TextSymbol> text_symbols;
    string view_lylic;
    
    // animation
    double drop_point_x;
    double drop_point_y;
    double start_point_x;
    double start_point_y;
    
    ofxBox2d box2d;
    ofPolyline drawing;
    ofxBox2dEdge edgeLine;
    vector <shared_ptr<CustomParticle> > customParticles;
    vector<shared_ptr<CustomParticle> > tmp_line;
    vector<vector<shared_ptr<CustomParticle>> > lylic;
    
    ofImage image0;
    ofImage image1;
    vector <ofImage> images;
    
    float min_popcone_size = 20;
    float max_popcone_size = 30;
    
    double dencity;
    double bounce;
    double friction;
    double gravity;
    double pop_power;
};
