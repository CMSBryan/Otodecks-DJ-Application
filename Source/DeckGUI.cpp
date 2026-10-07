//Chai Ming Shi Bryan
#include "DeckGUI.h"
#include <cmath>

DeckGUI::DeckGUI(DJAudioPlayer* _player,
    juce::AudioFormatManager& formatManager,
    juce::AudioThumbnailCache& cacheToUse)
    : djAudioPlayer(_player),
    waveformDisplay(formatManager, cacheToUse),
    rotationAngle(0.0f),
    isPlaying(false)
{
    addAndMakeVisible(playButton);
    playButton.setButtonText("PLAY");
    playButton.addListener(this);
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4CAF50));
    playButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    addAndMakeVisible(stopButton);
    stopButton.setButtonText("STOP");
    stopButton.addListener(this);
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffF44336));
    stopButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    addAndMakeVisible(loadButton);
    loadButton.setButtonText("LOAD");
    loadButton.addListener(this);
    loadButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2196F3));
    loadButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    addAndMakeVisible(loopButton);
    loopButton.setButtonText("LOOP");
    loopButton.setClickingTogglesState(true); // Makes the button stay pressed in
    loopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff404040));
    loopButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffFF9800)); // Turns orange when active

    addAndMakeVisible(volSlider);
    volSlider.setRange(0.0, 1.0);
    volSlider.setValue(0.8);
    volSlider.setNumDecimalPlacesToDisplay(1);
    volSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    volSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    volSlider.addListener(this);
    volSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff42a2c8));

    addAndMakeVisible(speedSlider);
    speedSlider.setRange(0.5, 2.0);
    speedSlider.setValue(1.0);
    speedSlider.setNumDecimalPlacesToDisplay(1);
    speedSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    speedSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    speedSlider.addListener(this);
    speedSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffFF9800));

    addAndMakeVisible(posSlider);
    posSlider.setRange(0.0, 1.0);
    posSlider.setValue(0.0);
    posSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    posSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    posSlider.addListener(this);
    posSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff404040));
    posSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff42a2c8));

    addAndMakeVisible(volLabel);
    volLabel.setText("Volume", juce::dontSendNotification);
    volLabel.setJustificationType(juce::Justification::centred);
    volLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);

    addAndMakeVisible(speedLabel);
    speedLabel.setText("Speed", juce::dontSendNotification);
    speedLabel.setJustificationType(juce::Justification::centred);
    speedLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);

    addAndMakeVisible(posLabel);
    posLabel.setText("Position", juce::dontSendNotification);
    posLabel.setJustificationType(juce::Justification::centred);
    posLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);

    addAndMakeVisible(fileLabel);
    fileLabel.setText("No file loaded", juce::dontSendNotification);
    fileLabel.setJustificationType(juce::Justification::centred);
    fileLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    startTimer(16);

    addAndMakeVisible(waveformDisplay);
    waveformDisplay.onSeekRequested = [this](double pos) {
        if (djAudioPlayer != nullptr)
        {
            djAudioPlayer->setPositionRelative(pos);
            posSlider.setValue(pos, juce::dontSendNotification);
        }
        };

    for (auto* slider : { &lowSlider, &midSlider, &highSlider })
    {
        addAndMakeVisible(*slider);
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        slider->setNumDecimalPlacesToDisplay(1);
        slider->addListener(this);
    }
    lowSlider.setRange(0.1, 3.0);  lowSlider.setValue(1.0);
    midSlider.setRange(0.1, 3.0);  midSlider.setValue(1.0);
    highSlider.setRange(0.1, 3.0); highSlider.setValue(1.0);

    // --- EQ Labels Setup ---
    addAndMakeVisible(lowLabel);   lowLabel.setText("Low", juce::dontSendNotification);   lowLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(midLabel);   midLabel.setText("Mid", juce::dontSendNotification);   midLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(highLabel);  highLabel.setText("High", juce::dontSendNotification);  highLabel.setJustificationType(juce::Justification::centred);

    for (int i = 0; i < 8; ++i)
    {
        addAndMakeVisible(cueButtons[i]);
        cueButtons[i].setButtonText("Cue " + juce::String(i + 1));
        cueButtons[i].addListener(this);
        cueButtons[i].setColour(juce::TextButton::buttonColourId, juce::Colour(0xff303030));
        cueButtons[i].setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    }

    addAndMakeVisible(clearCuesButton);
    clearCuesButton.setButtonText("Clear All Cues");
    clearCuesButton.addListener(this);
    clearCuesButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffB71C1C));
    clearCuesButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
}

