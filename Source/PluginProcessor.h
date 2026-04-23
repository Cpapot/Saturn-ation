/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum class SaturationMode {
    HardClip = 0,
    Tube = 1,
    Tape = 2
};

//==============================================================================
/**
*/
class SaturnationAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SaturnationAudioProcessor();
    ~SaturnationAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SaturnationAudioProcessor)


    //=========================== Saturation Parameters ============================
    SaturationMode					saturationMode = SaturationMode::HardClip;	// Saturation mode
	float							driveAmount = 5.0f;							// Augment signal before clipping (0.1 to 10.0)
	float							applySaturation(float sample);

	//=========================== Tone Control Parameters ============================
	float							toneAmount = 0.0f;							// Tone control dark to bright (-1.0 to 1.0)
  	std::array<juce::IIRFilter, 2>	toneLowpass;								// Per-channel low-pass for tilt tone control
	float							applyToneControl(float sample, int channel);
	
	//=========================== Cutoff Control Parameters ============================
	float							lowCutoffFrequency = 0.0f;					// Cutoff frequency for the low-pass filter (0Hz to 1kHz)
	float							highCutoffFrequency = 20000.0f;				// Cutoff frequency for the high-pass filter(20kHz to 1kHz)
	std::array<juce::IIRFilter, 2>	lowCutFilters;   							// high-pass
	std::array<juce::IIRFilter, 2>	highCutFilters;  							// low-pass
	float							applyCutoff(float sample, int channel);
};
