#include "SaturnKnobLookAndFeel.h"
#include "BinaryData.h"

namespace
{
    juce::Image getKnobImage()
    {
        return juce::ImageCache::getFromMemory(BinaryData::knob_512x512_png, BinaryData::knob_512x512_pngSize);
    }
}

SaturnKnobLookAndFeel::SaturnKnobLookAndFeel() = default;
SaturnKnobLookAndFeel::~SaturnKnobLookAndFeel() = default;

void SaturnKnobLookAndFeel::drawLedOutline(juce::Graphics& g, float sliderPosProportional, float radius, juce::Point<float> centre)
{
    const int totalLEDs = 20;
    const float startDeg = 120.0f;
    const float arcDeg = 300.0f; 
    const float ledOffset = 1.0f;
    const float ledSize = 3.0f;
    const juce::Colour ledOn = juce::Colour(0xffc6ff5c);
    const juce::Colour ledOff = juce::Colour(0x33222222);


    const float startRad = juce::degreesToRadians(startDeg);
    const float arcRad   = juce::degreesToRadians(arcDeg);

    const int lit = juce::jlimit (0, totalLEDs, (int)std::round (sliderPosProportional * (float)totalLEDs));

    const float ledRadius = radius + ledOffset + ledSize * 0.5f;
    for (int i = 0; i < totalLEDs; ++i)
    {
        const float t = (totalLEDs == 1) ? 0.0f : (float)i / (float)(totalLEDs - 1);
        const float theta = startRad + t * arcRad;

        const float cx = centre.x + std::cos(theta) * ledRadius;
        const float cy = centre.y + std::sin(theta) * ledRadius;

        g.setColour(i < lit ? ledOn : ledOff);
        g.fillEllipse(cx - ledSize * 0.5f, cy - ledSize * 0.5f, ledSize, ledSize);
    }
}


void SaturnKnobLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, \
			float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, \
			juce::Slider& slider)
{
	   using namespace juce;

        (void)slider;

        auto bounds = Rectangle<float>((float)x, (float)y, (float)width, (float)height).reduced(4.0f);
        auto radius = jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        auto centre = bounds.getCentre();

        const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        auto knobImage = getKnobImage();
        if (knobImage.isValid() && knobImage.getWidth() > 0 && knobImage.getHeight() > 0)
        {
            const auto imageWidth = (float)knobImage.getWidth();
            const auto imageHeight = (float)knobImage.getHeight();
            const auto scale = jmin(bounds.getWidth() / imageWidth, bounds.getHeight() / imageHeight);
            const auto halfWidth = imageWidth * 0.5f;
            const auto halfHeight = imageHeight * 0.5f;

            const auto cosAngle = std::cos(angle);
            const auto sinAngle = std::sin(angle);

            const AffineTransform transform(
                scale * cosAngle, -scale * sinAngle,
                centre.x - scale * (cosAngle * halfWidth - sinAngle * halfHeight),
                scale * sinAngle,  scale * cosAngle,
                centre.y - scale * (sinAngle * halfWidth + cosAngle * halfHeight));

            g.drawImageTransformed(knobImage, transform);
            drawLedOutline(g, sliderPosProportional, radius, centre);
        }
        else
        {
            // Fallback visuel si la ressource embarquée est absente.
            ColourGradient bg(Colour(0xff4b4b4b), centre.x, centre.y - radius,
                              Colour(0xff1e1e1e), centre.x, centre.y + radius, false);
            g.setGradientFill(bg);
            g.fillEllipse(bounds);
            drawLedOutline(g, sliderPosProportional, radius, centre);
        }
}