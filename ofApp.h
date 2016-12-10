#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxTrueTypeFontUC.h"
#include "ofxJSON.h"

struct TextSymbol {
    std::string text;
    double start_time;
    double end_time;
    int size;
    double pos_x;
    double pos_y;
    bool is_draw;
};

class CustomCircle : public ofxBox2dCircle {
public:
    CustomCircle();
    void update(); // refresh
    float counter; //count
    float phase; // init
    int lifeTime; // life time
    bool dead;// isDead
    
};


// ------------------------------------------------- a simple extended box2d circle
class CustomParticle : public ofxBox2dCircle {
    
public:
    CustomParticle(vector<ofImage> images, string txt) {
        imgs = images;
        text = txt;
        font.load("SmartFontUI.ttf", 50, true, true);
        bake_level = ofRandomuf();
        image_count = images.size();
    }
    // openframeworks obj
    ofColor color;
    vector<ofImage> imgs;
    ofxTrueTypeFontUC font;
    
    // param
    float bake_level = 0.0; // 0.0~1.0
    int image_count;
    string text;
    
    void draw() {
        float radius = getRadius();
        
        glPushMatrix();
        glTranslatef(getPosition().x, getPosition().y, 0);
        
        int tmp_img_num = (int)(image_count + 1) * bake_level;
        if (tmp_img_num == 0){
            font.drawString(text, 0, 0);
        } else if (tmp_img_num == 1) {
            imgs[tmp_img_num - 1].draw(-radius/2,-radius/2, radius*2, radius*2);
        } else {
            imgs[tmp_img_num - 1].draw(-radius/2,-radius/2, radius*2, radius*2);
        }
        
        glPopMatrix();
    }
};
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
