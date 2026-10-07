//Chai Ming Shi Bryan
#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
using namespace juce;

class DJAudioPlayer : public juce::AudioSource {
  public:

    DJAudioPlayer(AudioFormatManager& _formatManager);
    ~DJAudioPlayer();

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void loadURL(URL audioURL);
    void setGain(double gain);
    void setSpeed(double ratio);
    void setPosition(double posInSecs);
    void setPositionRelative(double pos);

    /** Sets the gain of the low shelf EQ filter */
    void setLowGain(double gain);
    /** Sets the gain of the mid peak EQ filter */
    void setMidGain(double gain);
    /** Sets the gain of the high shelf EQ filter */
    void setHighGain(double gain);
    

    void start();
    void stop();

    /** get the relative position of the playhead */
    double getPositionRelative();

    double getCurrentPosition() const { return transportSource.getCurrentPosition(); }
    double getLengthInSeconds() const { return transportSource.getLengthInSeconds(); }

    // --- Hot Cue Methods ---
        /** Saves a playback position in seconds to the specified cue index (0-7) */
    void setHotCue(int cueIndex, double positionInSeconds);
    /** Instantly moves the transport source playhead to the saved cue timestamp */
    void jumpToHotCue(int cueIndex);
    /** Clears the saved timestamp at the specified cue index */
    void clearHotCue(int cueIndex);
    /** Resets all 8 hot cue slots to an empty state (-1.0) */
    void clearAllHotCues();
    /** Retrieves the saved timestamp for a specific cue index */
    double getHotCue(int cueIndex) const;
    /** Persists the current hot cue array to an XML settings file for the loaded track */
    void saveHotCuesForTrack(const juce::File& file);

private:
    AudioFormatManager& formatManager;
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    AudioTransportSource transportSource; 
    ResamplingAudioSource resampleSource{&transportSource, false, 2};
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lowFilter, midFilter, highFilter;
    double currentSampleRate = 44100.0;
    juce::File currentLoadedFile;
    void loadHotCuesForTrack(const juce::File& file);
};




