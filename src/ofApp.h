#pragma once

#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxTrueTypeFontUC.h"
#include "ofxOpenCv.h"
#define _USE_LIVE_VIDEO	
#include "ofxJSON.h"
#include "CustomParticle.h"
#include "Detection.h"

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
    string music_file_name;
    ofSoundPlayer music;
    float synthPosition;
    float margin_time;

    // io
    string json_file_name;
    ofxJSONElement sync_lyric_json;
    string font_file_name;
    ofxTrueTypeFontUC font;
    int font_size;

    // animation
    int window_width;
    int window_height;
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

    double density;
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
    
    void jumpPopcones(int d);
    
    int 				threshold;
    bool				bLearnBakground;
    float               width;
    float               height;
    double              motionCount = 0;
    int                 lastJumpTime = 0;
    int                 drawCount = 0;
    
    int tracking_interval;
    int diff_param;
    int number_of_object;
    
    const double INF = (1 << 27);
};
