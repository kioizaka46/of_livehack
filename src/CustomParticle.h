#include "ofMain.h"
#include "ofxTrueTypeFontUC.h"
#include "ofxBox2d.h"

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
    int collisioned_count;
    
    CustomParticle(vector<ofImage> images, string txt, float st_time, int f_size);
    void draw();
};
