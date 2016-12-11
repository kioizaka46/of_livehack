#pragma once

#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxTrueTypeFontUC.h"
#include "ofxOpenCv.h"

#define _USE_LIVE_VIDEO	
#include "ofxJSON.h"

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
        int font_size;
        float opacity;

    CustomParticle(vector<ofImage> images, string txt, float st_time, int f_size) {
        imgs = images;
        text = txt;
        font_size = f_size;
        font.load("yugothicbold.otf", font_size, true, true);
//        bake_level = ofRandomuf();
        bake_level = 0.0;
        image_count = images.size();
        start_time = st_time;
        opacity = 1.0;
    }
    void draw() {
        float radius = getRadius();
        float image_expand = 1.2;

        glPushMatrix();
        glTranslatef(getPosition().x, getPosition().y, 0);


        glRotatef(getRotation(), 0, 0, 1.0f);

        int tmp_img_num = (int)(image_count + 1) * bake_level;
        if (tmp_img_num == 0){
            ofSetColor(0,0,0,255*opacity);
            font.drawString(text, 0 - (font_size * 0.5), 0 + (font_size * 0.5));
        } else if (tmp_img_num == 1) {
            ofSetColor(255,255,255,255*opacity);
            imgs[tmp_img_num - 1].draw(0 - (font_size * 0.5), 0 - (font_size * 0.5), font_size * image_expand, font_size * image_expand);
        } else {
            ofSetColor(255,255,255,255*opacity);
            imgs[tmp_img_num - 1].draw(0 - (font_size * image_expand *  0.5), 0 - (font_size * image_expand * 0.5), font_size * image_expand, font_size * image_expand);
        }

        // --------- Debug only---------
        // ofSetColor(155,155,155);
        // ofDrawCircle(0, 0, radius);
        // -----------------------------

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
    

    // my method
    vector<shared_ptr<CustomParticle>> getLineObj(int line_index);

    // sounds
    ofSoundPlayer music;
    float synthPosition;
    float margin_time;

    // io
    ofxJSONElement sync_lyric_json;
    ofxTrueTypeFontUC font;
    int font_size;

    // animation
    double drop_point_x;
    double drop_point_y;
    double start_point_x;
    double start_point_y;
    float radius_fix_pram;

    // viewer
    int now_lyric_line;
    int loaded_line_head;
    int preload_number;
    int word_margin;

    vector <shared_ptr<CustomParticle> > custom_particles;
    vector<shared_ptr<CustomParticle> > tmp_line;

    vector<vector<shared_ptr<CustomParticle>> > buffering_particles;
    vector<vector<shared_ptr<CustomParticle>> > viewable_particles;

    ofxBox2d box2d;
    ofPolyline drawing;
    ofxBox2dEdge edgeLine;

    vector <ofImage> images;

    float min_popcone_size = 20;
    float max_popcone_size = 30;

    double dencity;
    double bounce;
    double friction;
    double gravity;
    double pop_power;
    
    #ifdef _USE_LIVE_VIDEO
		  ofVideoGrabber 		vidGrabber;
    #else
		  ofVideoPlayer 		vidPlayer;
    #endif

    
    ofxCvColorImage			colorImg;
    ofxCvGrayscaleImage 	grayImage;
    ofxCvGrayscaleImage 	grayBg;
    ofxCvGrayscaleImage 	grayDiff;
    ofxCvContourFinder 	contourFinder;
    ofxCvContourFinder 	lastContourFinder;
    
    int motionVector(ofxCvContourFinder const& contourFinder,ofxCvContourFinder const& lastContourFinder);
    vector<pair<double,double> > getHolePoints(ofxCvContourFinder const& contourFinder);
    bool isSameMotion(pair<double,double> point,pair<double,double> lastPoint);
    double getDistance(pair<double,double> point,pair<double,double> lastPoint);
    int motionIndex(pair<double,double> point,pair<double,double> lastPoint);
    void jumpPopcones(int d);
    
    int 				threshold;
    bool				bLearnBakground;
    float               width;
    float               height;
    double              motionCount = 0;
    int                 lastJumpTime = 0;
    int                 drawCount = 0;
    const double INF = (1 << 27);
    
};
