#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    // file setting
    json_file_name      = "sync_koi_hoshinogen.json";
    music_file_name     = "koi_hoshinogen.mp3";
    
    // draw setting
    font_size           = 35;
    word_margin         = 10;
    radius_fix_pram     = 0.6;
    margin_time         = 300;
    drop_point_x        = 100.0;
    drop_point_y        = 350.0;
    start_point_x       = 200.0;
    start_point_y       = 250.0;
    preload_number      = 3;
    loaded_line_head    = 0;
    now_lyric_line      = 0;
    
    // physic setting
    dencity             = 0.1;
    bounce              = 0.3;
    friction            = 0.3;
    gravity             = 50;
    pop_power           = 200;
    
    // motion tracking setting
    width               = 320;
    height              = 240;
    bLearnBakground     = true;
    threshold           = 80;
    number_of_object    = 3;
    diff_param          = 1;
    tracking_interval   = 3;
    
    // load images
    ofDirectory dir;
    ofDisableArbTex();
    int n = dir.listDir("popcones");
    for (int i=0; i<n; i++) {
        images.push_back(ofImage(dir.getPath(i)));
    }
    
    // draw setting
    ofSetVerticalSync(true);
    ofBackgroundHex(0xE7C1DA);
    ofSetLogLevel(OF_LOG_NOTICE);
    ofEnableAlphaBlending();

    // box2d setting
    box2d.init();
    box2d.setGravity(0, gravity);
    box2d.createGround();
    box2d.setFPS(30.0);
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
    
    // load font
    font.loadFont(font_file_name, 25, true, true);

    // parse json and create obj
    std::string file = json_file_name;
    bool parsingSuccessful = sync_lyric_json.open(file);

    if (parsingSuccessful){
        // viewable particle initialize
        vector<shared_ptr<CustomParticle>> dummy_obj;
        dummy_obj.push_back(shared_ptr<CustomParticle>(new CustomParticle(images, "", 0, font_size)));
        for(int i = 0; i < dummy_obj.size(); i++){
            dummy_obj[i].get()->setup(box2d.getWorld(),
                                    ofGetWidth(),
                                    start_point_y,
                                    font_size * radius_fix_pram);
        }
        viewable_particles.push_back(dummy_obj);

        // buffering
        for(int i = 0; i < preload_number; i++) {
            buffering_particles.push_back(getLineObj(i));
            loaded_line_head = preload_number;
        }

        // setup position of box2dworld
        for(int i = 0; i< buffering_particles.size(); i++){
            for(int j = 0; j < buffering_particles[i].size(); j++){
                buffering_particles[i][j].get()->setup(box2d.getWorld(),
                                                       (ofGetWidth() - buffering_particles[i].size() * (font_size + word_margin))/2 + (j * (font_size + word_margin)),
                                                       start_point_y,
                                                       font_size * radius_fix_pram);
            }
        }
    } else {
        ofLogError("ofApp::setup")  << "Failed to parse JSON" << endl;
    }

    // music load and play
    music.load(music_file_name);
    music.setMultiPlay(true);
    music.play();
    music.setPositionMS(10000);
}
vector<shared_ptr<CustomParticle>> ofApp::getLineObj(int line_index){
    vector<shared_ptr<CustomParticle>> tmp_obj;
    if(0 <= line_index && line_index < sync_lyric_json["lines"].size()){
        double start_time = sync_lyric_json["lines"][line_index]["time"].asDouble();
        for(int i=0; i < sync_lyric_json["lines"][line_index]["words"].size(); i++){
            string word_str = sync_lyric_json["lines"][line_index]["words"][i]["string"].asString();
            tmp_obj.push_back(shared_ptr<CustomParticle>(new CustomParticle(images, word_str, start_time, font_size)));
        }
    } else {
        ofLogError()  << "Not found Line " << line_index << endl;
    }
    return tmp_obj;
}
//--------------------------------------------------------------
void ofApp::update() {
    float music_pos = music.getPositionMS();

    // judge next lyric line started
    if (music_pos + margin_time > buffering_particles[0][0]->start_time && loaded_line_head < sync_lyric_json["lines"].size()) {
        // set physics for now lyric line on viewable obj tail
        int tail_index_vp = viewable_particles.size() - 1;
        for(int i = 0; i < viewable_particles[tail_index_vp].size(); i++){
            viewable_particles[tail_index_vp][i].get()->setPhysics(dencity, bounce, friction);
            viewable_particles[tail_index_vp][i].get()->setup(box2d.getWorld(),
                                                              viewable_particles[tail_index_vp][i].get()->getPosition().x ,
                                                              viewable_particles[tail_index_vp][i].get()->getPosition().y + font_size,
                                                              font_size * radius_fix_pram);
        }

        // next lyric line add viewable obj
        viewable_particles.push_back(buffering_particles[0]);

        // remove buffering head
        if(buffering_particles.size() > 0){
            buffering_particles.erase(buffering_particles.begin() + 0);
        }

        // add(load) next lyric on buffering tail
        buffering_particles.push_back(getLineObj(loaded_line_head));

        // setup position next lyric on buffering tail
        int tail_index_bp = buffering_particles.size() - 1;
        for(int i = 0; i < buffering_particles[tail_index_bp].size(); i++) {
            buffering_particles[tail_index_bp][i].get()->setup(box2d.getWorld(),
                                                               (ofGetWidth() - buffering_particles[tail_index_bp].size() * (font_size + word_margin))/2 + (i * (font_size + word_margin)),
                                                               start_point_y,
                                                               font_size * radius_fix_pram);
        }
        loaded_line_head++;
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
    
    box2d.update();
    ofSoundUpdate();
}

//--------------------------------------------------------------
void ofApp::draw() {
    // draw viewable lyrics
    for(int i = 0; i < viewable_particles.size(); i++){
        for(int j = 0; j < viewable_particles[i].size(); j++){
            viewable_particles[i][j]->draw();
        }
    }    
    for (int i = 0; i < contourFinder.nBlobs; i++){
        if(contourFinder.blobs[i].hole){
            bLearnBakground = true;
        }
    }
    
    motionCount = contourFinder.nBlobs;
    
    drawCount++;
    // judge jump motion
    if(contourFinder.nBlobs > number_of_object && abs(contourFinder.nBlobs-lastContourFinder.nBlobs) > diff_param && (time(NULL) - lastJumpTime) > tracking_interval) {
        lastJumpTime = time(NULL);
        int d = motionVector(contourFinder,lastContourFinder);
        jumpPopcones(d);

    }
    
    if(contourFinder.nBlobs > 0) lastContourFinder = contourFinder;
    
    ofFill();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if (key == 'a') {
        for(int i = 1; i < viewable_particles.size(); i++){
            for(int j = 0; j < viewable_particles[i].size(); j++){
                float vec_x = viewable_particles[i][j].get()->getPosition().x;
                float vec_y = viewable_particles[i][j].get()->getPosition().y;
                viewable_particles[i][j].get()->addRepulsionForce(vec_x, vec_y + 50, pop_power);
            }
        }
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

//--------------------------------------------------------------
void ofApp::jumpPopcones(int d) {
    
    double dx = 0,dy = 0;
    // temporary left and right are valid.
    switch (d) {
        case 1:
            // nothing move
//            cout << "down" << endl;
            dy = 50;
            break;
        case 2
//            cout << "up" << endl;
            dy = -50;
            break;
        case 3:
//            cout << "left" << endl;
            dx = 50;
            break;
        case 4:
//            cout << "right" << endl;
            dx = -50;
            break;
        default:
            break;
    }
    
    // Pop lyrics and popcones
    for(int i = 0; i < viewable_particles.size(); i++){
        for(int j = 0; j < viewable_particles[i].size(); j++){
            float vec_x = viewable_particles[i][j].get()->getPosition().x;
            float vec_y = viewable_particles[i][j].get()->getPosition().y;
            viewable_particles[i][j].get()->addRepulsionForce(vec_x + dx, vec_y + dy, pop_power);
        }
    }
}