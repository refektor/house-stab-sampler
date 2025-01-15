#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

//==============================================================================
PluginProcessorEditor::PluginProcessorEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    juce::ignoreUnused (processor);

    addAndMakeVisible(*processor.webView);
    setResizable(false, false);
    setSize (400, 400); // Set the size of your plugin window
    startTimer(50);
}

PluginProcessorEditor::~PluginProcessorEditor()
{
    stopTimer();
    processor.webView->setVisible(false); // Ensure it's hidden when the UI is closed
}

void PluginProcessorEditor::timerCallback()
{
    auto rmsLevel = processor.getRMSLevel();
    juce::String jsCode = "updateNeedle(" + juce::String(rmsLevel) + ");";
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