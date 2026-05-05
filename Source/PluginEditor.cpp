/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

namespace
{
    juce::Image getBackgroundImage()
    {
        static const juce::Image backgroundImage = []
        {
            return juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);
        }();

        return backgroundImage;
    }
}

//==============================================================================
SaturnationAudioProcessorEditor::SaturnationAudioProcessorEditor (SaturnationAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (600, 300);
	auto setupKnob = [this](juce::Slider& s)
	{
		s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
		s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
		s.setLookAndFeel(&knobLnf);
		s.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f,  // start ~216°
							juce::MathConstants<float>::pi * 2.8f,  // end   ~504°
							true);
		addAndMakeVisible(s);
	};

	setupKnob(driveKnob);
	setupKnob(characterKnob);
	setupKnob(toneKnob);
	setupKnob(lowCutKnob);
	setupKnob(highCutKnob);
	setupKnob(mixKnob);

	auto& apvts = audioProcessor.getAPVTS();
	driveAtt		= new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, "driveAmount",         driveKnob);
	characterAtt	= new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, "saturationMode",      characterKnob);
	toneAtt			= new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, "toneAmount",          toneKnob);
	lowCutAtt		= new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, "lowCutoffFrequency",  lowCutKnob);
	highCutAtt		= new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, "highCutoffFrequency", highCutKnob);
	mixAtt			= new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, "mixAmount",           mixKnob);
}

SaturnationAudioProcessorEditor::~SaturnationAudioProcessorEditor()
{
	// Reset look and feel before destruction
	driveKnob.setLookAndFeel(nullptr);
	characterKnob.setLookAndFeel(nullptr);
	toneKnob.setLookAndFeel(nullptr);
	lowCutKnob.setLookAndFeel(nullptr);
	highCutKnob.setLookAndFeel(nullptr);
	mixKnob.setLookAndFeel(nullptr);
	
	// ScopedPointer will automatically delete attachments in proper order
}

//==============================================================================
void SaturnationAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto backgroundImage = getBackgroundImage();
    if (backgroundImage.isValid())
    {
        g.drawImageWithin(backgroundImage,
                          0, 0,
                          getWidth(), getHeight(),
                          juce::RectanglePlacement::stretchToFit,
                          false);
    }
    else
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    }
}

void SaturnationAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
	auto area = getLocalBounds().reduced(24);

	// 3 knobs en haut, 3 en bas
	auto top = area.removeFromTop(area.getHeight() / 2);
	auto bottom = area;

	const int knobSize = 120;
	driveKnob.setBounds    (top.removeFromLeft(top.getWidth() / 3).withSizeKeepingCentre(knobSize, knobSize));
	characterKnob.setBounds(top.removeFromLeft(top.getWidth() / 2).withSizeKeepingCentre(knobSize, knobSize));
	toneKnob.setBounds     (top.withSizeKeepingCentre(knobSize, knobSize));

	lowCutKnob.setBounds   (bottom.removeFromLeft(bottom.getWidth() / 3).withSizeKeepingCentre(knobSize, knobSize));
	highCutKnob.setBounds  (bottom.removeFromLeft(bottom.getWidth() / 2).withSizeKeepingCentre(knobSize, knobSize));
	mixKnob.setBounds      (bottom.withSizeKeepingCentre(knobSize, knobSize));
}
