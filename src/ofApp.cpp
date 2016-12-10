#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    // load images
    ofDirectory dir;
    ofDisableArbTex();
    int n = dir.listDir("popcones");
    for (int i=0; i<n; i++) {
        images.push_back(ofImage(dir.getPath(i)));
	}
    
    dencity     = 0.1;
    bounce      = 0.3;
    friction    = 0.3;
    gravity     = 60;
    pop_power   = 200;
    
    ofSetVerticalSync(true);
    ofBackgroundHex(0xfdefc2);
    ofSetLogLevel(OF_LOG_NOTICE);
    
    bMouseForce = false;
    
    box2d.init();
    box2d.setGravity(0, gravity);
    box2d.createGround();
    box2d.setFPS(60.0);
    box2d.registerGrabbing();
    box2d.createBounds(0, 0, ofGetWidth(), ofGetHeight());
    
    #ifdef _USE_LIVE_VIDEO
        vidGrabber.setVerbose(true);
        vidGrabber.setup(320,240);
    #else
        vidPlayer.load("/path/to/veideo.mov");
        vidPlayer.play();
        vidPlayer.setLoopState(OF_LOOP_NORMAL);
    #endif
    
    width = 320;
    height = 240;
    
    bLearnBakground = true;
    threshold = 80;
    
}
//--------------------------------------------------------------
void ofApp::update() {
    box2d.update();
    
    if(bMouseForce) {
        float strength = 1.0f;
        float damping  = 1.0f;
        for(int i=0; i<customParticles.size(); i++) {
            customParticles[i].get()->addAttractionPoint(mouseX, mouseY, strength);
            customParticles[i].get()->setDamping(damping, damping);
        }
    }
    
    bool bNewFrame = false;
    
    #ifdef _USE_LIVE_VIDEO
        vidGrabber.update();
        bNewFrame = vidGrabber.isFrameNew();
    #else
        vidPlayer.update();
        bNewFrame = vidPlayer.isFrameNew();
    #endif
    
    if (bNewFrame){
        
    #ifdef _USE_LIVE_VIDEO
        colorImg.setFromPixels(vidGrabber.getPixels());
    #else
        colorImg.setFromPixels(vidPlayer.getPixels());
    #endif
        
        grayImage = colorImg;
        
        if (bLearnBakground == true){
            grayBg = grayImage;		// the = sign copys the pixels from grayImage into grayBg (operator overloading)
            bLearnBakground = false;
        }
        
        // take the abs value of the difference between background and incoming and then threshold:
        grayDiff.absDiff(grayBg, grayImage);
        grayDiff.threshold(threshold);
        
        // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
        // also, find holes is set to true so we will get interior contours as well....
        contourFinder.findContours(grayDiff, 20, (width*height)/3, 10, true);	// find holes
    }
    
}


//--------------------------------------------------------------
void ofApp::draw() {
    for(int i=0; i<customParticles.size(); i++) {
        customParticles[i].get()->draw();
    }
    
    for (int i = 0; i < contourFinder.nBlobs; i++){
        if(contourFinder.blobs[i].hole){
            bLearnBakground = true;
        }
    }
    
    motionCount = contourFinder.nBlobs;
    
    drawCount++;
    // judge jump motion
    // 定数にチューニング必要
    if(contourFinder.nBlobs > 3 && abs(contourFinder.nBlobs-lastContourFinder.nBlobs) > 1 /*&& (time(NULL) - lastJumpTime) > 0.01*/) {
        lastJumpTime = time(NULL);
        int d = motionVector(contourFinder,lastContourFinder);
        jumpPopcones(d);

    }
    
    if(contourFinder.nBlobs > 0) lastContourFinder = contourFinder;
    
    ofFill();
}

vector<pair<double,double> > ofApp::getHolePoints(ofxCvContourFinder const& contourFinder) {
    
    vector<pair<double,double> > holePoints;
    
    for (int i = 0; i < contourFinder.nBlobs; i++){
        //if(contourFinder.blobs[i].hole){
            holePoints.push_back(make_pair(contourFinder.blobs[i].centroid.x,contourFinder.blobs[i].centroid.y));
        //}
    }
    
    return holePoints;
}

bool ofApp::isSameMotion(pair<double,double> point,pair<double,double> lastPoint) {
    const double thresh_hold = 50;
    double dist = (point.first-lastPoint.first)*(point.first-lastPoint.first) + (point.second-lastPoint.second)*(point.second-lastPoint.second);
    return dist <= thresh_hold;
}

double ofApp::getDistance(pair<double,double> point,pair<double,double> lastPoint) {
    double dist = (point.first-lastPoint.first)*(point.first-lastPoint.first) + (point.second-lastPoint.second)*(point.second-lastPoint.second);
    return dist;
}

int ofApp::motionIndex(pair<double,double> point,pair<double,double> lastPoint) {
    

    //右
    if(point.first-lastPoint.first > 0) return 3;
    //左
    return 4;
    
}

int ofApp::motionVector(ofxCvContourFinder const& contourFinder,ofxCvContourFinder const& lastContourFinder) {

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
            //if(!isSameMotion(holePoints[i], lastHolePoints[j])) continue;
            double dist = getDistance(holePoints[i],lastHolePoints[j]);
            if(mxDist > dist && dist > 0) {
                mxDist = dist;
                pairIdx = j;
            }
        }
        // 定数にチューニング必要
        if(mxDist < 500) {
            int vIdx = motionIndex(holePoints[i],lastHolePoints[pairIdx]);
            motionVectorCnt[vIdx]++;
        }
    }
    vector<int>::iterator it = max_element(motionVectorCnt.begin(),motionVectorCnt.end());
    int mxVectorIdx = std::distance(motionVectorCnt.begin(), it);
    return mxVectorIdx;
    
}

void ofApp::jumpPopcones(int d) {
    
    double dx = 0,dy = 0;
    switch (d) {
        case 1:
            // nothing move
            cout << "down" << endl;
            //dy = 50;
            break;
        case 2:
            cout << "up" << endl;
            dy = -50;
            break;
        case 3:
            cout << "left" << endl;
            dx = 50;
            break;
        case 4:
            cout << "right" << endl;
            dx = -50;
            break;
        default:
            break;
    }
    
    for(int i = 0; i < customParticles.size(); i++){
        float vec_x = customParticles[i].get()->getPosition().x;
        float vec_y = customParticles[i].get()->getPosition().y;
        customParticles[i].get()->addRepulsionForce(vec_x+dx, vec_y+dy, pop_power);
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if(key == 'c') {
        float r = ofRandom(min_popcone_size, max_popcone_size);
        customParticles.push_back(shared_ptr<CustomParticle>(new CustomParticle(images, "あ")));
        CustomParticle * p = customParticles.back().get();
        
        p->setPhysics(dencity, bounce, friction);
        p->setup(box2d.getWorld(), mouseX, mouseY, r);
    }
    if (key == 'a') {
        jumpPopcones(3);
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
    
}

//--------------------------------------------------------------
void ofApp::resized(int w, int h){
    
}
