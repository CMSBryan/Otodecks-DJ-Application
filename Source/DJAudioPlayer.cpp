//Chai Ming Shi Bryan
#include "DJAudioPlayer.h"

DJAudioPlayer::DJAudioPlayer(AudioFormatManager& _formatManager) 
: formatManager(_formatManager)
{

}
DJAudioPlayer::~DJAudioPlayer()
{

}

void DJAudioPlayer::prepareToPlay (int samplesPerBlockExpected, double sampleRate) 
{
    // 1. Prevent processing if the sample rate is invalid
    if (sampleRate <= 0.0) return;

    currentSampleRate = sampleRate;

    // prepare transport and resampling layers
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);

    // 2. Tell the DSP filters about the audio stream properties
    juce::dsp::ProcessSpec spec{ (double)sampleRate, (juce::uint32)samplesPerBlockExpected, 2 };
    spec.maximumBlockSize = samplesPerBlockExpected;
    spec.sampleRate = sampleRate;
    spec.numChannels = 2; // Stereo audio

    lowFilter.prepare(spec);
    midFilter.prepare(spec);
    highFilter.prepare(spec);

    // Set default flat filter coefficients (Low-pass, Band-pass/All-pass behavior or simple shelving)
    *lowFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 250.0f, 0.7071f, 1.0f);
    *midFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 1000.0f, 0.7071f, 1.0f);
    *highFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 4000.0f, 0.7071f, 1.0f);
}
void DJAudioPlayer::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    resampleSource.getNextAudioBlock(bufferToFill);

    juce::dsp::AudioBlock<float> block(*bufferToFill.buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    lowFilter.process(context);
    midFilter.process(context);
    highFilter.process(context);

}

void DJAudioPlayer::setLowGain(double gain)
{
    *lowFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(currentSampleRate, 300.0f, 0.7f, static_cast<float>(gain));
}

void DJAudioPlayer::setMidGain(double gain)
{
    float safeGain = static_cast<float>(juce::jmax(0.05, gain));
    *midFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate, 1000.0f, 1.5f, safeGain);
}

void DJAudioPlayer::setHighGain(double gain)
{
    *highFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate, 4000.0f, 0.7f, static_cast<float>(gain));
}

void DJAudioPlayer::releaseResources()
{
    transportSource.releaseResources();
    resampleSource.releaseResources();
}

std::array<double, 8> hotCues = { -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0 };
juce::File currentLoadedFile;

void DJAudioPlayer::loadURL(const juce::URL audioURL)
{
    auto* reader = formatManager.createReaderFor(audioURL.createInputStream(false));
    if (reader != nullptr) // good file!
    {
        std::unique_ptr<AudioFormatReaderSource> newSource(new AudioFormatReaderSource(reader, true));
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        readerSource.reset(newSource.release());

        // Load persisted hot cues for this track
        if (audioURL.isLocalFile())
        {
            currentLoadedFile = audioURL.getLocalFile();
            loadHotCuesForTrack(currentLoadedFile);
        }
    }
}
void DJAudioPlayer::setGain(double gain)
{
    if (gain < 0 || gain > 1.0)
    {
        std::cout << "DJAudioPlayer::setGain gain should be between 0 and 1" << std::endl;
    }
    else {
        transportSource.setGain(gain);
    }
   
}
void DJAudioPlayer::setSpeed(double ratio)
{
  if (ratio < 0 || ratio > 100.0)
    {
        std::cout << "DJAudioPlayer::setSpeed ratio should be between 0 and 100" << std::endl;
    }
    else {
        resampleSource.setResamplingRatio(ratio);
    }
}
void DJAudioPlayer::setPosition(double posInSecs)
{
    transportSource.setPosition(posInSecs);
}

void DJAudioPlayer::setPositionRelative(double pos)
{
     if (pos < 0 || pos > 1.0)
    {
        std::cout << "DJAudioPlayer::setPositionRelative pos should be between 0 and 1" << std::endl;
    }
    else {
        double posInSecs = transportSource.getLengthInSeconds() * pos;
        setPosition(posInSecs);
    }
}


void DJAudioPlayer::start()
{
    transportSource.start();
}
void DJAudioPlayer::stop()
{
  transportSource.stop();
}

double DJAudioPlayer::getPositionRelative()
{
    double length = transportSource.getLengthInSeconds();

    // Prevent divide-by-zero crashes if the file isn't fully loaded
    if (length <= 0.0)
    {
        return 0.0;
    }
    return transportSource.getCurrentPosition() / transportSource.getLengthInSeconds();
}

void DJAudioPlayer::setHotCue(int cueIndex, double positionInSeconds)
{
    if (cueIndex >= 0 && cueIndex < 8)
    {
        hotCues[cueIndex] = positionInSeconds;
        saveHotCuesForTrack(currentLoadedFile);
    }
}

void DJAudioPlayer::jumpToHotCue(int cueIndex)
{
    if (cueIndex >= 0 && cueIndex < 8 && hotCues[cueIndex] >= 0.0)
    {
        transportSource.setPosition(hotCues[cueIndex]);
    }
}

void DJAudioPlayer::clearHotCue(int cueIndex)
{
    if (cueIndex >= 0 && cueIndex < 8)
    {
        hotCues[cueIndex] = -1.0;
        saveHotCuesForTrack(currentLoadedFile);
    }
}

void DJAudioPlayer::clearAllHotCues()
{
    hotCues.fill(-1.0);
    saveHotCuesForTrack(currentLoadedFile);
}

double DJAudioPlayer::getHotCue(int cueIndex) const
{
    if (cueIndex >= 0 && cueIndex < 8)
        return hotCues[cueIndex];
    return -1.0;
}

void DJAudioPlayer::saveHotCuesForTrack(const juce::File& file)
{
    if (!file.existsAsFile()) return;

    // Save to an application properties file or a sidecar JSON/XML file
    juce::PropertiesFile::Options options;
    options.applicationName = "Otodecks";
    options.filenameSuffix = ".settings";
    options.folderName = "Otodecks_App";

    juce::PropertiesFile props(options);

    juce::StringArray cueStrings;
    for (double cue : hotCues)
        cueStrings.add(juce::String(cue));

    props.setValue("cues_" + file.getFullPathName(), cueStrings.joinIntoString(","));
    props.saveIfNeeded();
}

void DJAudioPlayer::loadHotCuesForTrack(const juce::File& file)
{
    hotCues.fill(-1.0);
    if (!file.existsAsFile()) return;

    juce::PropertiesFile::Options options;
    options.applicationName = "Otodecks";
    options.filenameSuffix = ".settings";
    options.folderName = "Otodecks_App";

    juce::PropertiesFile props(options);
    juce::String cueStr = props.getValue("cues_" + file.getFullPathName(), "");

    if (cueStr.isNotEmpty())
    {
        juce::StringArray parsed = juce::StringArray::fromTokens(cueStr, ",", "");
        for (int i = 0; i < juce::jmin(8, parsed.size()); ++i)
        {
            hotCues[i] = parsed[i].getDoubleValue();
        }
    }
}
