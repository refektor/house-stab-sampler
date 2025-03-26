#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

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

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processor (p),
    reverbSliderRelay{ "reverbMix" },
    delaySliderRelay{ "delayMix" },
    chorusSliderRelay{ "chorusMix" },
    driveSliderRelay{ "saturationDrive" },
    attackSliderRelay{ "attack" },
    decaySliderRelay{ "decay" },
    sustainSliderRelay{ "sustain" },
    releaseSliderRelay{ "release" },
    presetRelay{ "presetIndex" },
    webView {
        juce::WebBrowserComponent::Options{}
            .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
        #ifdef _WIN32
            .withWinWebView2Options(juce::WebBrowserComponent::Options::WinWebView2{}.withUserDataFolder
            (juce::File::getSpecialLocation(juce::File::tempDirectory)))
        #endif
            .withResourceProvider([this](const juce::String& url) { return getResource(url); })
            .withNativeIntegrationEnabled()
            .withEventListener("presetSelectionChanged", [this](const juce::var& message) {
                // Update parameter value when combo box changes from UI
                // auto* param = processor.getState().getParameter("presetIndex");
                // if (param != nullptr)
                //     param->setValueNotifyingHost(param->convertTo0to1(message["presetIndex"]));
            }).withEventListener("setAttack", [this](const juce::var& message) {
                processor.setAttack(message["attack"]);
            }).withEventListener("setDecay", [this](const juce::var& message) {
                processor.setDecay(message["decay"]);
            }).withEventListener("setSustain", [this](const juce::var& message) {
                processor.setSustain(message["sustain"]);
            }).withEventListener("setRelease", [this](const juce::var& message) {
                processor.setRelease(message["release"]);
            }).withEventListener("setReverbMix", [this](const juce::var& message) {
                processor.setReverbMix(message["mix"]);
            }).withEventListener("setDelayMix", [this](const juce::var& message) {
                processor.setDelayMix(message["mix"]);
            }).withEventListener("setChorusMix", [this](const juce::var& message) {
                processor.setChorusMix(message["mix"]);
            }).withEventListener("setSaturationDrive", [this](const juce::var& message) {
                processor.setSaturationDrive(message["drive"]);
            })
            .withOptionsFrom(reverbSliderRelay)
            .withOptionsFrom(delaySliderRelay)
            .withOptionsFrom(chorusSliderRelay)
            .withOptionsFrom(driveSliderRelay)
            .withOptionsFrom(attackSliderRelay)
            .withOptionsFrom(decaySliderRelay)
            .withOptionsFrom(sustainSliderRelay)
            .withOptionsFrom(releaseSliderRelay)
            .withOptionsFrom(presetRelay)
    },
    reverbSliderAttachment{*processor.getState().getParameter("reverbMix"), reverbSliderRelay},
    delaySliderAttachment{*processor.getState().getParameter("delayMix"), delaySliderRelay},
    chorusSliderAttachment{*processor.getState().getParameter("chorusMix"), chorusSliderRelay},
    driveSliderAttachment{*processor.getState().getParameter("saturationDrive"), driveSliderRelay},
    attackSliderAttachment{*processor.getState().getParameter("attack"), attackSliderRelay},
    decaySliderAttachment{*processor.getState().getParameter("decay"), decaySliderRelay},
    sustainSliderAttachment{*processor.getState().getParameter("sustain"), sustainSliderRelay},
    releaseSliderAttachment{*processor.getState().getParameter("release"), releaseSliderRelay},
    presetAttachment{*processor.getState().getParameter("presetIndex"), presetRelay}
{
    juce::ignoreUnused (processor);

    addAndMakeVisible(webView);
    webView.goToURL(webView.getResourceProviderRoot());

    setResizable(false, false);
    setSize (700, 700); // Set the size of your plugin window
    startTimer(50);
}

PluginEditor::~PluginEditor()
{
    stopTimer();
    webView.setVisible(false); // Ensure it's hidden when the UI is closed
}

void PluginEditor::timerCallback()
{
    // Update VU meter needle
    auto volumeLevel = processor.getMagnitude();
    juce::String jsCode = "updateNeedle(" + juce::String(volumeLevel) + ");\n";

    // Update mix parameters for automation from DAW
    // jsCode += "setReverbKnob(" + juce::String(processor.getReverbMix()) + ");\n";
    // jsCode += "setDelayKnob(" + juce::String(processor.getDelayMix()) + ");\n";
    // jsCode += "setChorusKnob(" + juce::String(processor.getChorusMix()) + ");\n";
    // jsCode += "setDriveKnob(" + juce::String(processor.getDriveMix()) + ");\n";
    // jsCode += "setAttackKnob(" + juce::String(processor.getAttack()) + ");\n";
    // jsCode += "setDecayKnob(" + juce::String(processor.getDecay()) + ");\n";
    // jsCode += "setSustainKnob(" + juce::String(processor.getSustain()) + ");\n";
    // jsCode += "setReleaseKnob(" + juce::String(processor.getRelease()) + ");\n";
    webView.evaluateJavascript(jsCode);
}

auto PluginEditor::getResource(const juce::String& url) -> std::optional<Resource>
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

//==============================================================================


void PluginEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    //webView.setBounds(getLocalBounds());
    webView.setBounds(getLocalBounds());
}