#!/bin/bash

# === Configuration ===
INSTALLER_CERT="Developer ID Installer: Adrian Fekete (FQJJN5KH4A)"  
PLUGIN_NAME="$1"
PKG_NAME="${PLUGIN_NAME}.pkg"

# Parse arguments
while getopts "n" opt; do
  case $opt in
    n)
      NOTARIZE=true
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
  esac
done
shift $((OPTIND -1))

# Paths to notarized VST3 and AU plugins
VST3_PATH="./build/artifacts/VST3/${PLUGIN_NAME}.vst3"
AU_PATH="./build/artifacts/AU/${PLUGIN_NAME}.component"

# Validate input
if [ -z "$PLUGIN_NAME" ]; then
    echo "❌ Error: No plugin name provided!"
    echo "Usage: ./create_pkg.sh YourPlugin"
    exit 1
fi

# Create a temporary folder for PKG contents
PKG_FOLDER="${PLUGIN_NAME}_PKG"
mkdir -p "$PKG_FOLDER/Library/Audio/Plug-Ins/VST3"
mkdir -p "$PKG_FOLDER/Library/Audio/Plug-Ins/Components"

# Move notarized plugins into PKG folder
if [ -d "$VST3_PATH" ]; then
    cp -R "$VST3_PATH" "$PKG_FOLDER/Library/Audio/Plug-Ins/VST3/"
else
    echo "⚠️ Notarized VST3 file not found: $VST3_PATH"
fi

if [ -d "$AU_PATH" ]; then
    cp -R "$AU_PATH" "$PKG_FOLDER/Library/Audio/Plug-Ins/Components/"
else
    echo "⚠️ Notarized AU file not found: $AU_PATH"
fi

# === Create PKG Installer ===
if [ "$(ls -A $PKG_FOLDER/Library/Audio/Plug-Ins/)" ]; then
    echo "📦 Creating PKG installer: $PKG_NAME..."
    pkgbuild --root "$PKG_FOLDER" --identifier "com.yourcompany.${PLUGIN_NAME}" --version "1.0" --install-location "/" --sign "$INSTALLER_CERT" "$PKG_NAME"
    
    # Notarization step
    if [ "$NOTARIZE" = true ]; then
        echo "🔒 Notarizing the package..."
        # Submit PKG for notarization
        echo "🚀 Submitting PKG for notarization..."
        xcrun notarytool submit "$PKG_NAME" --keychain-profile "NotaryProfile" --wait

        # Staple notarization ticket to PKG
        echo "📌 Stapling notarization ticket for PKG..."
        xcrun stapler staple "$PKG_NAME"

        echo "🎉 PKG notarized and ready: $PKG_NAME"
    else
        echo "⏩ Skipping notarization step."
    fi
    
else
    echo "⚠️ No notarized plugins found, skipping PKG creation."
fi

# Cleanup
rm -rf "$PKG_FOLDER"