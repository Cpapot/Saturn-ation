#include "SaturnKnobLookAndFeel.h"
#include "BinaryData.h"

namespace
{
    juce::Image getKnobImage()
    {
        static const juce::Image knobImage = []
        {
            return juce::ImageCache::getFromMemory(BinaryData::knob_512x512_png, BinaryData::knob_512x512_pngSize);
        }();

        return knobImage;
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
        }
        else
        {
            // Fallback visuel si la ressource embarquée est absente.
            ColourGradient bg(Colour(0xff4b4b4b), centre.x, centre.y - radius,
                              Colour(0xff1e1e1e), centre.x, centre.y + radius, false);
            g.setGradientFill(bg);
            g.fillEllipse(bounds);
        }
}