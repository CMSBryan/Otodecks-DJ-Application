//Chai Ming Shi Bryan
#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
using namespace juce;
#include "DJAudioPlayer.h"
#include "DeckGUI.h"
#include "MusicLibrary.h"
#include "SamplerGUI.h"


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public juce::AudioAppComponent,
                        public juce::ChangeListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

    void changeListenerCallback (juce::ChangeBroadcaster* source) override;


private:
    //==============================================================================
    // Your private member variables go here...
     
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache{5}; 

    DJAudioPlayer player1{ formatManager };
    DJAudioPlayer player2{ formatManager };

    juce::MixerAudioSource mixerSource;

    DeckGUI deckGUI1{ &player1, formatManager, thumbnailCache };
    DeckGUI deckGUI2{ &player2, formatManager, thumbnailCache };
    MusicLibrary musicLibrary;

    std::unique_ptr<SamplerGUI> samplerGUI;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
