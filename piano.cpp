// piano.cpp
#include "piano.hpp"
#include <cmath>

Piano::Piano(int x,int y,int keyW,int keyH,int numKeys,double step) {
    double base = 261.63;  // Middle C frequency
    keys.clear();
    for(int i = 0; i < numKeys; ++i) {
        Key k;
        k.rect = { x + i * keyW, y, keyW - 2, keyH };
        k.active = false;
        k.frequency = base * std::pow(step, i);
        keys.push_back(k);
    }
}

void Piano::render(SDL_Renderer* r) {
    for(auto& k : keys) {
        SDL_SetRenderDrawColor(r,
            k.active ? 200 : 255,
            k.active ? 100 : 255,
            k.active ? 255 : 255,
            255
        );
        SDL_RenderFillRect(r, &k.rect);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
        SDL_RenderDrawRect(r, &k.rect);
    }
}

void Piano::handleMouseClick(int mx,int my,bool down) {
    for(auto& k : keys) {
        if(mx >= k.rect.x && mx <= k.rect.x + k.rect.w &&
           my >= k.rect.y && my <= k.rect.y + k.rect.h) {
            k.active = down;
        }
    }
}

std::vector<double> Piano::getActiveFrequencies() const {
    std::vector<double> out;
    for(auto& k : keys) if(k.active) out.push_back(k.frequency);
    return out;
}

int Piano::getKeyIndexAt(int mx,int my) const {
    for(int i = 0; i < (int)keys.size(); ++i) {
        auto& k = keys[i];
        if(mx >= k.rect.x && mx <= k.rect.x + k.rect.w &&
           my >= k.rect.y && my <= k.rect.y + k.rect.h) {
            return i;
        }
    }
    return -1;
}

double Piano::getFrequencyAt(int idx) const {
    if(idx >= 0 && idx < (int)keys.size()) return keys[idx].frequency;
    return 0.0;
}

void Piano::setGeometry(int x, int y, int keyW, int keyH) {
    for (size_t i = 0; i < keys.size(); ++i) {
        keys[i].rect = { x + int(i) * keyW, y, keyW - 2, keyH };
    }
}

void Piano::updateFrequencies(double step) {
    const double base = 261.63;  // Middle C frequency
    for (size_t i = 0; i < keys.size(); ++i) {
        keys[i].frequency = base * std::pow(step, double(i));
    }
}