juce::String formatTime(double seconds)
{
    int mins = static_cast<int>(seconds) / 60;
    int secs = static_cast<int>(seconds) % 60;
    return juce::String::formatted("%02d:%02d", mins, secs);
}

DeckGUI::~DeckGUI()
{

    stopTimer();
}

void DeckGUI::paint(juce::Graphics& g)
{
    juce::ColourGradient gradient(juce::Colour(0xff1a1a1a), 0, 0,
        juce::Colour(0xff2d2d30), 0, static_cast<float>(getHeight()), false);
    g.setGradientFill(gradient);
    g.fillAll();

    g.setColour(juce::Colour(0xff505050));
    g.drawRect(getLocalBounds(), 2);

    auto centerArea = getLocalBounds().removeFromTop(200).reduced(20);
    drawCircularWaveform(g, centerArea);
}

void DeckGUI::resized()

{

    auto area = getLocalBounds();



    // 1. Carve from top to bottom sequentially so nothing can ever overlap

    fileLabel.setBounds(area.removeFromTop(22));

    int waveformHeight = juce::jlimit(140, 220, area.getHeight() / 3);
    waveformDisplay.setBounds(area.removeFromTop(waveformHeight));

    auto buttonArea = area.removeFromTop(45);

    auto buttonWidth = buttonArea.getWidth() / 4;

    loadButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(4));

    playButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(4));

    stopButton.setBounds(buttonArea.removeFromRight(buttonWidth).reduced(4));

    loopButton.setBounds(buttonArea.reduced(4));



    posLabel.setBounds(area.removeFromTop(18));

    posSlider.setBounds(area.removeFromTop(25).reduced(4));



    // --- Row 1: Volume & Speed ---

    auto row1 = area.removeFromTop(120);

    auto halfWidth = row1.getWidth() / 2;



    auto volCol = row1.removeFromLeft(halfWidth).reduced(6, 2);

    auto speedCol = row1.reduced(6, 2);



    volLabel.setBounds(volCol.removeFromTop(15));

    volSlider.setBounds(volCol);



    speedLabel.setBounds(speedCol.removeFromTop(15));

    speedSlider.setBounds(speedCol);



    // --- Row 2: Low, Mid, High EQ ---

    auto row2 = area.removeFromTop(100);

    auto thirdWidth = row2.getWidth() / 3;



    auto lowCol = row2.removeFromLeft(thirdWidth).reduced(4, 2);

    auto midCol = row2.removeFromLeft(thirdWidth).reduced(4, 2);

    auto highCol = row2.reduced(4, 2);



    lowLabel.setBounds(lowCol.removeFromTop(15));

    lowSlider.setBounds(lowCol);



    midLabel.setBounds(midCol.removeFromTop(15));

    midSlider.setBounds(midCol);



    highLabel.setBounds(highCol.removeFromTop(15));

    highSlider.setBounds(highCol);

    // --- Hot Cue Buttons Row ---
    auto cueArea = area.removeFromTop(30);
    auto cueWidth = cueArea.getWidth() / 9; // 8 cue buttons + 1 clear button

    for (int i = 0; i < 8; ++i)
    {
        cueButtons[i].setBounds(cueArea.removeFromLeft(cueWidth).reduced(2));
    }
    clearCuesButton.setBounds(cueArea.reduced(2));

}

