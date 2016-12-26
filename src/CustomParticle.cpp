#include "CustomParticle.h"


CustomParticle::CustomParticle(vector<ofImage> images, string txt, float st_time, int f_size) {
    imgs = images;
    text = txt;
    font_size = f_size;
    font.load("yugothicbold.otf", font_size, true, true);
    //        bake_level = ofRandomuf();
    bake_level = 0.0;
    image_count = images.size();
    start_time = st_time;
    opacity = 1.0;
    collisioned_count = 0;
}

void CustomParticle::draw() {
    float radius = getRadius();
    float image_expand = 1.2;
    
    glPushMatrix();
    glTranslatef(getPosition().x, getPosition().y, 0);
    
    
    glRotatef(getRotation(), 0, 0, 1.0f);
    
    int tmp_img_num = (int)(image_count + 1) * bake_level;
    if (tmp_img_num == 0){
        ofSetColor(255,255,255,255*opacity);
        font.drawString(text, 0 - (font_size * 0.5), 0 + (font_size * 0.5));
    } else if (tmp_img_num == 1) {
        ofSetColor(255,255,255,255*opacity);
        imgs[tmp_img_num - 1].draw(0 - (font_size * 0.5), 0 - (font_size * 0.5), font_size * image_expand, font_size * image_expand);
    } else {
        ofSetColor(255,255,255,255*opacity);
        imgs[tmp_img_num - 1].draw(0 - (font_size * image_expand *  0.5), 0 - (font_size * image_expand * 0.5), font_size * image_expand, font_size * image_expand);
    }
    
    // --------- Debug only---------
    // ofSetColor(155,155,155);
    // ofDrawCircle(0, 0, radius);
    // -----------------------------
    
    glPopMatrix();
}
