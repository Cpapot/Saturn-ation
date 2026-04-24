/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SaturnKnobLookAndFeel.h"


//==============================================================================
/**
*/
class SaturnationAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SaturnationAudioProcessorEditor (SaturnationAudioProcessor&);
    ~SaturnationAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SaturnationAudioProcessor&			audioProcessor;

	SaturnKnobLookAndFeel				knobLnf;
	juce::Slider						driveKnob, characterKnob, toneKnob, \
										lowCutKnob, highCutKnob, mixKnob;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> 
										driveAtt, characterAtt, toneAtt, \
										lowCutAtt, highCutAtt, mixAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SaturnationAudioProcessorEditor)
};
