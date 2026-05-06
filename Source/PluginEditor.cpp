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
    : AudioProcessorEditor (&p), audioProcessor (p)
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
