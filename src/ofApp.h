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

    double density;
    double bounce;
    double friction;
    double gravity;
    double pop_power;
    
    // camera
    double camera_draw_opacity;
    ofVideoGrabber 		vidGrabber;
    ofxCvColorImage			colorImg;
    ofxCvGrayscaleImage 	grayImage;
    ofxCvGrayscaleImage 	grayBg;
    ofxCvGrayscaleImage 	grayDiff;
    ofxCvContourFinder 	contourFinder;
    ofxCvContourFinder 	lastContourFinder;
    
    // servo controller thread various
    int rotate_degree;
    ServoControllerThread servo_thread;
    int sensor_interval_ms;
    int next_execute_time;
    bool execute_flag;
    void threadUpdate();
    
    
    void jumpPopcones(int d);
    
    int 				threshold;
    bool				bLearnBakground;
    float               width;
    float               height;
    double              motionCount = 0;
    int                 lastJumpTime = 0;
    int                 drawCount = 0;
    int                 loopCnt = 1;
    const int           judgePoint = 50;
    const int           resultBeginTime = 53670;
    const int           feverBeginTime = 20780;
    bool                feverTimeFlag = false;
    bool                resultTimeFlag = false;
    double tracking_interval;
    double diff_param;
    double number_of_object;
    
    const double INF = (1 << 27);
    
    // for result ranking view
    float area_a, area_b, area_c, pop_a, pop_b, pop_c;
    string current_area_name;
    double area_img_expanded;
    
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
