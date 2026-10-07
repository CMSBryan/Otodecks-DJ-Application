//Chai Ming Shi Bryan
#include "SamplerGUI.h"

SamplerGUI::SamplerGUI(juce::AudioFormatManager& formatManagerToUse,
    juce::AudioDeviceManager& deviceManagerToUse)
    : formatManager(formatManagerToUse),
    audioDeviceManager(deviceManagerToUse)
{
    samplesFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("OtodecksSamples");
    if (!samplesFolder.exists())
    {
        samplesFolder.createDirectory();
    }

    addAndMakeVisible(recordButton);
    recordButton.addListener(this);
    recordButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffB71C1C)); // Red
    recordButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    addAndMakeVisible(playButton);
    playButton.addListener(this);
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4CAF50)); // Green
    playButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    addAndMakeVisible(deleteButton);
    deleteButton.addListener(this);
    deleteButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff303030));
    deleteButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    addAndMakeVisible(settingsButton);
    settingsButton.addListener(this);
    settingsButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff303030));
    settingsButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    addAndMakeVisible(sampleListBox);
    sampleListBox.setModel(this);

    // Register safely with the shared device manager for mic input
    audioDeviceManager.addAudioCallback(this);

    refreshSampleList();
}

SamplerGUI::~SamplerGUI()
{
    audioDeviceManager.removeAudioCallback(this);
}

void SamplerGUI::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1e1e1e));
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawText("Loop Sampler Panel", 20, 10, getWidth() - 40, 30, juce::Justification::left);
}

void SamplerGUI::resized()
{
    auto area = getLocalBounds().reduced(10);
    area.removeFromTop(35);

    auto topRow = area.removeFromTop(40);
    int btnWidth = topRow.getWidth() / 4 - 4;

    recordButton.setBounds(topRow.removeFromLeft(btnWidth).reduced(2));
    topRow.removeFromLeft(4);
    playButton.setBounds(topRow.removeFromLeft(btnWidth).reduced(2));
    topRow.removeFromLeft(4);
    deleteButton.setBounds(topRow.removeFromLeft(btnWidth).reduced(2));
    topRow.removeFromLeft(4);
    settingsButton.setBounds(topRow.reduced(2));

    area.removeFromTop(10);
    sampleListBox.setBounds(area);
}

void SamplerGUI::buttonClicked(juce::Button* button)
{
    if (button == &recordButton)
    {
        if (!isRecording)
        {
            auto timeString = juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S");
            auto fileToSave = samplesFolder.getChildFile("Sample_" + timeString + ".wav");

            fileToSave.deleteFile();
            fileStream = fileToSave.createOutputStream();
            if (fileStream != nullptr)
            {
                juce::WavAudioFormat wavFormat;
                writer.reset(wavFormat.createWriterFor(fileStream.release(), currentSampleRate, currentNumChannels, 16, {}, 0));

                if (writer != nullptr)
                {
                    isRecording = true;
                    recordButton.setButtonText("Stop Recording");
                    recordButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
                }
            }
        }
        else
        {
            {
                const juce::ScopedLock sl(writerLock);
                writer.reset();
                fileStream.reset();
            }

            isRecording = false;
            recordButton.setButtonText("Record Mic Sample");
            recordButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffB71C1C));

            refreshSampleList();
        }
    }
    else if (button == &playButton)
    {
        previewSelectedSample();
    }
    else if (button == &deleteButton)
    {
        deleteSelectedSample();
    }
    else if (button == &settingsButton)
    {
        auto settingsComp = std::make_unique<juce::AudioDeviceSelectorComponent>(
            audioDeviceManager,
            0, 2, 0, 2, true, true, true, false
        );
        settingsComp->setSize(500, 400);

        juce::DialogWindow::showDialog(
            "Audio Settings",
            settingsComp.release(),
            this,
            juce::Colours::darkgrey,
            true
        );
    }
}

int SamplerGUI::getNumRows() { return sampleFiles.size(); }

void SamplerGUI::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowNumber < sampleFiles.size())
    {
        if (rowIsSelected)
            g.fillAll(juce::Colour(0xff0277BD));
        else
            g.fillAll(juce::Colour(0xff2d2d2d));

        g.setColour(juce::Colours::white);
        g.setFont(14.0f);
        g.drawText(sampleFiles[rowNumber].getFileName(), 10, 0, width - 20, height, juce::Justification::centredLeft, true);
    }
}

