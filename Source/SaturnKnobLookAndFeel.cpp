#include "SaturnKnobLookAndFeel.h"

void SaturnKnobLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, \
			float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, \
			juce::Slider& slider)
{
	   using namespace juce;

        auto bounds = Rectangle<float>((float)x, (float)y, (float)width, (float)height).reduced(4.0f);
        auto radius = jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        auto centre = bounds.getCentre();

        const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Fond knob
        {
            ColourGradient bg(Colour(0xff4b4b4b), centre.x, centre.y - radius,
                              Colour(0xff1e1e1e), centre.x, centre.y + radius, false);
            g.setGradientFill(bg);
            g.fillEllipse(bounds);
        }

        // Anneau métal
        {
            auto ring = bounds.reduced(radius * 0.12f);
            g.setColour(Colour(0xffb7b7b7).withAlpha(0.85f));
            g.drawEllipse(ring, 2.0f);
            g.setColour(Colour(0xff000000).withAlpha(0.45f));
            g.drawEllipse(ring.reduced(2.0f), 1.2f);
        }

        // Arc de valeur (cyan)
        {
            auto arcBounds = bounds.reduced(radius * 0.08f);
            Path valueArc;
            constexpr float arcThickness = 6.0f;

            valueArc.addCentredArc(centre.x, centre.y,
                                   arcBounds.getWidth() * 0.5f,
                                   arcBounds.getHeight() * 0.5f,
                                   0.0f,
                                   rotaryStartAngle,
                                   angle,
                                   true);

            g.setColour(Colour(0xff55f2e6));
            g.strokePath(valueArc, PathStrokeType(arcThickness, PathStrokeType::curved, PathStrokeType::rounded));

            // Arc "reste" gris
            Path restArc;
            restArc.addCentredArc(centre.x, centre.y,
                                  arcBounds.getWidth() * 0.5f,
                                  arcBounds.getHeight() * 0.5f,
                                  0.0f,
                                  angle,
                                  rotaryEndAngle,
                                  true);
            g.setColour(Colour(0xff7a7a7a).withAlpha(0.55f));
            g.strokePath(restArc, PathStrokeType(arcThickness, PathStrokeType::curved, PathStrokeType::rounded));
        }

        // Centre doré
        {
            auto cap = bounds.reduced(radius * 0.33f);
            ColourGradient gold(Colour(0xfff1d083), cap.getCentreX(), cap.getY(),
                                Colour(0xff9d6f1e), cap.getCentreX(), cap.getBottom(), false);
            g.setGradientFill(gold);
            g.fillEllipse(cap);

            g.setColour(Colour(0xffffffff).withAlpha(0.20f));
            g.drawEllipse(cap.reduced(1.0f), 1.0f);
        }

        // Pointeur
        {
            auto pointerLen = radius * 0.42f;
            auto pointerThickness = 3.0f;

            Path p;
            p.addRoundedRectangle(-pointerThickness * 0.5f, -pointerLen, pointerThickness, pointerLen, 1.2f);

            g.setColour(Colour(0xffd9c08b));
            g.fillPath(p, AffineTransform::rotation(angle).translated(centre.x, centre.y));
        }
}