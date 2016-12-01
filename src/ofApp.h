#pragma once

#include "ofMain.h"
#include "ofxJSON.h"
#include "ofxTrueTypeFontUC.h"

struct TextSymbol {
    std::string text;
    double start_time;
    double end_time;
    int size;
    double pos_x;
    double pos_y;
    bool is_draw;
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
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
};
