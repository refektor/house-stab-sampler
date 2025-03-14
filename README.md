# House Stab Sampler

## Description

This project is a VST/plug-in sampler that contains house stab samples.

## Contributing

### Building the Plug-in

```
cmake -S . -B build
cmake --build build
```

### UI Development

1. Navigate to `source/ui/public`
2. Run `python3 -m http.server 8000`
3. Open a browser and navigate to `http://localhost:8000` to see and test the UI
