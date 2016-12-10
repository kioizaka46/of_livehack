#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxTrueTypeFontUC.h"
#include "ofxOpenCv.h"

#define _USE_LIVE_VIDEO	

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
