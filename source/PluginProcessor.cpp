#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "BinaryData.h"


//==============================================================================

PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
    state{*this, nullptr, "PARAMETERS", createParameterLayout(parameters)}
{
    // Add voices to the sampler
    for (int i = 0; i < 8; ++i)
        sampler.addVoice(new juce::SamplerVoice());
    

    // Initialize reverb parameters
    reverbParams.roomSize = 0.5f;
    reverbParams.damping = 0.5f;
    reverbParams.wetLevel = 0.0f;
    reverbParams.dryLevel = 1.0f;
    reverbParams.width = 1.0f;
    reverbParams.freezeMode = 0.0f;
    reverb.setParameters(reverbParams);

    // Initialize chorus parameters
    chorus.setRate(1.0f); // Rate in Hz
    chorus.setDepth(0.5f); // Depth (0 to 1)
    chorus.setCentreDelay(7.0f); // Centre delay in ms
    chorus.setFeedback(0.5f); // Feedback (0 to 1)
    chorus.setMix(0.0f); // Mix (0 to 1)
    
    setPreset(1);
}

PluginProcessor::~PluginProcessor()
{
    sampler.clearSounds();
    sampler.clearVoices();
    //delete currentSound;
    currentSound = nullptr;  // Safeguard against dangling pointer

    reverb.reset();
    delayLine.reset();
    chorus.reset();
}

