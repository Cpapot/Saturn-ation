#include "SaturnVuMeter.h"
#include "BinaryData.h"

SaturnVuMeter::SaturnVuMeter (SaturnationAudioProcessor& processorToUse)
    : processor (processorToUse)
{
    startTimerHz (60);

    auto baseImg = juce::ImageCache::getFromMemory (BinaryData::vuMeter_png, BinaryData::vuMeter_pngSize);
    if (baseImg.isValid())
    {
        baseImage = baseImg;
        hasBaseImage = true;
    }

    auto colorImg = juce::ImageCache::getFromMemory (BinaryData::vuMeterColor_png, BinaryData::vuMeterColor_pngSize);
    if (colorImg.isValid())
    {
        colorImage = colorImg;
        hasColorImage = true;
    }
}

SaturnVuMeter::~SaturnVuMeter()
{
    stopTimer();
}

void SaturnVuMeter::setMeterBackgroundColour (juce::Colour newColour)
{
    backgroundColour = newColour;
    repaint();
}

void SaturnVuMeter::setMeterFillColour (juce::Colour newColour)
{
    fillColour = newColour;
    repaint();
}

void SaturnVuMeter::setMeterPeakColour (juce::Colour newColour)
{
    peakColour = newColour;
    repaint();
}

void SaturnVuMeter::setMeterBorderColour (juce::Colour newColour)
{
    borderColour = newColour;
    repaint();
}

void SaturnVuMeter::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
	float proportion = juce::jmap (displayedLevelDb, minDb, maxDb, 0.0f, 1.0f);

    auto makeScaledImageRect = [&bounds] (const juce::Image& image)
    {
        if (! image.isValid())
            return bounds;

        const auto imageAspect = (float) image.getWidth() / (float) image.getHeight();
        const auto drawHeight = bounds.getHeight();
        const auto drawWidth = drawHeight * imageAspect;

        // Keep the aspect ratio, but let the image extend sideways if needed.
        return juce::Rectangle<float> (bounds.getCentreX() - drawWidth * 0.5f, bounds.getY(), drawWidth, drawHeight);
    };

    // If we have the base contour image, draw it first
    if (hasBaseImage)
    {
        const auto baseRect = makeScaledImageRect (baseImage);

        // Draw the base contour at its natural aspect ratio
        g.drawImage (baseImage,
                     baseRect.getX(), baseRect.getY(), baseRect.getWidth(), baseRect.getHeight(),
                     0, 0, baseImage.getWidth(), baseImage.getHeight(),
                     false);

        // If we have the colored fill image, draw only its bottom portion corresponding to the level
        if (hasColorImage)
        {
            const auto colorRect = makeScaledImageRect (colorImage);
            const auto filledHeight = colorRect.getHeight() * juce::jlimit (0.0f, 1.0f, proportion);
            if (filledHeight > 1.0f)
            {
                juce::Rectangle<float> fillArea = colorRect.withTrimmedTop (colorRect.getHeight() - filledHeight);

                g.saveState();
                g.reduceClipRegion (fillArea.toNearestInt());
                // Draw the color image at the same scale as the base contour
                g.drawImage (colorImage,
                             colorRect.getX(), colorRect.getY(), colorRect.getWidth(), colorRect.getHeight(),
                             0, 0, colorImage.getWidth(), colorImage.getHeight(),
                             false);
                g.restoreState();
            }
        }
    }
    else
    {
        // Draw background frame
        g.setColour (backgroundColour);
        g.fillRoundedRectangle (bounds, 3.0f);

        g.setColour (borderColour);
        g.drawRoundedRectangle (bounds, 3.0f, 1.0f);

        // Compute fill area (bottom-up)
        const auto fillHeight = bounds.getHeight() * juce::jlimit (0.0f, 1.0f, proportion);
        auto fillArea = bounds.removeFromBottom (fillHeight);

        if (! fillArea.isEmpty())
        {
            // Fallback: simple gradient fill
            g.setGradientFill (juce::ColourGradient (
                fillColour.brighter (0.15f), fillArea.getCentreX(), fillArea.getBottom(),
                fillColour.darker (0.25f),    fillArea.getCentreX(), fillArea.getY(),
                false));
            g.fillRoundedRectangle (fillArea, 3.0f);
        }
    }

    // peak marker removed (visual handled by color image)
}

void SaturnVuMeter::timerCallback()
{
    const float target = juce::jlimit (0.0f, 1.0f, processor.getDriveMeterLevel());

    displayedLevel = juce::jmax (target, displayedLevel * 0.86f);
    peakHoldLevel  = juce::jmax (target, peakHoldLevel * 0.94f);

	displayedLevelDb = juce::Decibels::gainToDecibels (displayedLevel, -100.0f);
    peakHoldLevelDb  = juce::Decibels::gainToDecibels (peakHoldLevel, -100.0f);

    repaint();
}
