#pragma once

#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxTrueTypeFontUC.h"
#include "ofxOpenCv.h"
#define _USE_LIVE_VIDEO	
#include "ofxJSON.h"
#include "CustomParticle.h"
#include "Detection.h"
#include "ServoControllerThread.h"


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
    
    void drawResult();
    
    bool bMouseForce;
    
    // my method
    vector<shared_ptr<CustomParticle>> getLineObj(int line_index), getCustomObj(int line_index, int x, int y);
    
    // sounds
    string music_file_name;
    ofSoundPlayer music;
    float synthPosition;
    float margin_time           = 300;

    // io
    string json_file_name;
    ofxJSONElement sync_lyric_json;
    string font_file_name;
    ofxTrueTypeFontUC font;
    int font_size               = 40;

    // animation
    int window_width;
    int window_height;
    double drop_point_x         = 100;
    double drop_point_y         = 350;
    double start_point_x        = 200;
    double start_point_y        = 250;
    float radius_fix_pram       = 0.6;

    // viewer
    int now_lyric_line          = 0;
    int loaded_line_head        = 0;
    int preload_number          = 3;
    int word_margin             = 20;
    double next_lyric_ms;

    vector<vector<shared_ptr<CustomParticle>>> viewable_particles, result_viewable_particles;
    
    ofxBox2d box2d;
    ofPolyline drawing, groundLine, cupLine;
    ofxBox2dEdge edgeLine;
    
    vector <ofImage> images;
   
    // result
    ofImage yaneA, yaneB, yaneC, textA, textB, textC, first, second, third;
    vector<shared_ptr<ofxBox2dCircle> > circles, pop ;
    vector <ofPolyline> lines;
    ofPtr<ofxBox2dPolygon> cup;
    
    float min_popcone_size = 20;
    float max_popcone_size = 30;

    const double density                          = 0.5;
    const double bounce                           = 0.4;
    double friction;
    const double gravity                          = 25;
    const double pop_power                        = 600;
    
    // camera
    double camera_draw_opacity              = 0.7;
    ofVideoGrabber 		vidGrabber;
    ofxCvColorImage			colorImg;
    ofxCvGrayscaleImage 	grayImage;
    ofxCvGrayscaleImage 	grayBg;
    ofxCvGrayscaleImage 	grayDiff;
    ofxCvContourFinder 	contourFinder;
    ofxCvContourFinder 	lastContourFinder;
    
    // servo controller thread various
    int rotate_degree                               = 0;
    ServoControllerThread servo_thread;
    int sensor_interval_ms                          = 10000;
    int next_execute_time;
    bool execute_flag;
    void threadUpdate();
    
    
    void jumpPopcones(int d);
    
    const int 				threshold                   = 80;
    bool				bLearnBakground             = true;
    float               width                       = 320;
    float               height                      = 240;
    double              motionCount = 0;
    int                 lastJumpTime = 0;
    int                 drawCount = 0;
    int                 loopCnt = 1;
    int                 drop_count_a = 0;
    int                 drop_count_b = 0;
    int                 drop_count_c = 0;
    const int           judgePoint = 50;
    const int           resultBeginTime = 211000;
    const int           feverBeginTime = 190000;
    bool                feverTimeFlag = false;
    bool                resultTimeFlag = false;
    bool                resultTimeFlagment = false;
    bool                resultCalcuratedA = false;
    bool                resultCalcuratedB = false;
    bool                resultCalcuratedC = false;
    bool                isCalcurating = false;
    const double tracking_interval                = 1.5;
    const double diff_param                       = 1.5;
    const double number_of_object                 = 2;
    
    const double INF = (1 << 27);
    
    // for result ranking view
    int area_a = 0,area_b = 0,area_c = 0;
    int pop_a, pop_b, pop_c;
    int rank1, rank2, rank3;
    string current_area_name                = "A";
    double area_img_expanded                = 0.3;
    
    const int w_size = 1000;
    const int h_size = 800;
    
    // fevertime
    ofImage snow_img;
    ofImage fevertime_img;
    float xpos, ypos;
    float xspeed;
    bool flag_motion;
    vector <ofImage> area_images;
    ofxBox2dCircle* area_circle_obj;
    
    ofImage img;
    ofxCvHaarFinder finder;
};