void DeckGUI::buttonClicked(juce::Button* button)
{
    if (button == &playButton)
    {
        if (isPlaying)
        {
            // Pause playback
            djAudioPlayer->stop();
            isPlaying = false;
            playButton.setButtonText("PLAY");
            playButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4CAF50)); // Green
        }
        else
        {
            // Start or resume playback
            if (djAudioPlayer->getPositionRelative() >= 0.99)
            {
                djAudioPlayer->setPositionRelative(0.0);
            }
            djAudioPlayer->start();
            isPlaying = true;
            playButton.setButtonText("PAUSE");
            playButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffFF9800)); // Orange/Yellow for pause state
        }
    }
    else if (button == &stopButton)
    {
        // True hard stop and reset
        djAudioPlayer->stop();
        djAudioPlayer->setPositionRelative(0.0);
        posSlider.setValue(0.0, juce::dontSendNotification);
        isPlaying = false;
        rotationAngle = 0.0f;
        playButton.setButtonText("PLAY");
        playButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4CAF50));
    }
    else if (button == &loadButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select files...",
            juce::File::getSpecialLocation(juce::File::userHomeDirectory),
            "*.wav;*.mp3;*.aif;*.flac");

        fileChooser->launchAsync(juce::FileBrowserComponent::openMode
            | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto results = fc.getResults();
                if (results.isEmpty())
                    return;
                loadFile(juce::URL{ results.getFirst() });
            });
    }
    else if (button == &clearCuesButton)
    {
        if (djAudioPlayer != nullptr)
        {
            djAudioPlayer->clearAllHotCues();
            updateCueButtonColors(); // Updates buttons to grey immediately
        }
    }
    else
    {
        for (int i = 0; i < 8; ++i)
        {
            if (button == &cueButtons[i])
            {
                if (djAudioPlayer != nullptr)
                {
                    double currentPos = djAudioPlayer->getCurrentPosition();
                    double existingCue = djAudioPlayer->getHotCue(i);

                    if (existingCue < 0.0 || juce::ModifierKeys::getCurrentModifiers().isShiftDown())
                    {
                        djAudioPlayer->setHotCue(i, currentPos);
                    }
                    else
                    {
                        djAudioPlayer->jumpToHotCue(i);
                    }

                    updateCueButtonColors(); // Updates button colors after setting/jumping
                }
            }
        }
    }
}

void DeckGUI::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volSlider)
    {
        djAudioPlayer->setGain(volSlider.getValue());
    }
    else if (slider == &speedSlider)
    {
        djAudioPlayer->setSpeed(speedSlider.getValue());
    }
    else if (slider == &posSlider)
    {
        djAudioPlayer->setPositionRelative(posSlider.getValue());
    }
    else if (slider == &lowSlider)
    {
        djAudioPlayer->setLowGain(lowSlider.getValue());
    }
    else if (slider == &midSlider)
    {
        djAudioPlayer->setMidGain(midSlider.getValue());
    }
    else if (slider == &highSlider)
    {
        djAudioPlayer->setHighGain(highSlider.getValue());
    }
}

void DeckGUI::timerCallback()
{
    if (djAudioPlayer != nullptr)
    {
        double currentPos = djAudioPlayer->getCurrentPosition();
        double totalLength = djAudioPlayer->getLengthInSeconds();

        posSlider.setValue(djAudioPlayer->getPositionRelative(), juce::dontSendNotification);

        // Format and display current time vs total length
        posLabel.setText(formatTime(currentPos) + " / " + formatTime(totalLength), juce::dontSendNotification);

        waveformDisplay.setPositionRelative(djAudioPlayer->getPositionRelative());
        waveformDisplay.setRotationAngle(rotationAngle);
    }
}

bool DeckGUI::isInterestedInFileDrag(const juce::StringArray& files)
{
    return true;
}

void DeckGUI::filesDropped(const juce::StringArray& files, int x, int y)
{
    if (files.size() == 1)
    {
        loadFile(juce::URL{ juce::File{files[0]} });
    }
}

void DeckGUI::loadFile(juce::URL audioURL)
{
    djAudioPlayer->stop(); // Ensure old audio stops playing

    djAudioPlayer->loadURL(audioURL);
	updateCueButtonColors(); // Update cue button colors based on the new track
    djAudioPlayer->setPositionRelative(0.0); // Reset internal playhead
    posSlider.setValue(0.0, juce::dontSendNotification); // Reset visual slider

    waveformDisplay.loadURL(audioURL);
    auto file = audioURL.getLocalFile();
    fileLabel.setText(file.getFileNameWithoutExtension(), juce::dontSendNotification);

    // Reset play/pause button state on new track load
    isPlaying = false;
    playButton.setButtonText("PLAY");
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4CAF50)); // Green
}

