# Sampler Template
## Description
This project serves as a template for creating new sampler plugins.

## Steps to create sampler plugin
1. Clone this repo.
2. Add presets to `presets/` directory.
3. Change `NUM_PRESETS` in `source/ui/public/js/index.js` to the number of presets that you put into the `presets/` directory.
4. Change `index.html` and `styles.css` to modify the UI look & feel.
5. Change PluginProcessor.cpp::184 for the path to the UI.

## Building
```
cmake -S . -B build
cmake --build build
```