#!/bin/bash

# === Configuration ===
DEVELOPER_ID="Developer ID Application: Adrian Fekete (FQJJN5KH4A)"  # Replace with your Apple Developer ID
NOTARY_PROFILE="NotaryProfile"  # Your stored notarization profile

# Plugin name without extension (passed as argument)
PLUGIN_NAME="$1"

# Paths to VST3 and AU plugins
VST3_PATH="./build/artifacts/VST3/${PLUGIN_NAME}.vst3"
AU_PATH="./build/artifacts/AU/${PLUGIN_NAME}.component"

# Validate input
if [ -z "$PLUGIN_NAME" ]; then
    echo "❌ Error: No plugin name provided!"
    echo "Usage: ./notarize_vst3_au.sh YourPlugin"
    exit 1
fi

# Track notarization status
NOTARIZATION_FAILED=0

# === Notarize VST3 ===
if [ -d "$VST3_PATH" ]; then
    echo "🔏 Signing VST3: $VST3_PATH..."
    codesign --force --deep --timestamp --options runtime --sign "$DEVELOPER_ID" "$VST3_PATH"
    
    echo "📦 Creating ZIP for notarization..."
    zip -r "${PLUGIN_NAME}_VST3.zip" "$VST3_PATH"

    echo "🚀 Submitting VST3 for notarization..."
    xcrun notarytool submit "${PLUGIN_NAME}_VST3.zip" --keychain-profile "$NOTARY_PROFILE" --wait

    echo "📌 Stapling notarization ticket for VST3..."
    xcrun stapler staple "$VST3_PATH"
else
    echo "⚠️ VST3 file not found: $VST3_PATH"
    NOTARIZATION_FAILED=1
fi

# === Notarize AU ===
if [ -d "$AU_PATH" ]; then
    echo "🔏 Signing AU: $AU_PATH..."
    codesign --force --deep --timestamp --options runtime --sign "$DEVELOPER_ID" "$AU_PATH"

    echo "📦 Creating ZIP for notarization..."
    zip -r "${PLUGIN_NAME}_AU.zip" "$AU_PATH"

    echo "🚀 Submitting AU for notarization..."
    xcrun notarytool submit "${PLUGIN_NAME}_AU.zip" --keychain-profile "$NOTARY_PROFILE" --wait

    echo "📌 Stapling notarization ticket for AU..."
    xcrun stapler staple "$AU_PATH"
else
    echo "⚠️ AU file not found: $AU_PATH"
    NOTARIZATION_FAILED=1
fi

# === Summary ===
if [ $NOTARIZATION_FAILED -ne 0 ]; then
    echo "⚠️ Some notarizations failed. Check the output above."
    exit 1
else
    echo "✅ Both VST3 and AU notarized successfully!"
    exit 0
fi