void DeckGUI::setPositionRelative(double pos)
{
    if (djAudioPlayer != nullptr)
    {
        djAudioPlayer->setPositionRelative(pos);
    }
}

void DeckGUI::drawCircularWaveform(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto center = bounds.getCentre().toFloat();
    auto radius = static_cast<float>(juce::jmin(bounds.getWidth(), bounds.getHeight())) / 2.0f - 10.0f;

    g.setColour(juce::Colour(0xff404040));
    g.drawEllipse(center.x - radius, center.y - radius, radius * 2, radius * 2, 3.0f);

    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillEllipse(center.x - radius + 5, center.y - radius + 5, (radius - 5) * 2, (radius - 5) * 2);

    auto transform = juce::AffineTransform::rotation(rotationAngle, center.x, center.y);
    g.addTransform(transform);

    g.setColour(juce::Colour(0xff303030));
    for (int i = 0; i < 12; ++i)
    {
        auto angle = static_cast<float>(i) * juce::MathConstants<float>::twoPi / 12.0f;
        auto startX = center.x + (radius - 40) * std::cos(angle);
        auto startY = center.y + (radius - 40) * std::sin(angle);
        auto endX = center.x + (radius - 15) * std::cos(angle);
        auto endY = center.y + (radius - 15) * std::sin(angle);
        g.drawLine(startX, startY, endX, endY, 1.5f);
    }

    g.addTransform(transform.inverted());

    g.setColour(isPlaying ? juce::Colour(0xff4CAF50) : juce::Colour(0xff666666));
    g.fillEllipse(center.x - 8, center.y - 8, 16, 16);

    if (waveformDisplay.getTotalLength() > 0)
    {
        g.setColour(juce::Colour(0xff42a2c8));
        auto numSamples = 120;
        auto angleStep = juce::MathConstants<float>::twoPi / static_cast<float>(numSamples);

        juce::Path waveformPath;
        bool firstPoint = true;

        for (int i = 0; i < numSamples; ++i)
        {
            auto angle = static_cast<float>(i) * angleStep;
            auto samplePos = static_cast<double>(i) / static_cast<double>(numSamples);

            float minSample = 0.0f;
            float maxSample = 0.0f;
            waveformDisplay.getApproximateMinMax(
                samplePos * waveformDisplay.getTotalLength(),
                (samplePos + 0.008f) * waveformDisplay.getTotalLength(),
                0,
                minSample,
                maxSample
            );
            auto amplitude = std::abs(maxSample - minSample);
            auto waveRadius = radius - 35 + amplitude * 20;

            auto x = center.x + waveRadius * std::cos(angle - juce::MathConstants<float>::halfPi);
            auto y = center.y + waveRadius * std::sin(angle - juce::MathConstants<float>::halfPi);

            if (firstPoint)
            {
                waveformPath.startNewSubPath(x, y);
                firstPoint = false;
            }
            else
            {
                waveformPath.lineTo(x, y);
            }
        }

        waveformPath.closeSubPath();
        g.strokePath(waveformPath, juce::PathStrokeType(2.0f));

        auto currentPos = djAudioPlayer->getPositionRelative();
        auto posAngle = static_cast<float>(currentPos) * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
        auto indicatorX = center.x + (radius - 10) * std::cos(posAngle);
        auto indicatorY = center.y + (radius - 10) * std::sin(posAngle);

        g.setColour(juce::Colour(0xffFFEB3B));
        g.fillEllipse(indicatorX - 4, indicatorY - 4, 8, 8);
    }
}

void DeckGUI::updateCueButtonColors()
{
    if (djAudioPlayer == nullptr) return;

    for (int i = 0; i < 8; ++i)
    {
        if (djAudioPlayer->getHotCue(i) >= 0.0)
        {
            // Highlight orange if an active cue timestamp exists
            cueButtons[i].setColour(juce::TextButton::buttonColourId, juce::Colour(0xffFF8F00));
        }
        else
        {
            // Reset to default dark grey if unassigned
            cueButtons[i].setColour(juce::TextButton::buttonColourId, juce::Colour(0xff303030));
        }
    }
}