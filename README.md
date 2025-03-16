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

## Packaging
### Notarization
Notairzation is only required when preparing for distribution. Run the following command (Note the <Plugin Name> should be the same as `PRODUCT_NAME` in `CMakeLists.txt`):
```notarize.sh <Plugin Name>```

### Packaging for Mac
To package the plugin for easy installation on Mac, run the following commant:
```create_pkg.sh```

To prepare the PKG for distribution, run:
```create_pkg.sh -n```
