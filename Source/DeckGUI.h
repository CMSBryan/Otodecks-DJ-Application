//Chai Ming Shi Bryan
#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
using namespace juce;
#include "DJAudioPlayer.h"
#include "WaveformDisplay.h"

class DeckGUI : public juce::Component,
    public juce::Button::Listener,
    public juce::Slider::Listener,
    public juce::Timer,
    public juce::FileDragAndDropTarget // Retained for drag-and-drop support
{
public:
    DeckGUI(DJAudioPlayer* player, juce::AudioFormatManager& formatManager, juce::AudioThumbnailCache& cacheToUse);
    ~DeckGUI();

    void paint(juce::Graphics& g) override;
    void resized() override;

    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void timerCallback() override;

    // FileDragAndDropTarget overrides
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    void loadFile(juce::URL audioURL);
    void setPositionRelative(double pos);

private:
    DJAudioPlayer* djAudioPlayer;
    WaveformDisplay waveformDisplay;

    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::TextButton loadButton;
    juce::TextButton loopButton;

    juce::Slider volSlider;
    juce::Slider speedSlider;
    juce::Slider posSlider;

    juce::Label volLabel;
    juce::Label speedLabel;
    juce::Label posLabel;
    juce::Label fileLabel;

    juce::Slider lowSlider;
    juce::Slider midSlider;
    juce::Slider highSlider;

    juce::Label lowLabel;
    juce::Label midLabel;
    juce::Label highLabel;

    float rotationAngle;
    bool isPlaying;
    std::unique_ptr<juce::FileChooser> fileChooser;

    void drawCircularWaveform(juce::Graphics& g, juce::Rectangle<int> bounds);

    std::array<juce::TextButton, 8> cueButtons;
    juce::TextButton clearCuesButton;

	void updateCueButtonColors();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeckGUI)
};