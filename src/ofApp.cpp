#include "ofApp.h"
#include "map"
#include "algorithm"
using namespace std;

//--------------------------------------------------------------
void ofApp::setup() {
    // file setting
    json_file_name      = "sync_koi_hoshinogen.json";
    music_file_name     = "koi_hoshinogen.mp3";
    
    // draw setting
    font_size           = 30;
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
    
    // thread setting
    sensor_interval_ms  = 10000;
    rotate_degree       = 0;
    current_area_name   = "A";
    next_execute_time   = ofGetElapsedTimeMillis();
    servo_thread.sending(rotate_degree);
    
    // rank setting
    area_a = 0;
    area_b = 0;
    area_c = 0;
    area_img_expanded = 0.3;
    
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
    printf("test3\n");
    ofSetWindowShape(window_width, window_height);
    
    // area circle
    int area_num = dir.listDir("areas");
    for (int i=0; i<area_num; i++) {
        area_images.push_back(ofImage(dir.getPath(i)));
    }
    area_circle_obj = new ofxBox2dCircle();
    area_circle_obj->setup(box2d.getWorld(), ofGetWidth()/2, ofGetHeight(), area_images[0].getWidth() * (area_img_expanded * 0.75));
    
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
    
    // --- [DEBUG ONLY] Start Music Position ---
    music.setPositionMS(2000);
    // -----------------------------------------
    
    //snow
    finder.setup("haarcascade_frontalface_default.xml");
    
    snow_img.load("popcorn.png");
    fevertime_img.load("fevertime.png");
    fevertime_img.setAnchorPoint(0.5, 0.5);
    xpos = ofGetWidth()/2;
    ypos = ofGetHeight()/2;
    xspeed = -15;
    
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

vector<shared_ptr<CustomParticle>> ofApp::getCustomObj(int line_index, int x, int y){
    // create line obj
    vector<shared_ptr<CustomParticle>> tmp_obj;
    string test = "t";
    tmp_obj.push_back(shared_ptr<CustomParticle>(new CustomParticle(images, test, 0, font_size)));
    
    // set physics
    for(int i = 0; i < tmp_obj.size(); i++) {
        tmp_obj[i].get()->setPhysics(density, bounce, friction);
        tmp_obj[i].get()->setup(box2d.getWorld(), x, y, 20);
    }
    return tmp_obj;
}
//--------------------------------------------------------------
void ofApp::update() {
    // move depend on phisic setting
    box2d.update();
    
    // thread update
    threadUpdate();
    
    // set fevertime motion
    xpos += xspeed;
    flag_motion = false;
    if(xpos < -1270){
        flag_motion = true;
    }
    
    // camera captured
    bool bNewFrame = false;
    vidGrabber.update();
    bNewFrame = vidGrabber.isFrameNew();
    /*fevertime set*/ //TODO
    feverTimeFlag = music.getPositionMS() > feverBeginTime && music.getPositionMS() < resultBeginTime;
    resultTimeFlag = music.getPositionMS() > resultBeginTime;
    resultTimeFlagment = music.getPositionMS() < resultBeginTime;
    // cout << resultTimeFlagment << endl;
    
    
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
    
    // result
    
    // judge next lyric line started
    int tail_index = viewable_particles.size() - 1;
    
    float music_pos = music.getPositionMS();
    next_lyric_ms = sync_lyric_json["lines"][loaded_line_head]["time"].asDouble();
    if (music_pos + margin_time > next_lyric_ms && loaded_line_head < sync_lyric_json["lines"].size()) {
        // next lyric line add viewable obj
        viewable_particles.push_back(getLineObj(loaded_line_head));
        // set position of now lyric under next lyric
        tail_index = viewable_particles.size() - 1;
        
        for(int i = 0; i < viewable_particles[tail_index].size() && !feverTimeFlag; i++){
            viewable_particles[tail_index][i].get()->setPosition(
                                                                 (ofGetWidth() - viewable_particles[tail_index].size() * (font_size + word_margin))/2 + (i * (font_size + word_margin)),
                                                                 start_point_y + font_size);
        }
        // line indenting
        loaded_line_head++;
    }
    
    // fix now lyric position
    if(tail_index >= 0){
        for(int i = 0; i < viewable_particles[tail_index].size() && !feverTimeFlag; i++){
            viewable_particles[tail_index][i].get()->setPosition(
                                                                 (ofGetWidth() - viewable_particles[tail_index].size() * (font_size + word_margin))/2 + (i * (font_size + word_margin)),
                                                                 start_point_y);
        }
    }
    
    // change box2d bound size if change window size
    if (window_width != ofGetWidth() || window_height != ofGetHeight()) {
        // update viewable particles position
        for(int i = 0; i < viewable_particles.size() && !feverTimeFlag; i++) {
            for (int j = 0; j < viewable_particles[i].size(); j++) {
                viewable_particles[i][j].get()->setPosition(
                                                            (ofGetWidth() - viewable_particles[i].size() * (font_size + word_margin))/2 + (j * (font_size + word_margin)),
                                                            start_point_y);
            }
        }
        window_width = ofGetWidth();
        window_height = ofGetHeight();
        box2d.createBounds(0, 0, window_width, window_height);
        printf("test4\n");
        // capture camera view
        vidGrabber.update();
    }
    
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
            
            if (viewable_particles[i][j].get()->collisioned_count == 5) {
                viewable_particles[i][j].get()->opacity = 0.7;
            } else if (viewable_particles[i][j].get()->collisioned_count == 10) {
                viewable_particles[i][j].get()->opacity = 0.3;
            } else if (viewable_particles[i][j].get()->collisioned_count == 15){;
                viewable_particles[i][j].get()->opacity = 1.0;
                viewable_particles[i][j].get()->bake_level = 0.5;
            } else if (viewable_particles[i][j].get()->collisioned_count == 20) {
                viewable_particles[i][j].get()->opacity = 1.0;
                viewable_particles[i][j].get()->bake_level = 0.7;
            } else if (viewable_particles[i][j].get()->collisioned_count > 50) {
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
    
    //image.setFromPixels(vidGrabber.getPixels().getData(), window_width, window_height, OF_IMAGE_COLOR);
    //  face detection
    if(loopCnt % judgePoint == 0) finder.findHaarObjects(grayImage, 10, 10);
    loopCnt++;
    
    // sound update
    ofSoundUpdate();
}

//--------------------------------------------------------------
void ofApp::draw() {
    printf("time : %d\n", music.getPositionMS());
    // draw camera caputured
    // TODO reverse capture image
    ofSetColor(255, 255, 255, 255 * camera_draw_opacity);
    vidGrabber.draw(0,0);
    
    // draw snow
    ofSetLineWidth(3);
    ofNoFill();
    
    int control_size_x = 80;
    int control_size_y = 200;
    feverTimeFlag = music.getPositionMS() > feverBeginTime && music.getPositionMS() < resultBeginTime;
    if(feverTimeFlag) {
        for(int i = 0; i < finder.blobs.size(); i++) {
            ofRectangle cur = finder.blobs[i].boundingRect;
            ofSetColor(255, 255, 255);
            snow_img.draw(cur.x - control_size_x/2 ,cur.y - control_size_y/2 - 50, cur.width + control_size_x, cur.height + control_size_y);
            // braw fever time popcorn
            if(loopCnt % 15 == 0){
                viewable_particles.push_back(getCustomObj(loaded_line_head, cur.x + cur.width/2, cur.y + cur.height/2));
                int idx = viewable_particles[loaded_line_head].size() - 1;
                viewable_particles[loaded_line_head][idx].get()->addRepulsionForce(cur.x + (cur.width/2 + control_size_x) + (rand()%100 - rand()%100), cur.y + (cur.height/2 + control_size_y) + (rand()%100 - rand()%100), 100);
                
                viewable_particles[loaded_line_head][idx].get()->bake_level = 0.8;
                viewable_particles[loaded_line_head][idx]->draw();
                loaded_line_head++;
            }
        }
    }
    ofSetColor(255, 255, 255);
    fevertime_img.draw(xpos, 0, ofGetWidth(), ofGetHeight());
    if(flag_motion) {
        fevertime_img.draw(ofGetWidth()-250, 15, 230, 80);
        
    }
    
    // draw viewable lyrics
    for(int i = 0; i < viewable_particles.size(); i++){
        for(int j = 0; j < viewable_particles[i].size(); j++){
            viewable_particles[i][j]->draw();
            //viewable_particles[i][j];
        }
    }
    
    // TODO What's doing here ??
//    for(int i = 0; i < result_viewable_particles.size(); i++){
//        for(int j = 0; j < result_viewable_particles[i].size(); j++){
//            result_viewable_particles[i][j].get()->bake_level = 0.7;
//            result_viewable_particles[i][j]->draw();
//        }
//    }
    
    // motion section
    for (int i = 0; i < contourFinder.nBlobs; i++){
        if(contourFinder.blobs[i].hole){
            bLearnBakground = true;
        }
    }
    motionCount = contourFinder.nBlobs;
    drawCount++;
    
    // judge jump motion
    if(contourFinder.nBlobs > number_of_object && abs(contourFinder.nBlobs - lastContourFinder.nBlobs) > diff_param && (time(NULL) - lastJumpTime) > tracking_interval && !resultTimeFlag) {
        lastJumpTime = time(NULL);
        int d = motionVector(contourFinder, lastContourFinder);
        jumpPopcones(d);
    }
    if (12750 <= music.getPositionMS() && music.getPositionMS() <= 211000) {
        if (music.getPositionMS() <= 77033 || 88039 <= music.getPositionMS()) {
            
            // draw current area image
            int img_index;
            if(current_area_name == "A"){
                img_index = 0;
            }else if(current_area_name == "B"){
                img_index = 1;
            }else{
                img_index = 2;
            }
            ofSetColor(255,255,255);
            area_images[img_index].draw((ofGetWidth() - (area_images[img_index].getWidth() * area_img_expanded))/2, ofGetHeight() - (area_images[img_index].getHeight() * area_img_expanded), area_images[img_index].getWidth() * area_img_expanded, area_images[img_index].getHeight() * area_img_expanded);
        }
    }
    
    
    //reflesh befor result
    if ((resultBeginTime - 8000) <= music.getPositionMS() && music.getPositionMS() <= resultBeginTime) {
        box2d.createBounds(0, 0, 0, 0);
        printf("test1\n");
    }
    
    // draw result
    if (resultBeginTime < music.getPositionMS()) {
        // isCalcurate
        if (!isCalcurating) {
            box2d.createBounds(0, 0, window_width, window_height);
            printf("test2\n");
            printf("area_a = %d, area_b = %d, area_c = %d\n", area_a, area_b, area_c);
            std::unordered_map<std::string, int> rank = {
                {"area_a", 0},
                {"area_b", 0},
                {"area_c", 0},
            };
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
            if (rank["area_a"] == 1) { rank1 = ofGetWidth()*1/6-50;}
            else if (rank["area_b"] == 1) { rank1 = ofGetWidth()*1/2-50;}
            else if (rank["area_c"] == 1) { rank1 = ofGetWidth()*5/6-50;}
            if (rank["area_a"] == 2) { rank2 = ofGetWidth()*1/6-50;}
            else if (rank["area_b"] == 2) { rank2 = ofGetWidth()*1/2-50;}
            else if (rank["area_c"] == 2) { rank2 = ofGetWidth()*5/6-50;}
            if (rank["area_a"] == 3) { rank3 = ofGetWidth()*1/6-50;}
            else if (rank["area_b"] == 3) { rank3 = ofGetWidth()*1/2-50;}
            else if (rank["area_c"] == 3) { rank3 = ofGetWidth()*5/6-50;}
            isCalcurating = true;
            drawResult();
            printf("areaA_rank = %d, areaA_rank = %d, areaA_rank = %d\n", rank["area_a"], rank["area_b"], rank["area_c"]);
        }
        ofSetColor(255, 255, 255, 255);
        first.draw(rank1, 120, 100, 70);
        second.draw(rank2, 120, 100, 70);
        third.draw(rank3, 120, 100, 70);
        yaneA.draw(10, 0, 320, 100);
        yaneB.draw(ofGetWidth()/3, 0, 320, 100);
        yaneC.draw(ofGetWidth()*2/3, 0, 320, 100);
        textA.draw(ofGetWidth()*1/6-50, ofGetHeight() - 105, 150, 105);
        textB.draw(ofGetWidth()*1/2-50, ofGetHeight() - 105, 150, 105);
        textC.draw(ofGetWidth()*5/6-50, ofGetHeight() - 105, 150, 105);
        // drop pop
        if (!resultCalcuratedA) {
            result_viewable_particles.push_back(getCustomObj(loaded_line_head, ofGetWidth()/6+ofRandom(20), 0));
            drop_count_a ++;
            printf("drop_count_a = %d\n", drop_count_a);
            if (area_a == drop_count_a) {
                resultCalcuratedA = true;
            }
        }
        if (!resultCalcuratedB) {
            result_viewable_particles.push_back(getCustomObj(loaded_line_head, ofGetWidth()/2+ofRandom(20), 0));
            drop_count_b ++;
            printf("drop_count_b = %d\n", drop_count_b);
            if (area_b == drop_count_b) {
                resultCalcuratedB = true;
            }
        }
        if (!resultCalcuratedC) {
            result_viewable_particles.push_back(getCustomObj(loaded_line_head, ofGetWidth()*5/6+ofRandom(20), 0));
            drop_count_c ++;
            printf("drop_count_c = %d\n", drop_count_c);
            if (area_c == drop_count_c) {
                resultCalcuratedC = true;
            }
        }
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
void ofApp::drawResult() {
    // clear all popcones
//    for(int i = 0; i < viewable_particles.size(); i++){
//        for(int j = 0; j < viewable_particles[i].size(); j++){
//            viewable_particles[i][j].get()->destroy();
//            viewable_particles.erase(viewable_particles.begin());
//        }
//    }
    // image images
    yaneA.load("images/yane_A.png");
    yaneB.load("images/yane_B.png");
    yaneC.load("images/yane_C.png");
    textA.load("areas/text_A.png");
    textB.load("areas/text_B.png");
    textC.load("areas/text_C.png");
    first.load("images/1st.png");
    second.load("images/2nd.png");
    third.load("images/3rd.png");;
    // make frame
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
    cupLine.addVertex((ofGetWidth()*2/3)-20, ofGetHeight()-10);
    cupLine.addVertex((ofGetWidth()*2/3)-20, 0);
    cupLine.addVertex((ofGetWidth()*2/3)-10, 0);
    cupLine.addVertex((ofGetWidth()*2/3)-10, ofGetHeight());
    cupLine.addVertex((ofGetWidth()*2/3), ofGetHeight());
    cupLine.addVertex((ofGetWidth()*2/3), 0);
    cupLine.addVertex((ofGetWidth()*2/3)+10, 0);
    cupLine.addVertex((ofGetWidth()*2/3)+10, ofGetHeight()-10);
    cupLine.addVertex(ofGetWidth()-20, ofGetHeight()-10);
    cupLine.addVertex(ofGetWidth()-20, 0);
    cupLine.addVertex(ofGetWidth()-10, 0);
    cupLine.addVertex(ofGetWidth()-10, ofGetHeight());
    cupLine.addVertex(10, ofGetHeight());
    cupLine.close();
    cup = ofPtr<ofxBox2dPolygon>(new ofxBox2dPolygon);
    cup.get()->addVertexes(cupLine);
    cup.get()->triangulatePoly(10);
    cup.get()->setPhysics(0.0, 0.5, 0.1);
    cup.get()->create(box2d.getWorld());
}
