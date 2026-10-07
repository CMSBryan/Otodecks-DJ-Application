//Chai Ming Shi Bryan
#include "../JuceLibraryCode/JuceHeader.h"
using namespace juce;
#include "WaveformDisplay.h"

WaveformDisplay::WaveformDisplay(AudioFormatManager & 	formatManagerToUse,
                                 AudioThumbnailCache & 	cacheToUse) :
                                 thumbnail(1000, formatManagerToUse, cacheToUse), 
                                 fileLoaded(false), 
                                 position(0)
                          
{

  thumbnail.addChangeListener(this);
  setInterceptsMouseClicks(true, true);
}

WaveformDisplay::~WaveformDisplay()
{
}

void WaveformDisplay::setRotationAngle(float angle)
{
    if (rotationAngle != angle)
    {
        rotationAngle = angle;
        repaint();
    }
}

void WaveformDisplay::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

    auto localBounds = getLocalBounds();
    auto center = localBounds.getCentre().toFloat();
    auto radius = static_cast<float>(juce::jmin(localBounds.getWidth(), localBounds.getHeight())) / 2.0f - 10.0f;

    // Apply rotation transform to spin the vinyl
    auto transform = juce::AffineTransform::rotation(rotationAngle, center.x, center.y);
    g.addTransform(transform);

    // --- All your existing vinyl drawing code goes here ---
    g.setColour(juce::Colour(0xff404040));
    g.drawEllipse(center.x - radius, center.y - radius, radius * 2, radius * 2, 3.0f);

    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillEllipse(center.x - radius + 5, center.y - radius + 5, (radius - 5) * 2, (radius - 5) * 2);

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

    // Remove transform before drawing non-spinning elements like text
    g.addTransform(transform.inverted());

    g.setColour(fileLoaded ? juce::Colour(0xff4CAF50) : juce::Colour(0xff666666));
    g.fillEllipse(center.x - 8, center.y - 8, 16, 16);

    if (fileLoaded && thumbnail.getTotalLength() > 0)
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
            thumbnail.getApproximateMinMax(
                samplePos * thumbnail.getTotalLength(),
                (samplePos + 0.008f) * thumbnail.getTotalLength(),
                0,
                minSample,
                maxSample
            );
            auto amplitude = std::abs(maxSample - minSample);
            auto waveRadius = juce::jmin(radius, radius - 35 + amplitude * 20);

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

        auto posAngle = static_cast<float>(position) * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
        auto indicatorX = center.x + (radius - 10) * std::cos(posAngle);
        auto indicatorY = center.y + (radius - 10) * std::sin(posAngle);

        g.setColour(juce::Colour(0xffFFEB3B));
        g.fillEllipse(indicatorX - 4, indicatorY - 4, 8, 8);
    }

    if (!fileLoaded)
    {
        g.setFont(20.0f);
        g.setColour(juce::Colours::white);
        g.drawText("File not loaded...", getLocalBounds(),
            juce::Justification::centred, true);
    }
}

void WaveformDisplay::resized()
{

}

void WaveformDisplay::loadURL(URL audioURL)
{
    thumbnail.setSource(new juce::FileInputSource(audioURL.getLocalFile()));
    fileLoaded = true;
    repaint();
}

void WaveformDisplay::changeListenerCallback (ChangeBroadcaster *source)
{
    std::cout << "wfd: change received! " << std::endl;

    repaint();

}

void WaveformDisplay::setPositionRelative(double pos)
{
  if (pos != position)
  {
    position = pos;
    repaint();
  }

  
}

double WaveformDisplay::calculateRelativePosFromMouse(juce::Point<int> mousePos)
{
    auto center = getLocalBounds().getCentre().toFloat();
    float dx = mousePos.x - center.x;
    float dy = mousePos.y - center.y;
    float angle = std::atan2(dy, dx) + juce::MathConstants<float>::halfPi;

    while (angle < 0.0f) angle += juce::MathConstants<float>::twoPi;
    while (angle >= juce::MathConstants<float>::twoPi) angle -= juce::MathConstants<float>::twoPi;

    return angle / juce::MathConstants<float>::twoPi;
}

void WaveformDisplay::mouseDown(const juce::MouseEvent& event)
{
    if (onSeekRequested)
        onSeekRequested(calculateRelativePosFromMouse(event.getPosition()));
}

void WaveformDisplay::mouseUp(const juce::MouseEvent& event)
{
    // Once the user lets go of the click, actually seek the audio track
    if (onSeekRequested)
        onSeekRequested(calculateRelativePosFromMouse(event.getPosition()));
}

void WaveformDisplay::mouseDrag(const juce::MouseEvent& event)
{
    mouseDown(event);
}



