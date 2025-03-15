#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_dsp/juce_dsp.h>

class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    
    void setPreset(int index);
    float getMagnitude() const { return volumeLevel; }

    void setAttack(float newAttack) { 
        attack = newAttack;
        applySoundADSRParams();
    }
    void setDecay(float newDecay) { 
        decay = newDecay;
        applySoundADSRParams();
    }
    void setSustain(float newSustain) { 
        sustain = newSustain;
        applySoundADSRParams();
    }
    void setRelease(float newRelease) { 
        release = newRelease;
        applySoundADSRParams();
    }

    void setReverbMix(float mix) { 
        reverbParams.wetLevel = mix;
        reverbParams.dryLevel = 1.0f - mix;
        reverb.setParameters(reverbParams);
    }

    void setDelayMix(float mix) {
        this->delayMix = mix;
    }

    void setChorusMix(float mix) {
        chorus.setMix(mix);
    }

    void setSaturationDrive(float drive) {
        saturationDrive = drive;
    }

    std::unique_ptr<juce::WebBrowserComponent> webView;

private:
    juce::Synthesiser sampler;
    juce::AudioFormatManager formatManager;
    float volumeLevel = 0.0f;
    juce::SamplerSound *currentSound = nullptr;

    double m_sampleRate = 0.0;

    // ADSR values
    float attack = 0.1f;
    float decay = 0.1f;
    float sustain = 1.0f;
    float release = 0.1f;

    // Delay
    juce::dsp::DelayLine<float> delayLine { 44100 }; // 1 second delay at 44.1kHz sample rate
    float delayTime = 0.5f; // 500ms delay
    float feedback = 0.5f; // 50% feedback
    float delayMix = 0.0f; // 50% wet/dry mix

    // Reverb
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;

    // Chorus
    juce::dsp::Chorus<float> chorus;

    // Saturation
    float saturationDrive = 1.0f;


    using Resource = juce::WebBrowserComponent::Resource;
    std::optional<Resource> getResource(const juce::String& url);
    
    void applySoundADSRParams();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};