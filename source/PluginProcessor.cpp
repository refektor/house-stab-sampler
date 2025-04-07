#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "BinaryData.h"


//==============================================================================

namespace {
static auto streamToVector (juce::InputStream& stream)
{
    using namespace juce;
    std::vector<std::byte> result ((size_t) stream.getTotalLength());
    stream.setPosition (0);
    [[maybe_unused]] const auto bytesRead = stream.read (result.data(), result.size());
    jassert (bytesRead == (ssize_t) result.size());
    return result;
}

static const char* getMimeForExtension (const juce::String& extension)
{
    using namespace juce;

    static const std::unordered_map<String, const char*> mimeMap =
    {
        { { "htm"   },  "text/html"                },
        { { "html"  },  "text/html"                 },
        { { "txt"   },  "text/plain"               },
        { { "jpg"   },  "image/jpeg"               },
        { { "png"   },  "image/png"                },
        { { "jpeg"  },  "image/jpeg"               },
        { { "svg"   },  "image/svg+xml"            },
        { { "ico"   },  "image/vnd.microsoft.icon" },
        { { "json"  },  "application/json"         },
        { { "png"   },  "image/png"                },
        { { "css"   },  "text/css"                 },
        { { "map"   },  "application/json"         },
        { { "js"    },  "text/javascript"          },
        { { "woff2" },  "font/woff2"               }
    };

    if (const auto it = mimeMap.find (extension.toLowerCase()); it != mimeMap.end())
        return it->second;

    jassertfalse;
    return "";
}
} // namespace

PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // Add voices to the sampler
    for (int i = 0; i < 8; ++i)
        sampler.addVoice(new juce::SamplerVoice());


    webView = std::make_unique<juce::WebBrowserComponent>(juce::WebBrowserComponent::Options{}
        .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
    #ifdef _WIN32
        .withWinWebView2Options(juce::WebBrowserComponent::Options::WinWebView2{}.withUserDataFolder
        (juce::File::getSpecialLocation(juce::File::tempDirectory)))
    #endif
        .withResourceProvider([this](const juce::String& url) { return getResource(url); })
        .withNativeIntegrationEnabled()
        .withEventListener("presetSelectionChanged", [this](const juce::var& message) {
            setPreset(message["presetIndex"]);
        }).withEventListener("setAttack", [this](const juce::var& message) {
            setAttack(message["attack"]); 
        }).withEventListener("setDecay", [this](const juce::var& message) {
            setDecay(message["decay"]);
        }).withEventListener("setSustain", [this](const juce::var& message) {
            setSustain(message["sustain"]);
        }).withEventListener("setRelease", [this](const juce::var& message) {
            setRelease(message["release"]);
        }).withEventListener("setReverbMix", [this](const juce::var& message) {
            setReverbMix(message["mix"]);
        }).withEventListener("setDelayMix", [this](const juce::var& message) {
            setDelayMix(message["mix"]);
        }).withEventListener("setChorusMix", [this](const juce::var& message) {
            setChorusMix(message["mix"]);
        }).withEventListener("setSaturationDrive", [this](const juce::var& message) {
            setSaturationDrive(message["drive"]);
        }));
    
    webView->goToURL(webView->getResourceProviderRoot());

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
    
    setPreset(0);
}

PluginProcessor::~PluginProcessor()
{
    sampler.clearSounds();
    sampler.clearVoices();
    //delete currentSound;
    currentSound = nullptr;  // Safeguard against dangling pointer

    webView.reset();
    reverb.reset();
    delayLine.reset();
    chorus.reset();
}

//==============================================================================

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

auto PluginProcessor::getResource(const juce::String& url) -> std::optional<Resource>
{
    //const auto resourceToRetrieve = url == "/" ? "index.html" : url.fromFirstOccurrenceOf("/", false, false);
    const auto resourceToRetrieve = url == "/" ? "index.html" : url.fromLastOccurrenceOf("/", false, false);
    
    // Convert resourceToRetrieve to a valid C++ identifier
    juce::String resourceName = resourceToRetrieve.replaceCharacter('/', '_').replaceCharacter('.', '_').removeCharacters("-");
    
    // Get the resource data and size
    int resourceSize = 0;
    const char* resourceData = BinaryData::getNamedResource(resourceName.toRawUTF8(), resourceSize);

    if (resourceData != nullptr && resourceSize > 0)
    {
        const auto extension = resourceToRetrieve.fromLastOccurrenceOf(".", false, false);
        return Resource{std::vector<std::byte>(reinterpret_cast<const std::byte*>(resourceData), reinterpret_cast<const std::byte*>(resourceData) + resourceSize), getMimeForExtension(extension)};
    }

    return std::nullopt;
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

    // Initialize delay parameters
    delayLine.setDelay(delayTime * sampleRate);

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

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    sampler.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    // Apply saturation to the audio buffer using tanh
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] = std::tanh(saturationDrive * channelData[sample]);
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
    rmsLevel = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginProcessorEditor (*this);
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
