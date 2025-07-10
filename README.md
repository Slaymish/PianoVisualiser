# Piano Waveform Visualiser

This project visualises the combination of multiple piano key frequencies into a single “platonic” sound. The interactive piano keyboard lets you select keys, combine their frequencies, and view the resulting waveform and frequency information in real time.

## Features

- Interactive on-screen piano keyboard (14 keys)
- Real-time audio synthesis using SDL2
- Display of combined frequency (RMS) and scaled display frequency
- Adjustable view scale via on-screen slider or Up/Down arrow keys
- Hover over keys to see individual key frequencies

## Dependencies

- SDL2
- SDL2_ttf
- C++17 compiler


## Build

```bash
g++ -std=c++17 main.cpp piano.cpp -o piano_waveform `sdl2-config --cflags --libs` -lSDL2_ttf -lm
```

## Usage

```bash
./piano_waveform
```

- Click and drag on the slider bar to adjust the view scale (0× to 10×).
- Press Up/Down arrow keys to adjust the semitone ratio.
- Click piano keys to toggle their active state.
- Hover over keys to display their individual frequency.

## Future Work

- Live MIDI / WAV file input
- 3D vortex visualisation with interactive editing by dragging on the vortex

