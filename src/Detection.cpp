#include "Detection.h"

vector<pair<double,double> > getHolePoints(ofxCvContourFinder const& contourFinder) {
    
    vector<pair<double,double> > holePoints;
    
    for (int i = 0; i < contourFinder.nBlobs; i++){
        holePoints.push_back(make_pair(contourFinder.blobs[i].centroid.x,contourFinder.blobs[i].centroid.y));
    }
    
    return holePoints;
}

double getDistance(pair<double,double> point,pair<double,double> lastPoint) {
    double dist = (point.first-lastPoint.first)*(point.first-lastPoint.first) + (point.second-lastPoint.second)*(point.second-lastPoint.second);
    return dist;
}

int motionIndex(pair<double,double> point,pair<double,double> lastPoint) {
    
    // left
    if(point.first-lastPoint.first > 0) return 3;
    // right
    return 4;
    
}

int motionVector(ofxCvContourFinder const& contourFinder,ofxCvContourFinder const& lastContourFinder) {
    
    vector<pair<double,double> > holePoints = getHolePoints(contourFinder);
    vector<pair<double,double> > lastHolePoints = getHolePoints(lastContourFinder);
    
    for(int i = 0 ; i < holePoints.size() ; i++) {
        cout << "tmp:" << holePoints[i].first << " " << holePoints[i].second << endl;
    }
    
    for(int i = 0 ; i < lastHolePoints.size() ; i++) {
        cout << "last:" << lastHolePoints[i].first << " " << lastHolePoints[i].second << endl;
    }
    
    vector<int> motionVectorCnt(5,0);
    
    for(int i = 0 ; i < holePoints.size() ; i++) {
        double mxDist = INF;
        int pairIdx = -1;
        for(int j = 0 ; j < lastHolePoints.size() ; j++) {
            double dist = getDistance(holePoints[i],lastHolePoints[j]);
            if(mxDist > dist && dist > 0) {
                mxDist = dist;
                pairIdx = j;
            }
        }
        // same motion threshold
        if(mxDist < 500) {
            int vIdx = motionIndex(holePoints[i],lastHolePoints[pairIdx]);
            motionVectorCnt[vIdx]++;
        }
    }
    vector<int>::iterator it = max_element(motionVectorCnt.begin(),motionVectorCnt.end());
    int mxVectorIdx = std::distance(motionVectorCnt.begin(), it);
    return mxVectorIdx;
    
}
 
