//Chai Ming Shi Bryan
#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    formatManager.registerBasicFormats();
    deviceManager.addChangeListener(this);

    addAndMakeVisible(deckGUI1);
    addAndMakeVisible(deckGUI2);
    addAndMakeVisible(musicLibrary);

    samplerGUI = std::make_unique<SamplerGUI>(formatManager, deviceManager);
    addAndMakeVisible(*samplerGUI);

    musicLibrary.setDeckPointers(&deckGUI1, &deckGUI2);

    // ROUTE AUDIO ONCE HERE: Do not do this in prepareToPlay!
    mixerSource.addInputSource(&player1, false);
    mixerSource.addInputSource(&player2, false);
    mixerSource.addInputSource(samplerGUI.get(), false);

    setSize(1200, 900);

    // Initialize audio cleanly
    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
        && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
            [this](bool granted) { setAudioChannels(granted ? 2 : 0, 2); });
    }
    else
    {
        setAudioChannels(2, 2);
    }

    // Re-register the sampler callback AFTER setAudioChannels resets the audio engine
    if (samplerGUI != nullptr)
    {
        deviceManager.addAudioCallback(samplerGUI.get());
    }
}

MainComponent::~MainComponent()
{
    
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
    mixerSource.removeAllInputs();
    samplerGUI.reset();
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &deviceManager)
    {
        // When headphones disconnect/reconnect, refresh the audio device state
        // This forces JUCE to look for the currently available active output device
        auto savedAudioSetup = deviceManager.getAudioDeviceSetup();
            deviceManager.initialise(0, 2, nullptr, true, {}, &savedAudioSetup);
    }
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    player1.prepareToPlay(samplesPerBlockExpected, sampleRate);
    player2.prepareToPlay(samplesPerBlockExpected, sampleRate);

    if (samplerGUI != nullptr)
        samplerGUI->prepareToPlay(samplesPerBlockExpected, sampleRate);

    mixerSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    mixerSource.getNextAudioBlock(bufferToFill);
    // Pass the generated deck audio to the sampler to be mixed with the mic and recorded
    if (samplerGUI != nullptr)
    {
        samplerGUI->recordMasterMix(bufferToFill);
    }

}

void MainComponent::releaseResources()
{
    player1.releaseResources();
    player2.releaseResources();

    if (samplerGUI != nullptr)
        samplerGUI->releaseResources();

    mixerSource.releaseResources();
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    int deckHeight = getHeight() * 0.60;

    deckGUI1.setBounds(0, 0, getWidth() / 2, deckHeight);
    deckGUI2.setBounds(getWidth() / 2, 0, getWidth() / 2, deckHeight);

    auto bottomArea = getLocalBounds();
    bottomArea.removeFromTop(deckHeight);

    int halfWidth = getWidth() / 2;
    musicLibrary.setBounds(bottomArea.removeFromLeft(halfWidth));

    if (samplerGUI != nullptr)
        samplerGUI->setBounds(bottomArea);
}
