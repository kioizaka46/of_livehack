#include "ofApp.h"
#include "map"
#include "algorithm"
#include "cmath"
using namespace std;

//--------------------------------------------------------------
void ofApp::setup() {
    // file setting
    json_file_name      = "sync_koi_hoshinogen.json";
    music_file_name     = "koi_hoshinogen.mp3";
    
    // time setting
    next_execute_time   = ofGetElapsedTimeMillis();
    servo_thread.sending(rotate_degree);
    
    // load images
    ofDirectory dir;
    ofDisableArbTex();
    int n = dir.listDir("popcornes");
    for (int i=0; i<n; i++) {
        images.push_back(ofImage(dir.getPath(i)));
    }
    
    // load fevertime popcorn images
    ofDirectory dir_f;
    int nf = dir_f.listDir("popcornes_mini");
    for (int i=0; i<nf; i++) {
        images_fevertime.push_back(ofImage(dir_f.getPath(i)));
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
    box2d.setFPS(30.0);
    box2d.registerGrabbing();
    box2d.createBounds(0, 0, window_width, window_height);
    
    ofSetWindowShape(window_width, window_height);
    
    // area circle
    int area_num = dir.listDir("areas");
    for (int i=0; i<area_num; i++) {
        area_images.push_back(ofImage(dir.getPath(i)));
    }
    area_circle_obj = new ofxBox2dCircle();
    area_circle_obj->setup(box2d.getWorld(), ofGetWidth()/2, ofGetHeight(), area_images[0].getWidth() * (area_img_expanded * 0.6));
    
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
    vidGrabber.setDesiredFrameRate(25);
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
    
    // snow
    finder.setup("haarcascade_frontalface_default.xml");
    snow_img.load("popcorn_snow.png");
    fevertime_img.load("fevertime.png");
    
    // image images
    yaneA.load("images/yane_A.png");
    yaneB.load("images/yane_B.png");
    yaneC.load("images/yane_C.png");
    textA.load("areas/text_A.png");
    textB.load("areas/text_B.png");
    textC.load("areas/text_C.png");
    
    // load rank images
    ofDirectory ranks_dir;
    ofDisableArbTex();
    int ranks_n = ranks_dir.listDir("ranks");
    for (int i = 0; i < ranks_n; i++) {
        rank_images.push_back(ofImage(ranks_dir.getPath(i)));
    }
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

shared_ptr<CustomParticle> ofApp::getCustomObj(vector<ofImage> popcorn_images, int line_index, int x, int y){
    // create line obj
    shared_ptr<CustomParticle> tmp_obj;
    string dummy_str = "*";
    tmp_obj = shared_ptr<CustomParticle>(new CustomParticle(popcorn_images, dummy_str, 0, font_size));
    
    // set physics
    tmp_obj->setPhysics(density, bounce, friction);
    tmp_obj->setup(box2d.getWorld(), x, y, 20);

    return tmp_obj;
}
//--------------------------------------------------------------
void ofApp::update() {
    // move depend on phisic setting
    box2d.update();
    
    // thread update
    threadUpdate();

    // camera captured
    bool bNewFrame = false;
    vidGrabber.update();
    bNewFrame = vidGrabber.isFrameNew();
    
    // set flag
    resultTimeFlag      = music.getPositionMS() > resultBeginTime;
    resultTimeFlagment  = music.getPositionMS() < resultBeginTime;
    feverTimeFlag       = music.getPositionMS() > feverBeginTime && music.getPositionMS() < feverEndTime;
   
    if (bNewFrame){
        colorImg.setFromPixels(vidGrabber.getPixels());
        
        grayImage = colorImg;
        if (bLearnBakground == true){
            grayBg = grayImage;// the = sign copys the pixels from grayImage into grayBg (operator overloading)
            bLearnBakground = false;
        }
        
        // take the abs value of the difference between background and incoming and then threshold:
        grayDiff.absDiff(grayBg, grayImage);
        grayDiff.threshold(threshold);
        
        // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
        // also, find holes is set to true so we will get interior contours as well....
        contourFinder.findContours(grayDiff, 20, (width*height)/3, 100, true);  // find holes
    }
    
    // judge next lyric line started
    int tail_index = viewable_particles.size() - 1;
    
    // syncronized lyric generate
    float music_pos = music.getPositionMS();
    next_lyric_ms = sync_lyric_json["lines"][loaded_line_head]["time"].asDouble();
    if (music_pos + margin_time > next_lyric_ms && loaded_line_head < sync_lyric_json["lines"].size() && !feverTimeFlag) {
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
    if(tail_index >= 0 && !feverTimeFlag){
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
        
        // capture camera view
        vidGrabber.update();
    }
    
    // check collision
    checkCollision();
    
    //  face detection
    if(feverTimeFlag) {
        if(loopCnt % judgePoint == 0) finder.findHaarObjects(grayImage, 10, 10);
    }
    
    // result view
    if (resultBeginTime + lyricClearMarginTime < music.getPositionMS()) {
        if (!isCalcurated) {
            setupResult();
            isCalcurated = true;
        }
        if (!resultGenerated_A && loopCnt % drop_interval == 0) {
            result_viewable_particles.push_back(getCustomObj(images, 0, ofGetWidth()/6+ofRandom(20), 0));
            drop_count_a ++;
            if(drop_end_time_ms_A < music.getPositionMS()) {
                resultGenerated_A = true;
                checkEnd++;
            }
        }
        if (!resultGenerated_B && loopCnt % drop_interval == 0) {
            result_viewable_particles.push_back(getCustomObj(images,loaded_line_head, ofGetWidth()/2+ofRandom(20), 0));
            drop_count_b ++;
            if (drop_end_time_ms_B < music.getPositionMS()) {
                resultGenerated_B = true;
                checkEnd++;
            }
        }
        if (!resultGenerated_C && loopCnt % drop_interval == 0) {
            result_viewable_particles.push_back(getCustomObj(images, loaded_line_head, ofGetWidth()*5/6+ofRandom(20), 0));
            drop_count_c ++;
            if (drop_end_time_ms_C < music.getPositionMS()) {
                resultGenerated_C = true;
                checkEnd++;
            }
        }
    }
    
    // loop count inclement for frame controll
    loopCnt++;
    
    // sound update
    ofSoundUpdate();
}

//--------------------------------------------------------------
void ofApp::draw() {
    // draw camera caputured
    // TODO reverse capture image
    ofSetColor(255, 255, 255, 255 * camera_draw_opacity);
    
    int camera_height_fixed = (double)ofGetWidth() * (double)(9.0/16.0);
    double camera_aspect_ratio = (double)ofGetHeight()/(double)camera_height_fixed;
    vidGrabber.draw(ofGetWidth()*camera_aspect_ratio,0, -ofGetWidth()*camera_aspect_ratio, ofGetHeight()*camera_aspect_ratio);
    
    // draw snow
    ofSetLineWidth(3);
    ofNoFill();
    
    // fever time face detection
    int control_size_x = 80;
    int control_size_y = 200;
    if(feverTimeFlag) {
        for(int i = 0; i < finder.blobs.size(); i++) {
            // draw image on face
            ofRectangle cur = finder.blobs[i].boundingRect;
            ofSetColor(255, 255, 255);
            snow_img.draw(
                          cur.x - control_size_x/2,
                          cur.y - control_size_y/2 - 50,
                          cur.width + control_size_x,
                          cur.height + control_size_y);
            
            // draw fever time popcorn
            if(loopCnt % 15 == 0){
                int tmp_last_index = viewable_particles.size() - 1;
                viewable_particles[tmp_last_index].push_back(getCustomObj(
                                                               images_fevertime,
                                                               loaded_line_head,
                                                               cur.x + cur.width/2,
                                                               cur.y + cur.height/2 - 150));
                int idx = viewable_particles[tmp_last_index].size() - 1;
                viewable_particles[tmp_last_index][idx].get()->addRepulsionForce(cur.x + (cur.width/2 + control_size_x) + (rand()%100 - rand()%100), cur.y + (cur.height/2 + control_size_y) + (rand()%50 - rand()%50), 20);
                viewable_particles[tmp_last_index][idx].get()->bake_level = 0.8;
                viewable_particles[tmp_last_index][idx]->draw();
            }
        }
    }
    
    // draw viewable lyrics
    for(int i = 0; i < viewable_particles.size(); i++){
        for(int j = 0; j < viewable_particles[i].size(); j++){
            viewable_particles[i][j]->draw();
        }
    }
    
    // draw and animate fevertime_text if fevertime
    ofSetColor(255, 255, 255);
    drawFeverText();
    
    // motion section
    for (int i = 0; i < contourFinder.nBlobs; i++){
        if(contourFinder.blobs[i].hole){
            bLearnBakground = true;
        }
    }
    motionCount = contourFinder.nBlobs;

    // judge jump motion
    if(contourFinder.nBlobs > number_of_object && abs(contourFinder.nBlobs - lastContourFinder.nBlobs) > diff_param && (time(NULL) - lastJumpTime) > tracking_interval && !resultTimeFlag) {
        lastJumpTime = time(NULL);
        int d = motionVector(contourFinder, lastContourFinder);
        jumpPopcones(d);
    }
    
    // TODO Why is following if section deleted on feature-result branch?
    if (contourFinder.nBlobs > 0) lastContourFinder = contourFinder;
    
    // draw current area image
    if (resultTimeFlagment) {
        int img_index;
        if(current_area_name == "A"){
            img_index = 0;
        }else if(current_area_name == "B"){
            img_index = 1;
        }else{
            img_index = 2;
        }
        ofSetColor(255,255,255);
        area_circle_obj->setPosition(ofGetWidth()/2, ofGetHeight());
        area_images[img_index].draw((ofGetWidth() - (area_images[img_index].getWidth() * area_img_expanded))/2, ofGetHeight() - (area_images[img_index].getHeight() * area_img_expanded), area_images[img_index].getWidth() * area_img_expanded, area_images[img_index].getHeight() * area_img_expanded);
    }
    
    // reflesh befor result
    if (finalLyricTime + 100 < music.getPositionMS() && !flushedAllLyric) {
        box2d.createBounds(0, 0, 0, 0);
        area_circle_obj->destroy();
        flushedAllLyric = true;
    }
    
    // draw result
    if (resultBeginTime < music.getPositionMS() && isCalcurated) {
        drawResult();
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
    if (key == 'l') {
        music.setPositionMS(music.getPositionMS() + 100);
    }
    if (key == 'k') {
        music.setPositionMS(music.getPositionMS() + 1000);
    }
    if (key == 's') {
        cout << music.getPositionMS() << endl;
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
            dy = 50;
            break;
        case 2:
            dy = -50;
            break;
        case 3:
            dx = 50;
            break;
        case 4:
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

//--------------------------------------------------------------
void ofApp::threadUpdate() {
    if(ofGetElapsedTimeMillis() >= next_execute_time) {
        execute_flag = true;
    }
    if(execute_flag){
        if(rotate_degree == 0){
            rotate_degree = 90;
            current_area_name = "A";
        }else if(rotate_degree == 90){
            rotate_degree = 180;
            current_area_name = "B";
        }else{
            rotate_degree = 0;
            current_area_name = "C";
        }
        servo_thread.sending(rotate_degree);
        
        next_execute_time += sensor_interval_ms;
        execute_flag = false;
    }
}

// -------------------------------------------------------------
void ofApp::setupResult(){
    // calucurating
    box2d.createBounds(0, 0, window_width, window_height);
    printf("area_a = %d, area_b = %d, area_c = %d\n", area_a, area_b, area_c);
    vector<pair<float,string> > pv;
    pv.push_back(make_pair(area_a,"A"));
    pv.push_back(make_pair(area_b,"B"));
    pv.push_back(make_pair(area_c,"C"));
    sort(pv.begin(),pv.end());
    reverse(pv.begin(),pv.end());
    for(int i = 0 ; i < 3 ; i++) {
        if (pv[i].second == "A") {
            rank["area_a"] = i + 1;
        } else if (pv[i].second == "B") {
            rank["area_b"] = i + 1;
        } else if (pv[i].second == "C") {
            rank["area_c"] = i + 1;
        }
    }
    
    // set rank images
    rank_img_A = rank_images[rank["area_a"] - 1];
    rank_img_B = rank_images[rank["area_b"] - 1];
    rank_img_C = rank_images[rank["area_c"] - 1];
    
    // calc drop proportion
    double sum = area_a + area_b + area_c;
    prop_area_A = area_a/sum;
    prop_area_B = area_b/sum;
    prop_area_C = area_c/sum;
    
    // calcurate drop end time
    double all_time = (musicEndTime - viewableRankTime) - (finalLyricTime + lyricClearMarginTime);
    vector<pair<double,string> > prop_pairs;
    double tmp_prop_A=0.0, tmp_prop_B=0.0, tmp_prop_C=0.0;
    prop_pairs.push_back(make_pair(prop_area_A, "area_a"));
    prop_pairs.push_back(make_pair(prop_area_B, "area_b"));
    prop_pairs.push_back(make_pair(prop_area_C, "area_c"));
    sort(prop_pairs.begin(),prop_pairs.end());
    reverse(prop_pairs.begin(),prop_pairs.end());
    for(int i = 0; i < 3 ; i++) {
        if (prop_pairs[i].second == "area_a") {
            for(int j = i; j < 3; j++){
                tmp_prop_A += prop_pairs[j].first;
            }
        } else if (prop_pairs[i].second == "area_b") {
            for(int j = i; j < 3; j++){
                tmp_prop_B += prop_pairs[j].first;
            }
        } else if (prop_pairs[i].second == "area_c") {
            for(int j = i; j < 3; j++){
                tmp_prop_C += prop_pairs[j].first;
            }
        }
    }

    drop_end_time_ms_A = (finalLyricTime + lyricClearMarginTime) + (all_time * tmp_prop_A);
    drop_end_time_ms_B = (finalLyricTime + lyricClearMarginTime) + (all_time * tmp_prop_B);
    drop_end_time_ms_C = (finalLyricTime + lyricClearMarginTime) + (all_time * tmp_prop_C);
    
    // make frames
    // area A
    cupLine.addVertex(10, 0);
    cupLine.addVertex(20, 0);
    cupLine.addVertex(20, ofGetHeight()-10);
    cupLine.addVertex(ofGetWidth()/3-20, ofGetHeight()-10);
    cupLine.addVertex(ofGetWidth()/3-20, 0);
    cupLine.addVertex(ofGetWidth()/3-10, 0);
    cupLine.addVertex(ofGetWidth()/3-10, ofGetHeight());
    cupLine.addVertex(ofGetWidth()/3, ofGetHeight());
    cupLine.addVertex(ofGetWidth()/3, 0);
    cupLine.addVertex(ofGetWidth()/3+10, 0);
    cupLine.addVertex(ofGetWidth()/3+10, ofGetHeight()-10);
    // area B
    cupLine.addVertex((ofGetWidth()*2/3)-20, ofGetHeight()-10);
    cupLine.addVertex((ofGetWidth()*2/3)-20, 0);
    cupLine.addVertex((ofGetWidth()*2/3)-10, 0);
    cupLine.addVertex((ofGetWidth()*2/3)-10, ofGetHeight());
    cupLine.addVertex((ofGetWidth()*2/3), ofGetHeight());
    cupLine.addVertex((ofGetWidth()*2/3), 0);
    cupLine.addVertex((ofGetWidth()*2/3)+10, 0);
    cupLine.addVertex((ofGetWidth()*2/3)+10, ofGetHeight()-10);
    // area C
    cupLine.addVertex(ofGetWidth()-20, ofGetHeight()-10);
    cupLine.addVertex(ofGetWidth()-20, 0);
    cupLine.addVertex(ofGetWidth()-10, 0);
    cupLine.addVertex(ofGetWidth()-10, ofGetHeight());
    cupLine.addVertex(10, ofGetHeight());
    cupLine.close();
    
    cup = ofPtr<ofxBox2dPolygon>(new ofxBox2dPolygon);
    cup.get()->addVertexes(cupLine);
    cup.get()->triangulatePoly(10);
    cup.get()->setPhysics(density, bounce, friction);
    cup.get()->create(box2d.getWorld());

}
void ofApp::drawResult() {
    // TODO don't use const number, use various number.
    // draw result popcones if exists
    for(int i = 0; i < result_viewable_particles.size(); i++){
        result_viewable_particles[i].get()->bake_level = 0.5;
        result_viewable_particles[i]->draw();
    }
    
    // draw texts for rank
    ofSetColor(255, 255, 255, 255);
    double roof_width_to_height_ratio = (ofGetWidth()/3)/yaneA.getWidth();
    yaneA.draw(0, 0, ofGetWidth()/3, yaneA.getHeight() * roof_width_to_height_ratio);
    yaneB.draw(ofGetWidth()/3, 0, ofGetWidth()/3, yaneB.getHeight() * roof_width_to_height_ratio);
    yaneC.draw(ofGetWidth()*2/3, 0, ofGetWidth()/3, yaneC.getHeight() * roof_width_to_height_ratio);
    textA.draw((ofGetWidth()*1/6) - (textA.getWidth() * area_img_expanded)/2, ofGetHeight() - (textA.getHeight() * area_img_expanded), textA.getWidth() * area_img_expanded, textA.getHeight() * area_img_expanded);
    textB.draw((ofGetWidth()*1/2) - (textB.getWidth() * area_img_expanded)/2, ofGetHeight() - (textB.getHeight() * area_img_expanded), textB.getWidth() * area_img_expanded, textB.getHeight() * area_img_expanded);
    textC.draw((ofGetWidth()*5/6) - (textC.getWidth() * area_img_expanded)/2, ofGetHeight() - (textC.getHeight() * area_img_expanded), textC.getWidth() * area_img_expanded, textC.getHeight() * area_img_expanded);
    
    // draw ranking image
    if (drop_end_time_ms_A + 500 < music.getPositionMS()){
        rank_img_A.draw((ofGetWidth()*1/6) - (rank_img_A.getWidth() * ranking_text_expand)/2, (ofGetHeight()/2) - (rank_img_A.getHeight() * ranking_text_expand)/2, rank_img_A.getWidth() * ranking_text_expand, rank_img_A.getHeight() * ranking_text_expand);
    }
    if (drop_end_time_ms_B + 500 < music.getPositionMS()){
        rank_img_B.draw((ofGetWidth()*1/2) - (rank_img_B.getWidth() * ranking_text_expand)/2, (ofGetHeight()/2) - (rank_img_B.getHeight() * ranking_text_expand)/2, rank_img_B.getWidth() * ranking_text_expand, rank_img_B.getHeight() * ranking_text_expand);
    }
    if (drop_end_time_ms_C + 500 < music.getPositionMS()){
        rank_img_C.draw((ofGetWidth()*5/6) - (rank_img_C.getWidth() * ranking_text_expand)/2, (ofGetHeight()/2) - (rank_img_C.getHeight() * ranking_text_expand)/2, rank_img_C.getWidth() * ranking_text_expand, rank_img_C.getHeight() * ranking_text_expand);
    }
}
// -------------------------------------------------------------
void ofApp::drawFeverText(){
    int current_time = music.getPositionMS();
    if(feverBeginTime < current_time && current_time < feverBeginTime + fever_text_animate_time){
        // animate fever time text
        float processed = ((float)current_time - (float)feverBeginTime) / (float)fever_text_animate_time;
        fevertime_img.draw(
                           ofGetWidth() - ((ofGetWidth() + (fevertime_img.getWidth() * fevertime_animating_img_expand)) * processed),
                           (ofGetHeight()/2) - (fevertime_img.getHeight() * fevertime_animating_img_expand)/2,
                           fevertime_img.getWidth() * fevertime_animating_img_expand,
                           fevertime_img.getHeight() * fevertime_animating_img_expand);
        
    } else if (feverBeginTime + fever_text_animate_time < current_time && current_time < feverEndTime){
        // draw text [fever time] on top right
        fevertime_img.draw(
                           ofGetWidth() - (fevertime_img.getWidth() * fevertime_img_expand) - 10,
                           10,
                           fevertime_img.getWidth() * fevertime_img_expand,
                           fevertime_img.getHeight() * fevertime_img_expand);
    }
}

// -------------------------------------------------------------
void ofApp::checkCollision(){
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
            
            if (viewable_particles[i][j].get()->collisioned_count == 5) {
                viewable_particles[i][j].get()->opacity = 0.7;
            } else if (viewable_particles[i][j].get()->collisioned_count == 10) {
                viewable_particles[i][j].get()->opacity = 0.3;
            } else if (viewable_particles[i][j].get()->collisioned_count == 12){;
                viewable_particles[i][j].get()->opacity = 1.0;
                viewable_particles[i][j].get()->bake_level = 0.5;
            } else if (viewable_particles[i][j].get()->collisioned_count == 15) {
                viewable_particles[i][j].get()->opacity = 1.0;
                viewable_particles[i][j].get()->bake_level = 0.7;
            } else if (viewable_particles[i][j].get()->collisioned_count > 20) {
                viewable_particles[i][j].get()->opacity = 0;
                viewable_particles[i][j].get()->destroy();
                viewable_particles[i].erase(viewable_particles[i].begin() + j );
                
                if(current_area_name == "A") {
                    area_a++;
                } else if(current_area_name == "B") {
                    area_b++;
                } else {
                    area_c++;
                }
            }
        }
    }
}
