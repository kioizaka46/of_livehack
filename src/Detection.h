#include "ofxBox2d.h"
#include "ofxOpenCv.h"
#define _USE_LIVE_VIDEO

const double INF = (1 << 27);

int motionVector(ofxCvContourFinder const& contourFinder,ofxCvContourFinder const& lastContourFinder);
vector<pair<double,double> > getHolePoints(ofxCvContourFinder const& contourFinder);
bool isSameMotion(pair<double,double> point,pair<double,double> lastPoint);
double getDistance(pair<double,double> point,pair<double,double> lastPoint);
int motionIndex(pair<double,double> point,pair<double,double> lastPoint);
void jumpPopcones(int d);
