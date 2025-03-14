#pragma once

#include "PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>

//==============================================================================
class PluginProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PluginProcessorEditor (PluginProcessor&);
    ~PluginProcessorEditor() override;

    //==============================================================================
    //void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginProcessor& processor;

    void timerCallback() override;

    int loadedPresetOptions = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessorEditor)
};
