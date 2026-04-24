#pragma once
/*
  ==============================================================================

	This file overrides the default LookAndFeel to create a custom appearance for the knobs in the plugin.

  ==============================================================================
*/

#include <JuceHeader.h>

class SaturnKnobLookAndFeel : public juce::LookAndFeel_V4
{
	public:
		void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, \
			float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, \
			juce::Slider& slider) override;
};