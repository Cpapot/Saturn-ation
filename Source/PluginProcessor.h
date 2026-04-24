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

	juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SaturnationAudioProcessor)

	//=========================== apvts ============================
    juce::AudioProcessorValueTreeState::ParameterLayout	createParameterLayout();
	void												updateParameters();
	void												precalculateAllValues();
	juce::AudioProcessorValueTreeState					apvts;

	//=========================== General Parameters ============================
	bool							pluginIsEnabled = true;						// Master enable/disable for the plugin

    //=========================== Saturation Parameters ============================
    SaturationMode					saturationMode = SaturationMode::HardClip;	// Saturation mode
	float							driveAmount = 5.0f;							// Augment signal before clipping (0.1 to 10.0)
	float							applySaturation(float sample);

	//precalculated values for saturation
	float							driveLinear;

	//=========================== Tone Control Parameters ============================
	float							toneAmount = 0.0f;							// Tone control dark to bright (-1.0 to 1.0)
  	std::array<juce::IIRFilter, 2>	toneLowpass;								// Per-channel low-pass for tilt tone control
	float							applyToneControl(float sample, int channel);

	//precalculated values for tone control
	float 							gHigh;
	float							gLow;

	//=========================== Cutoff Control Parameters ============================
    float							lowCutoffFrequency = 20.0f;					// High-pass cutoff frequency used for low cut (20Hz to 1kHz)
    float							highCutoffFrequency = 20000.0f;				// Low-pass cutoff frequency used for high cut (20kHz to 1kHz)
    std::array<juce::IIRFilter, 2>	lowCutFilters;   							// High-pass filters (low cut)
    std::array<juce::IIRFilter, 2>	highCutFilters;  							// Low-pass filters (high cut)
    double							currentSampleRate = 44100.0;
    float							lastLowCutoffFrequency = -1.0f;
    float							lastHighCutoffFrequency = -1.0f;
	bool							lowCutIsActive = false;
	bool							highCutIsActive = false;
    void							updateCutoffFilterCoefficients();
	float							applyCutoff(float sample, int channel);

	//=========================== Mix Control Parameters ============================
	float							applyMix(float drySample, float wetSample);
	float							mixAmount = 0.5f;							// Mix between dry and wet signal (0.0 to 1.0)

	//precalculated values for mix control
	float 							mixLinear;
};
