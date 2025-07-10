// main.cpp
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <mutex>
#include <string>
#include <numeric>
#include <algorithm>
#include "piano.hpp"

constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 600;
constexpr int SAMPLE_RATE = 48000;
constexpr int AMPLITUDE = 28000;
constexpr double DEFAULT_SEMITONE_RATIO = 1.0594630943592953;

std::vector<double> activeFrequencies;
double globalPhase = 0.0;
std::mutex freqMutex;
double semitoneStep = DEFAULT_SEMITONE_RATIO;

// UI state
int hoveredKey = -1;
bool draggingSlider = false;
double viewScale = 1.0; // multiplier for viewing wave
SDL_Rect sliderBar = {SCREEN_WIDTH - 220, 10, 200, 20};

void audioCallback(void* userdata, Uint8* stream, int len) {
    (void)userdata;
    std::lock_guard<std::mutex> lock(freqMutex);
    Sint16* buffer = (Sint16*)stream;
    int samples = len / sizeof(Sint16);
    static double volume = 0.0;
    double targetVolume = activeFrequencies.empty() ? 0.0 : 1.0;
    for (int i = 0; i < samples; ++i) {
        volume += (targetVolume - volume) * 0.0005;
        double sample = 0.0;
        for (double freq : activeFrequencies) {
            sample += std::sin(globalPhase * freq * 2.0 * M_PI / SAMPLE_RATE) * AMPLITUDE;
        }
        globalPhase += 1.0;
        buffer[i] = static_cast<Sint16>((activeFrequencies.empty() ? 0 : (sample / activeFrequencies.size()) * volume));
    }
}

void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y) {
    SDL_Color white{255, 255, 255, 255};
    SDL_Surface* surf = TTF_RenderText_Blended(font, text.c_str(), white);
    if (!surf) {
        std::cerr << "TTF_RenderText_Blended failed: " << TTF_GetError() << "\n";
        return;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    if (!tex) {
        std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << "\n";
        SDL_FreeSurface(surf);
        return;
    }
    SDL_Rect dst{x, y, surf->w, surf->h};
    SDL_RenderCopy(renderer, tex, nullptr, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

int main(int /*argc*/, char* /*argv*/[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }
    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init failed: " << TTF_GetError() << "\n";
        SDL_Quit();
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("Platonic Piano Visualiser",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        TTF_Quit(); SDL_Quit();
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window); TTF_Quit(); SDL_Quit();
        return 1;
    }
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/Adwaita/AdwaitaMono-Bold.ttf", 20);
    if (!font) {
        std::cerr << "TTF_OpenFont failed: " << TTF_GetError() << "\n";
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window);
        TTF_Quit(); SDL_Quit();
        return 1;
    }
    SDL_AudioSpec want{}, have{};
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = audioCallback;
    if (SDL_OpenAudio(&want, &have) < 0) {
        std::cerr << "SDL_OpenAudio failed: " << SDL_GetError() << "\n";
        TTF_CloseFont(font); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window);
        TTF_Quit(); SDL_Quit();
        return 1;
    }
    SDL_PauseAudio(0);

    Piano piano(50, 400, 40, 150, 14, semitoneStep);
    bool quit = false; SDL_Event e;
    double wavePhase = 0.0;
    
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x, my = e.button.y;
                if (mx >= sliderBar.x && mx <= sliderBar.x + sliderBar.w && my >= sliderBar.y && my <= sliderBar.y + sliderBar.h) {
                    draggingSlider = true;
                } else {
                    piano.handleMouseClick(mx, my, true);
                }
            } else if (e.type == SDL_MOUSEBUTTONUP) {
                draggingSlider = false;
                piano.handleMouseClick(e.button.x, e.button.y, false);
            } else if (e.type == SDL_MOUSEMOTION) {
                int mx = e.motion.x, my = e.motion.y;
                hoveredKey = piano.getKeyIndexAt(mx, my);
                if (draggingSlider) {
                    double pct = double(mx - sliderBar.x) / sliderBar.w;
                    pct = std::clamp(pct, 0.0, 1.0);
                    viewScale = pct * 10.0; // 0 to 10x multiplier
                }
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_UP) semitoneStep *= 1.01;
                else if (e.key.keysym.sym == SDLK_DOWN) semitoneStep /= 1.01;
                piano = Piano(50, 400, 40, 150, 14, semitoneStep);
            }
        }
        {
            std::lock_guard<std::mutex> lock(freqMutex);
            activeFrequencies = piano.getActiveFrequencies();
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        // render slider bar background
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        SDL_RenderFillRect(renderer, &sliderBar);
        // render slider knob
        int knobX = sliderBar.x + int((viewScale / 10.0) * sliderBar.w) - 5;
        SDL_Rect knob = {knobX, sliderBar.y - 5, 10, sliderBar.h + 10};
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderFillRect(renderer, &knob);
        // calculate frequencies
        double combinedFreq = 0.0;
        if (!activeFrequencies.empty()) {
            // compute RMS frequency
            double sumSquares = std::inner_product(activeFrequencies.begin(), activeFrequencies.end(),
                                                   activeFrequencies.begin(), 0.0);
            combinedFreq = std::sqrt(sumSquares / activeFrequencies.size());
        }
        double displayFreq = combinedFreq * (activeFrequencies.empty() ? 0.0 : viewScale);
        // render frequency texts
        renderText(renderer, font, "Combined Freq: " + std::to_string(int(combinedFreq)) + " Hz", 10, 10);
        renderText(renderer, font, "Display Freq: " + std::to_string(int(displayFreq)) + " Hz", 10, 35);
        // draw moving waveform above keys
        int waveY = 100, waveH = 100;
        for (int x = 0; x < SCREEN_WIDTH - 1; ++x) {
            double t1 = wavePhase + 2 * M_PI * displayFreq * x / SAMPLE_RATE;
            double t2 = wavePhase + 2 * M_PI * displayFreq * (x + 1) / SAMPLE_RATE;
            int y1 = waveY + waveH / 2 + int(std::sin(t1) * waveH / 2);
            int y2 = waveY + waveH / 2 + int(std::sin(t2) * waveH / 2);
            SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
            SDL_RenderDrawLine(renderer, x, y1, x + 1, y2);
        }
        wavePhase += 2 * M_PI * displayFreq / (SCREEN_WIDTH);
        piano.render(renderer);
        // show hovered key freq
        if (hoveredKey >= 0) {
            double f = piano.getFrequencyAt(hoveredKey);
            renderText(renderer, font, "Key freq: " + std::to_string(int(f)) + " Hz", 200, 10);
        }
        SDL_RenderPresent(renderer);
    }
    SDL_CloseAudio(); TTF_CloseFont(font); TTF_Quit(); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
    return 0;
}

