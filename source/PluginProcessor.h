#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <algorithm>

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

    float getAttack() const { return attack; }

    void setDecay(float newDecay) { 
        decay = newDecay;
        applySoundADSRParams();
    }

    float getDecay() const { return decay; }
    
    void setSustain(float newSustain) { 
        sustain = newSustain;
        applySoundADSRParams();
    }

    float getSustain() const { return sustain; }

    void setRelease(float newRelease) { 
        // a release of 0.0f will cause a clicking sound, so we set it to 0.01f
        if (newRelease == 0.0f) {
            newRelease = 0.001f;
        }   
        release = newRelease;
        applySoundADSRParams();
    }

    float getRelease() const { return release; }

    void setReverbMix(float mix) { 
        reverbMix = mix;
        reverbParams.wetLevel = mix;
        reverbParams.dryLevel = 1.0f - mix;
        reverb.setParameters(reverbParams);
    }

    float getReverbMix() const { return reverbMix; }

    void setDelayMix(float mix) {
        this->delayMix = mix;
    }

    float getDelayMix() const { return delayMix; }

    void setChorusMix(float mix) {
        chorusMix = mix;
        chorus.setMix(mix);
    }

    float getChorusMix() const { return chorusMix; }

    void setSaturationDrive(float drive) {
        saturationDrive = std::clamp(drive, 1.0f, 10.0f); // Clamp to [0.0, 1.0] 
    }

    float getDriveMix() const { return saturationDrive; }

    [[nodiscard]] juce::AudioProcessorValueTreeState& getState() noexcept { return state; }

private:
    juce::Synthesiser sampler;
    juce::AudioFormatManager formatManager;
    float volumeLevel = 0.0f;
    juce::SamplerSound *currentSound = nullptr;

    double m_sampleRate = 0.0;

    // ADSR values
    float attack = 0.0f;
    float decay = 0.0f;
    float sustain = 1.0f;
    float release = 0.0f;

    // Delay
    juce::dsp::DelayLine<float> delayLine { 44100 }; // 1 second delay at 44.1kHz sample rate
    float delayTime = 0.5f; // 500ms delay
    float feedback = 0.5f; // 50% feedback
    float delayMix = 0.0f; // 0% wet/dry mix

    // Reverb
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;
    float reverbMix = 0.0f;

    // Chorus
    juce::dsp::Chorus<float> chorus;
    float chorusMix = 0.0f;

    // Saturation
    float saturationDrive = 1.0f;

    // PARAMETER STATE ----
    // Define parameters
    struct Parameters {
        juce::AudioParameterFloat* attack{nullptr};
        juce::AudioParameterFloat* decay{nullptr};
        juce::AudioParameterFloat* sustain{nullptr};
        juce::AudioParameterFloat* release{nullptr};
        juce::AudioParameterFloat* reverbMix{nullptr};
        juce::AudioParameterFloat* delayMix{nullptr};
        juce::AudioParameterFloat* chorusMix{nullptr};
        juce::AudioParameterFloat* saturationDrive{nullptr};
        juce::AudioParameterChoice* presetIndex{nullptr};
    };
    Parameters parameters;
    juce::AudioProcessorValueTreeState state;
    [[nodiscard]] static juce::AudioProcessorValueTreeState::ParameterLayout 
    createParameterLayout(Parameters& parameters);
    
    void applySoundADSRParams();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};