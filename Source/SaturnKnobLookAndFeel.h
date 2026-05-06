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
		SaturnKnobLookAndFeel();
		~SaturnKnobLookAndFeel() override;

		void	drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, \
			float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, \
			juce::Slider& slider) override;
		
		void	setDirection(int newDirection);
		void	setKnobText(const juce::String& newText);
	private:
		int				direction = 0; // 0: left to right, 1: right to left
		juce::String	knobText = "Test"; // text to diplay on the knob (if empty, no text will be displayed)

		void	drawLedOutline(juce::Graphics& g, float sliderPosProportional, float radius, juce::Point<float> centre);
		void	drawText(juce::Graphics& g, juce::Rectangle<float> bounds);
};