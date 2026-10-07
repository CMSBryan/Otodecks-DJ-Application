//Chai Ming Shi Bryan
#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
using namespace juce;

class WaveformDisplay    : public Component, 
                           public ChangeListener
{
public:
    WaveformDisplay(juce::AudioFormatManager & 	formatManagerToUse,
                    juce::AudioThumbnailCache & 	cacheToUse );
    ~WaveformDisplay() override;

    void paint (Graphics&) override;
    void resized() override;

    void changeListenerCallback (ChangeBroadcaster *source) override;

    void loadURL(juce::URL audioURL);

    /** set the relative position of the playhead*/
    void setPositionRelative(double pos);
    void setRotationAngle(float angle);

    void mouseDown(const juce:: MouseEvent & event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    // Callback function to notify DeckGUI when user clicks to seek
    std::function<void(double)> onSeekRequested;

    double getTotalLength() const { return thumbnail.getTotalLength(); }

    void getApproximateMinMax(double startSec, double endSec, int channel, float& minSample, float& maxSample) const
    {
        thumbnail.getApproximateMinMax(startSec, endSec, channel, minSample, maxSample);
    }

private:
    double calculateRelativePosFromMouse(juce::Point<int> mousePos);
    juce::AudioThumbnail thumbnail;
    bool fileLoaded = false; 
    double position = 0;
    float rotationAngle = 0.0f;

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};
