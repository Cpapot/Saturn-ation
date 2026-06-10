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
		return juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);
    }
}

void SaturnationAudioProcessorEditor::setupSlider(juce::Slider& slider, juce::LookAndFeel& lnf)
{
	slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
	slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
	slider.setLookAndFeel(&lnf);
	slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f,
		juce::MathConstants<float>::pi * 2.8f,
		true);
	addAndMakeVisible(slider);
}

//==============================================================================
SaturnationAudioProcessorEditor::SaturnationAudioProcessorEditor (SaturnationAudioProcessor& p)
	: AudioProcessorEditor (&p), audioProcessor (p), driveMeter (audioProcessor)
{
    setSize (600, 300);
	driveKnobLnf.setKnobText("Drive");
	characterKnobLnf.setKnobText("Type");
	toneKnobLnf.setKnobText("Tone");
	lowCutKnobLnf.setKnobText("LowCut");
	highCutKnobLnf.setKnobText("HighCut");
	mixKnobLnf.setKnobText("Mix");
	highCutKnobLnf.setDirection(1);

	setupSlider(driveKnob, driveKnobLnf);
	setupSlider(characterKnob, characterKnobLnf);
	setupSlider(toneKnob, toneKnobLnf);
	setupSlider(lowCutKnob, lowCutKnobLnf);
	setupSlider(highCutKnob, highCutKnobLnf);
	setupSlider(mixKnob, mixKnobLnf);
	addAndMakeVisible (driveMeter);

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
	// Detach APVTS bindings first so no UI callbacks can hit partially destroyed controls.
	driveAtt = nullptr;
	characterAtt = nullptr;
	toneAtt = nullptr;
	lowCutAtt = nullptr;
	highCutAtt = nullptr;
	mixAtt = nullptr;

	// Then clear custom look-and-feel pointers from all controls.
	driveKnob.setLookAndFeel(nullptr);
	characterKnob.setLookAndFeel(nullptr);
	toneKnob.setLookAndFeel(nullptr);
	lowCutKnob.setLookAndFeel(nullptr);
	highCutKnob.setLookAndFeel(nullptr);
	mixKnob.setLookAndFeel(nullptr);
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
	auto area = getLocalBounds().reduced(24);

	// Reserve right area for the VU meter (around 22% width) so the image can keep its ratio and use the full height
	const int meterWidth = std::max(60, (int) std::round (area.getWidth() * 0.22f));
	auto meterArea = area.removeFromRight (meterWidth);
	// Make the meter take the full height inside its area
	driveMeter.setBounds (meterArea.reduced (1));

	// 3 knobs en haut, 3 en bas in remaining area
	auto top = area.removeFromTop(area.getHeight() / 2);
	auto bottom = area;

	const int knobSize = 120;

	// Slightly tighten knob layout so buttons are closer
	auto driveZone = top.removeFromLeft(std::max(1, top.getWidth() / 3));
	// place drive knob in its zone
	driveKnob.setBounds    (driveZone.withSizeKeepingCentre(knobSize, knobSize));
	characterKnob.setBounds(top.removeFromLeft(top.getWidth() / 2).withSizeKeepingCentre(knobSize, knobSize));
	toneKnob.setBounds     (top.withSizeKeepingCentre(knobSize, knobSize));

	lowCutKnob.setBounds   (bottom.removeFromLeft(bottom.getWidth() / 3).withSizeKeepingCentre(knobSize, knobSize));
	highCutKnob.setBounds  (bottom.removeFromLeft(bottom.getWidth() / 2).withSizeKeepingCentre(knobSize, knobSize));
	mixKnob.setBounds      (bottom.withSizeKeepingCentre(knobSize, knobSize));
}