//==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout(Parameters& parameters) {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Add parameters to the layout
    {
        auto parameter = std::make_unique<juce::AudioParameterFloat>("attack", "Attack", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f);
        parameters.attack = parameter.get();
        layout.add(std::move(parameter));
    }

    {
        auto parameter = std::make_unique<juce::AudioParameterFloat>("decay", "Decay", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f);
        parameters.decay = parameter.get();
        layout.add(std::move(parameter));
    }

    {
        auto parameter = std::make_unique<juce::AudioParameterFloat>("sustain", "Sustain", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f);
        parameters.sustain = parameter.get();
        layout.add(std::move(parameter));
    }

    {
        auto parameter = std::make_unique<juce::AudioParameterFloat>("release", "Release", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f);
        parameters.release = parameter.get();
        layout.add(std::move(parameter));
    }

    {
        auto parameter = std::make_unique<juce::AudioParameterFloat>("reverbMix", "Reverb Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f);
        parameters.reverbMix = parameter.get();
        layout.add(std::move(parameter));
    }

    {
        auto parameter = std::make_unique<juce::AudioParameterFloat>("delayMix", "Delay Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f);
        parameters.delayMix = parameter.get();
        layout.add(std::move(parameter));
    }

    {
        auto parameter = std::make_unique<juce::AudioParameterFloat>("chorusMix", "Chorus Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f);
        parameters.chorusMix = parameter.get();
        layout.add(std::move(parameter));
    }

    {
        auto parameter = std::make_unique<juce::AudioParameterFloat>("saturationDrive", "Drive", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f);
        parameters.saturationDrive = parameter.get();
        layout.add(std::move(parameter));
    }
    {
         // Create a StringArray of preset names from BinaryData
         juce::StringArray presetNames;
         for (int i = 0; i < BinaryData::namedResourceListSize; ++i) {
             juce::String name = BinaryData::namedResourceList[i];
             
             // Replace underscores with spaces
             name = name.replace("_", " ");
             
             // Only add .wave files (presets) and remove ".wav" extension if present
             if (name.endsWithIgnoreCase("wav")) {
                name = name.dropLastCharacters(3);
             
                // Trim any whitespace from the end
                name = name.trim();
                
                presetNames.add(name);
             }
                 
         }
        
        auto parameter = std::make_unique<juce::AudioParameterChoice>("presetIndex", "Preset Index", presetNames, 0);
        parameters.presetIndex = parameter.get();
        layout.add(std::move(parameter));
    }

    return layout;
}

void PluginProcessor::setPreset(int index)
{
    juce::ignoreUnused(index);

    if (sampler.getNumSounds() > 0)
    {
        sampler.clearSounds();
    }

    if (index >= BinaryData::namedResourceListSize) {
        return; // Invalid index
    }

    // Get the name and size of the binary data file
    const char* name = BinaryData::namedResourceList[index];
    int dataSize = 0;
    const void* data = BinaryData::getNamedResource(name, dataSize);
    
    // Create an input stream for the embedded audio file
    std::unique_ptr<juce::InputStream> inputStream(
        new juce::MemoryInputStream(data, dataSize, false));
   
    // Create an AudioFormatReader from the input stream
    formatManager.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(std::move(inputStream)));

    if (reader != nullptr)
    {
        // Create an AudioBuffer to hold the audio data
        juce::AudioBuffer<float> audioBuffer;
        audioBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
        reader->read(&audioBuffer, 0, (int)reader->lengthInSamples, 0, true, true);


        juce::BigInteger range;
        range.setRange(0, 127, true);

        // Create and add a SamplerSound
        currentSound = new juce::SamplerSound(
            "Sample", 
            *reader, 
            range,
            60,     // MIDI root note
            attack,    // Attack time
            release,    // Release time
            10.0    // Maximum length in seconds
        );
        applySoundADSRParams();
        sampler.addSound(currentSound);
    }
}

void PluginProcessor::applySoundADSRParams()
{
    juce::ADSR::Parameters adsrParams;
    adsrParams.attack = attack;
    adsrParams.decay = decay;
    adsrParams.sustain = sustain;
    adsrParams.release = release;
    
    currentSound->setEnvelopeParameters(adsrParams);
}

const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
   return true;
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);
    sampler.setCurrentPlaybackSampleRate(sampleRate);

    // set sample rate to be used for delay time in prcoessBlock function
    m_sampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    reverb.prepare(spec);
    delayLine.prepare(spec);
    chorus.prepare(spec);
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Get automation parameters
    // setDelayMix(state.getRawParameterValue("delayMix")->load());
    // setSaturationDrive(state.getRawParameterValue("driveMix")->load());
    // setChorusMix(state.getRawParameterValue("chorusMix")->load());
    // setReverbMix(state.getRawParameterValue("reverbMix")->load());
    // setAttack(state.getRawParameterValue("attack")->load());
    // setDecay(state.getRawParameterValue("decay")->load());
    // setSustain(state.getRawParameterValue("sustain")->load());
    // setRelease(state.getRawParameterValue("release")->load());

    setDelayMix(parameters.delayMix->get());
    setSaturationDrive(parameters.saturationDrive->get());
    setChorusMix(parameters.chorusMix->get());
    setReverbMix(parameters.reverbMix->get());
    setAttack(parameters.attack->get());
    setDecay(parameters.decay->get());
    setSustain(parameters.sustain->get());
    setRelease(parameters.release->get());
    
    setPreset(parameters.presetIndex->getIndex());

    // Set delay time based on BPM. We have to do it here because this is the only place where we can get the BPM from the host
    double bpm = 120.0; // Default in case host doesn't provide BPM
    if (auto* playHead = getPlayHead())
    {
        if (auto position = playHead->getPosition())
        {
            if (position->getBpm().hasValue())
                bpm = *position->getBpm();
        }
    }
    double delayMs = (60000.0 / (bpm * 2)); // 1/8th note
    delayTime = (delayMs * m_sampleRate) / 1000.0;
    delayLine.setDelay(delayTime);

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    sampler.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    // Calculate input RMS
    float inputRMS = 0.0f;
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* channelData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            inputRMS += channelData[sample] * channelData[sample];
        }
    }
    inputRMS = std::sqrt(inputRMS / (totalNumOutputChannels * buffer.getNumSamples()));

    // Apply saturation to the audio buffer using tanh
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] = std::tanh(saturationDrive * channelData[sample]);
        }
    }

    // Calculate output RMS
    float outputRMS = 0.0f;
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* channelData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            outputRMS += channelData[sample] * channelData[sample];
        }
    }
    outputRMS = std::sqrt(outputRMS / (totalNumOutputChannels * buffer.getNumSamples()));

    // Apply auto gain to match input RMS
    if (outputRMS > 0.0f)
    {
        float gain = inputRMS / outputRMS;
        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                channelData[sample] *= gain;
            }
        }
    }
    
    // Apply chorus to the audio buffer
    juce::dsp::AudioBlock<float> chorusBlock (buffer);
    juce::dsp::ProcessContextReplacing chorusCtx (chorusBlock);
    chorus.process(chorusCtx);

    // Apply delay to the audio buffer
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            auto delayedSample = delayLine.popSample(channel);
            delayLine.pushSample(channel, channelData[sample] + feedback * delayedSample);
            channelData[sample] = channelData[sample] * (1.0f - delayMix) + delayedSample * delayMix;
        }
    }

    // Apply reverb to the audio buffer
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing ctx (block);
    reverb.process (ctx);

    // Calculate RMS level for the first channel
    volumeLevel = buffer.getMagnitude(0, buffer.getNumSamples());
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
