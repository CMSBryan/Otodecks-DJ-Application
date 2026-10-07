//Chai Ming Shi Bryan
#pragma once

#include <JuceHeader.h>

class SamplerGUI : public juce::Component,
    public juce::Button::Listener,
    public juce::ListBoxModel,
    public juce::AudioIODeviceCallback,
    public juce::AudioSource
{
public:
    SamplerGUI(juce::AudioFormatManager& formatManagerToUse,
        juce::AudioDeviceManager& deviceManagerToUse);
    ~SamplerGUI(); // Removed override to prevent compiler quirks

    void paint(juce::Graphics&) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;

    // ListBoxModel overrides
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void selectedRowsChanged(int lastRowSelected) override;

    // AudioIODeviceCallback overrides for recording microphone input
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData, int numInputChannels,
        float* const* outputChannelData, int numOutputChannels,
        int numSamples, const juce::AudioIODeviceCallbackContext& context) override;
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

    // AudioSource overrides for preview playback mixing
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    /**
         * Intercepts the master audio buffer from the decks, mixes it with the live microphone
         * input background buffer, and writes the unified output to a WAV file.
         * @param deckBuffer The AudioSourceChannelInfo currently being processed by the master mixer
         */
    void recordMasterMix(const juce::AudioSourceChannelInfo& deckBuffer);

private:
    juce::AudioFormatManager& formatManager;
    juce::AudioDeviceManager& audioDeviceManager;

    juce::TextButton recordButton{ "Record Mic Sample" };
    juce::TextButton playButton{ "Preview Sample" };
    juce::TextButton deleteButton{ "Delete Sample" };
    juce::TextButton settingsButton{ "Audio Settings" };

    juce::ListBox sampleListBox{ "Samples List", this };

    juce::File samplesFolder;
    juce::Array<juce::File> sampleFiles;
    int selectedRow = -1;

    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

    std::atomic<bool> isRecording{ false };
    std::unique_ptr<juce::FileOutputStream> fileStream;
    std::unique_ptr<juce::AudioFormatWriter> writer;
    double currentSampleRate = 44100.0;
	int currentBlockSize = 512;
    int currentNumChannels = 2;
    juce::CriticalSection writerLock;

    void refreshSampleList();
    void deleteSelectedSample();
    void previewSelectedSample();

    juce::AudioBuffer<float> micBuffer;
    juce::AudioBuffer<float> mixBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplerGUI)
};
