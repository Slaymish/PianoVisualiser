#pragma once
#include <SDL2/SDL.h>
#include <vector>

class Piano {
public:
    Piano(int x,int y,int keyW,int keyH,int numKeys,double freqStep);
    void render(SDL_Renderer* r);
    void handleMouseClick(int mx,int my,bool down);
    std::vector<double> getActiveFrequencies() const;
    int getKeyIndexAt(int mx,int my) const;
    double getFrequencyAt(int index) const;
    // Update the geometry of the piano keys (preserving active state and frequencies)
    void setGeometry(int x, int y, int keyW, int keyH);
    // Update frequencies based on a new semitone step (preserving active state)
    void updateFrequencies(double freqStep);
private:
    struct Key { SDL_Rect rect; bool active; double frequency; };
    std::vector<Key> keys;
};
