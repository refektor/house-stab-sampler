#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginProcessorEditor::PluginProcessorEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    juce::ignoreUnused (processor);

    addAndMakeVisible(*processor.webView);
    setResizable(false, false);
    setSize (700, 700); // Set the size of your plugin window
    startTimer(50);
}

PluginProcessorEditor::~PluginProcessorEditor()
{
    stopTimer();
    processor.webView->setVisible(false); // Ensure it's hidden when the UI is closed
}

void PluginProcessorEditor::timerCallback()
{
    // Update VU meter needle
    auto volumeLevel = processor.getMagnitude();
    juce::String jsCode = "updateNeedle(" + juce::String(volumeLevel) + ");\n";

    // Update mix parameters for automation from DAW
    jsCode += "setReverbKnob(" + juce::String(processor.getReverbMix()) + ");\n";
    jsCode += "setDelayKnob(" + juce::String(processor.getDelayMix()) + ");\n";
    jsCode += "setChorusKnob(" + juce::String(processor.getChorusMix()) + ");\n";
    jsCode += "setDriveKnob(" + juce::String(processor.getDriveMix()) + ");\n";
    jsCode += "setAttackKnob(" + juce::String(processor.getAttack()) + ");\n";
    jsCode += "setDecayKnob(" + juce::String(processor.getDecay()) + ");\n";
    jsCode += "setSustainKnob(" + juce::String(processor.getSustain()) + ");\n";
    jsCode += "setReleaseKnob(" + juce::String(processor.getRelease()) + ");\n";
    processor.webView->evaluateJavascript(jsCode);


     
}

//==============================================================================


void PluginProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    //webView.setBounds(getLocalBounds());
    processor.webView->setBounds(getLocalBounds());
}