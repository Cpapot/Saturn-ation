/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SaturnKnobLookAndFeel.h"
#include "SaturnVuMeter.h"


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

    SaturnKnobLookAndFeel				driveKnobLnf, characterKnobLnf, toneKnobLnf,
                    lowCutKnobLnf, highCutKnobLnf, mixKnobLnf;
	  juce::Slider						driveKnob, characterKnob, toneKnob, \
										lowCutKnob, highCutKnob, mixKnob;
    SaturnVuMeter					driveMeter;
	  juce::ScopedPointer<juce::AudioProcessorValueTreeState::SliderAttachment> 
										driveAtt, characterAtt, toneAtt, \
										lowCutAtt, highCutAtt, mixAtt;

    void setupSlider(juce::Slider& slider, juce::LookAndFeel& lnf);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SaturnationAudioProcessorEditor)
};
