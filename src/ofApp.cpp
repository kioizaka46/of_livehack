#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    // file setting
    json_file_name      = "sync_koi_hoshinogen.json";
    music_file_name     = "koi_hoshinogen.mp3";
    
    // draw setting
    font_size           = 40;
    word_margin         = 20;
    radius_fix_pram     = 0.6;
    margin_time         = 300;
    drop_point_x        = 100.0;
    drop_point_y        = 350.0;
    start_point_x       = 200.0;
    start_point_y       = 250.0;
    preload_number      = 3;
    loaded_line_head    = 0;
    now_lyric_line      = 0;
    camera_draw_opacity = 0.7;
    
    // physic setting
    density             = 0.5;
    bounce              = 0.4;
    friction            = 1.0;
    gravity             = 25;
    pop_power           = 600;
    
    // motion tracking setting
    width               = 320;
    height              = 240;
    bLearnBakground     = true;
    threshold           = 80;
    number_of_object    = 2;
    diff_param          = 1.5;
    tracking_interval   = 1.5;
    
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
    window_width = ofGetWidth();
    window_height = ofGetHeight();

    // box2d setting
    box2d.init();
    box2d.setGravity(0, gravity);
    box2d.createGround();
    box2d.setFPS(45.0);
    box2d.registerGrabbing();
    box2d.createBounds(0, 0, window_width, window_height);
    
    ofSetWindowShape(window_width, window_height);

    
    // setup camera
    vidGrabber.setVerbose(true);
    vidGrabber.setup(window_width ,window_height);
    vector<ofVideoDevice> devices = vidGrabber.listDevices();
    for(int i = 0; i < devices.size(); i++){
        if(devices[i].bAvailable){
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
        }else{
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
        }
    }
    vidGrabber.setDeviceID(0);
    vidGrabber.setDesiredFrameRate(40);
    vidGrabber.initGrabber(window_width, window_height);
    
    
    // load font
    font.loadFont(font_file_name, font_size, true, true);

    // parse json and create obj
    std::string file = json_file_name;
    bool parsingSuccessful = sync_lyric_json.open(file);

    if (parsingSuccessful){
        loaded_line_head = 0;
    } else {
        ofLogError("ofApp::setup")  << "Failed to parse JSON" << endl;
    }

    // music load and play
    music.load(music_file_name);
    music.setMultiPlay(true);
    music.play();
    music.setPositionMS(10000);
    
    finder.setup("haarcascade_frontalface_default.xml");
    img.load("asyu.png");
}
vector<shared_ptr<CustomParticle>> ofApp::getLineObj(int line_index){
    // create line obj
    vector<shared_ptr<CustomParticle>> tmp_obj;
    if(0 <= line_index && line_index < sync_lyric_json["lines"].size()){
        double start_time = sync_lyric_json["lines"][line_index]["time"].asDouble();
        for(int i=0; i < sync_lyric_json["lines"][line_index]["words"].size(); i++){
            string word_str = sync_lyric_json["lines"][line_index]["words"][i]["string"].asString();
            if(word_str != " "){
                tmp_obj.push_back(shared_ptr<CustomParticle>(new CustomParticle(images, word_str, start_time, font_size)));
            }
        }
    } else {
        ofLogError()  << "Not found Line " << line_index << endl;
    }
    // set physics
    for(int i = 0; i < tmp_obj.size(); i++) {
        tmp_obj[i].get()->setPhysics(density, bounce, friction);
        tmp_obj[i].get()->setup(box2d.getWorld(),
                                              (ofGetWidth() - tmp_obj.size() * (font_size + word_margin))/2 + (i * (font_size + word_margin)),
                                              start_point_y,
                                              font_size * radius_fix_pram);
    }
    return tmp_obj;
}
//--------------------------------------------------------------
void ofApp::update() {
    // move depend on phisic setting
    box2d.update();
    
    // camera captured
    bool bNewFrame = false;
    vidGrabber.update();
    bNewFrame = vidGrabber.isFrameNew();
    
    if (bNewFrame){
        colorImg.setFromPixels(vidGrabber.getPixels());
        
        grayImage = colorImg;
        if (bLearnBakground == true){
            grayBg = grayImage;                // the = sign copys the pixels from grayImage into grayBg (operator overloading)
            bLearnBakground = false;
        }

        // take the abs value of the difference between background and incoming and then threshold:
        grayDiff.absDiff(grayBg, grayImage);
        grayDiff.threshold(threshold);
        
        // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
        // also, find holes is set to true so we will get interior contours as well....
        contourFinder.findContours(grayDiff, 20, (width*height)/3, 10, true);  // find holes
    }
    
    // judge next lyric line started
    int tail_index = viewable_particles.size() - 1;
    
    float music_pos = music.getPositionMS();
    next_lyric_ms = sync_lyric_json["lines"][loaded_line_head]["time"].asDouble();
    if (music_pos + margin_time > next_lyric_ms && loaded_line_head < sync_lyric_json["lines"].size()) {
        // next lyric line add viewable obj
        viewable_particles.push_back(getLineObj(loaded_line_head));
        
        // set position of now lyric under next lyric
        tail_index = viewable_particles.size() - 1;
        for(int i = 0; i < viewable_particles[tail_index].size(); i++){
            viewable_particles[tail_index][i].get()->setPosition(
                                                                 (ofGetWidth() - viewable_particles[tail_index].size() * (font_size + word_margin))/2 + (i * (font_size + word_margin)),
                                                                 start_point_y + font_size);
        }
        
        // line indenting
        loaded_line_head++;
    }
    
    // fix now lyric position
    if(tail_index >= 0){
        for(int i = 0; i < viewable_particles[tail_index].size(); i++){
            viewable_particles[tail_index][i].get()->setPosition(
                                                                 (ofGetWidth() - viewable_particles[tail_index].size() * (font_size + word_margin))/2 + (i * (font_size + word_margin)),
                                                                 start_point_y);
        }
    }
    
    // change box2d bound size if change window size
    if (window_width != ofGetWidth() || window_height != ofGetHeight()) {
        // update viewable particles position
        for(int i = 0; i < viewable_particles.size(); i++) {
            for (int j = 0; j < viewable_particles[i].size(); j++) {
                viewable_particles[i][j].get()->setPosition(
                                                             (ofGetWidth() - viewable_particles[i].size() * (font_size + word_margin))/2 + (j * (font_size + word_margin)),
                                                            start_point_y);
            }
        }
        
        window_width = ofGetWidth();
        window_height = ofGetHeight();
        box2d.createBounds(0, 0, window_width, window_height);
    }
    
    // capture camera view
    vidGrabber.update();
    
    
    // check collision
    float wall_right, wall_left, wall_celling, setp;
    setp = 100;
    for(int i = 0; i < viewable_particles.size(); i++){
        for(int j = 0; j < viewable_particles[i].size(); j++){
            double radius = viewable_particles[i][j].get()->getRadius();
            wall_right = ofGetWidth() - radius;
            wall_left = radius;
            wall_celling = radius;
            double x = viewable_particles[i][j].get()->getPosition().x;
            double y = viewable_particles[i][j].get()->getPosition().y;
            
            if (x <= wall_right && y <= setp) {
                viewable_particles[i][j].get()->collisioned_count++;
            } else if (x >= wall_left && y <= setp){
                viewable_particles[i][j].get()->collisioned_count++;
            } else if (y >= wall_celling && y <= setp){
                viewable_particles[i][j].get()->collisioned_count++;
            }
            
            if (viewable_particles[i][j].get()->collisioned_count == 50) {
                viewable_particles[i][j].get()->opacity = 0.7;
            } else if (viewable_particles[i][j].get()->collisioned_count == 100) {
                viewable_particles[i][j].get()->opacity = 0.3;
            } else if (viewable_particles[i][j].get()->collisioned_count == 150){;
                viewable_particles[i][j].get()->opacity = 1.0;
                viewable_particles[i][j].get()->bake_level = 0.5;
            } else if (viewable_particles[i][j].get()->collisioned_count == 170) {
                viewable_particles[i][j].get()->opacity = 1.0;
                viewable_particles[i][j].get()->bake_level = 0.7;
            } else if (viewable_particles[i][j].get()->collisioned_count > 200) {
                viewable_particles[i][j].get()->opacity = 0;
                viewable_particles[i][j].get()->destroy();
                viewable_particles[i].erase(viewable_particles[i].begin() + j );
            }
        }
    }
    
    
    //image.setFromPixels(vidGrabber.getPixels().getData(), window_width, window_height, OF_IMAGE_COLOR);
    //  face detection
    
    if(loopCnt % judgePoint == 0) finder.findHaarObjects(grayImage, 10, 10);
    loopCnt++;
    // sound update
    ofSoundUpdate();
}

