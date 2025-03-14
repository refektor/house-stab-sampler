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
    auto volumeLevel = processor.getMagnitude();
    juce::String jsCode = "updateNeedle(" + juce::String(volumeLevel) + ");";
    // Assuming you have a WebBrowserComponent named webBrowser
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