void SamplerGUI::selectedRowsChanged(int lastRowSelected) { selectedRow = lastRowSelected; }

void SamplerGUI::refreshSampleList()
{
    sampleFiles.clear();
    samplesFolder.findChildFiles(sampleFiles, juce::File::findFiles, false, "*.wav");
    sampleListBox.updateContent();
}

void SamplerGUI::deleteSelectedSample()
{
    if (selectedRow >= 0 && selectedRow < sampleFiles.size())
    {
        sampleFiles[selectedRow].deleteFile();
        refreshSampleList();
        selectedRow = -1;
    }
}

void SamplerGUI::previewSelectedSample()
{
    if (selectedRow >= 0 && selectedRow < sampleFiles.size())
    {
        transportSource.stop();
        transportSource.setSource(nullptr);

        auto* reader = formatManager.createReaderFor(sampleFiles[selectedRow]);
        if (reader != nullptr)
        {
            readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);

            transportSource.prepareToPlay(currentBlockSize, currentSampleRate > 0 ? currentSampleRate : 44100.0);

            // Set readAheadSize to 0 so it reads synchronously without needing a TimeSliceThread
            transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);

            transportSource.start();
        }
    }
}

void SamplerGUI::audioDeviceIOCallbackWithContext(const float* const* inputChannelData, int numInputChannels,
    float* const* outputChannelData, int numOutputChannels,
    int numSamples, const juce::AudioIODeviceCallbackContext& context)
{
    // Clear output buffer to prevent the screeching memory garbage
    for (int i = 0; i < numOutputChannels; ++i) {
        if (outputChannelData[i] != nullptr)
            juce::FloatVectorOperations::clear(outputChannelData[i], numSamples);
    }

    // Continuously capture the live microphone input into our background buffer
    if (numInputChannels > 0 && inputChannelData != nullptr)
    {
        micBuffer.setSize(numInputChannels, numSamples, false, false, true);
        for (int i = 0; i < numInputChannels; ++i)
            micBuffer.copyFrom(i, 0, inputChannelData[i], numSamples);
    }
}
void SamplerGUI::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    currentSampleRate = device->getCurrentSampleRate();
    currentNumChannels = device->getActiveInputChannels().countNumberOfSetBits();
    if (currentNumChannels == 0) currentNumChannels = 2;
}

void SamplerGUI::audioDeviceStopped() {}

void SamplerGUI::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlockExpected; // Save the actual block size
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void SamplerGUI::releaseResources()
{
    transportSource.releaseResources();
}

void SamplerGUI::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    transportSource.getNextAudioBlock(bufferToFill);
}

void SamplerGUI::recordMasterMix(const juce::AudioSourceChannelInfo& deckBuffer)
{
    if (isRecording && writer != nullptr)
    {
        const juce::ScopedLock sl(writerLock);

        // Ensure our temporary mix buffer matches the deck output size
        mixBuffer.setSize(deckBuffer.buffer->getNumChannels(), deckBuffer.numSamples, false, false, true);
        mixBuffer.clear();

        // 1. Copy the playing Decks into the mix buffer
        for (int i = 0; i < deckBuffer.buffer->getNumChannels(); ++i) {
            mixBuffer.copyFrom(i, 0, *deckBuffer.buffer, i, deckBuffer.startSample, deckBuffer.numSamples);
        }

        // 2. Sum the live Microphone into the mix buffer (if mic is active)
        if (micBuffer.getNumChannels() > 0 && micBuffer.getNumSamples() >= deckBuffer.numSamples) {
            int channelsToMix = juce::jmin(micBuffer.getNumChannels(), mixBuffer.getNumChannels());
            for (int i = 0; i < channelsToMix; ++i) {
                mixBuffer.addFrom(i, 0, micBuffer, i, 0, deckBuffer.numSamples);
            }
        }

        // 3. Write the unified Master Mix to the WAV file
        writer->writeFromFloatArrays(mixBuffer.getArrayOfReadPointers(), mixBuffer.getNumChannels(), deckBuffer.numSamples);
    }
}