//--------------------------------------------------------------
void ofApp::draw() {
    // draw camera caputured
    // TODO reverse capture image
    ofSetColor(255, 255, 255, 255 * camera_draw_opacity);
    vidGrabber.draw(0,0);
    
    // draw viewable lyrics
    for(int i = 0; i < viewable_particles.size(); i++){
        for(int j = 0; j < viewable_particles[i].size(); j++){
            viewable_particles[i][j]->draw();
        }
    }
    
    // motion section
    for (int i = 0; i < contourFinder.nBlobs; i++){
        if(contourFinder.blobs[i].hole){
            bLearnBakground = true;
        }
    }
    
    motionCount = contourFinder.nBlobs;
    
    drawCount++;
    // judge jump motion
    if(contourFinder.nBlobs > number_of_object && abs(contourFinder.nBlobs - lastContourFinder.nBlobs) > diff_param && (time(NULL) - lastJumpTime) > tracking_interval) {
        lastJumpTime = time(NULL);
        int d = motionVector(contourFinder, lastContourFinder);
        jumpPopcones(d);
    }
    
    if(contourFinder.nBlobs > 0) lastContourFinder = contourFinder;

    //image.draw(0, 0);
    ofSetLineWidth(3);
    ofNoFill();
    
    cout << finder.blobs.size() << endl;
        
    for(int i = 0; i < finder.blobs.size(); i++) {
        ofRectangle cur = finder.blobs[i].boundingRect;
        ofDrawRectangle(cur.x, cur.y, cur.width, cur.height);
        img.draw(cur.x,cur.y,cur.width,cur.height);
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if (key == 'a') {
        for(int i = 0; i < viewable_particles.size() - 1; i++){
            for(int j = 0; j < viewable_particles[i].size(); j++){
                float vec_x = viewable_particles[i][j].get()->getPosition().x;
                float vec_y = viewable_particles[i][j].get()->getPosition().y;
                viewable_particles[i][j].get()->addRepulsionForce(vec_x, vec_y + 50, pop_power);
            }
        }
    }
    if (key == 'p') {
        for(int try_count = 0; try_count < 30; try_count++){
            if(viewable_particles.size()==1)break;
            int i = (int)ofRandom(0,viewable_particles.size());
            if(i == viewable_particles.size()-1)continue;
            int j = (int)ofRandom(0,viewable_particles[i].size());
            if(viewable_particles[i][j].get()->bake_level >= 0.5) continue;
            float vec_x = viewable_particles[i][j].get()->getPosition().x;
            float vec_y = viewable_particles[i][j].get()->getPosition().y;
            viewable_particles[i][j].get()->addRepulsionForce(vec_x, vec_y + 50, pop_power);
            viewable_particles[i][j].get()->bake_level = 0.5;
            break;
        }
    }
    if (key == 'r'){
        box2d.createBounds(0, 0, 0, 0);
    }
    if (key == 'g'){
        box2d.createBounds(0, 0, window_width, window_height);
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
        case 2:
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
