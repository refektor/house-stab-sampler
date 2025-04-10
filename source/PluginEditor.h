#pragma once

#include "PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>

//==============================================================================
class PluginEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    //void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginProcessor& processor;

    void timerCallback() override;

    int loadedPresetOptions = false;

    juce::WebSliderRelay reverbSliderRelay;
    juce::WebSliderRelay delaySliderRelay;
    juce::WebSliderRelay chorusSliderRelay;
    juce::WebSliderRelay driveSliderRelay;
    juce::WebSliderRelay attackSliderRelay;
    juce::WebSliderRelay decaySliderRelay;
    juce::WebSliderRelay sustainSliderRelay;
    juce::WebSliderRelay releaseSliderRelay;
    juce::WebComboBoxRelay presetRelay;
    
    juce::WebBrowserComponent webView;

    juce::WebSliderParameterAttachment reverbSliderAttachment;
    juce::WebSliderParameterAttachment delaySliderAttachment;
    juce::WebSliderParameterAttachment chorusSliderAttachment;
    juce::WebSliderParameterAttachment driveSliderAttachment;
    juce::WebSliderParameterAttachment attackSliderAttachment;
    juce::WebSliderParameterAttachment decaySliderAttachment;
    juce::WebSliderParameterAttachment sustainSliderAttachment;
    juce::WebSliderParameterAttachment releaseSliderAttachment;
    juce::WebComboBoxParameterAttachment presetAttachment;

    using Resource = juce::WebBrowserComponent::Resource;
    std::optional<Resource> getResource(const juce::String& url);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
