#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout 
	SaturnationAudioProcessor::createParameterLayout()
{
	std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;


	// Master Enable (toggle)
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        "pluginIsEnabled", "Plugin Enabled", true));

	// Drive Amount (0.1 to 10.0)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "driveAmount", "Drive", 0.1f, 10.0f, 5.0f));

	// Saturation Mode (0 = HardClip, 1 = Tube, 2 = Tape)
	params.push_back (std::make_unique<juce::AudioParameterChoice> (
		"saturationMode", "Saturation", juce::StringArray ("Hard Clip", "Tube", "Tape"), 0));

	// Tone Amount (-1.0 to 1.0)
	params.push_back (std::make_unique<juce::AudioParameterFloat> (
		"toneAmount", "Tone", -1.0f, 1.0f, 0.0f));

	// Low Cut Frequency (high-pass cutoff, 20 to 1000 Hz)
	params.push_back (std::make_unique<juce::AudioParameterFloat> (
		"lowCutoffFrequency", "Low Cut", 20.0f, 1000.0f, 20.0f));


	// High Cut Frequency (low-pass cutoff, 1000 to 20000 Hz)
	params.push_back (std::make_unique<juce::AudioParameterFloat> (
		"highCutoffFrequency", "High Cut", 1000.0f, 20000.0f, 20000.0f));

	// Mix Amount (0.0 to 1.0)
	params.push_back (std::make_unique<juce::AudioParameterFloat> (
		"mixAmount", "Mix", 0.0f, 1.0f, 1.0f));

	return { params.begin(), params.end() };
}

// This function can be used to update any internal state based on parameter changes
void		SaturnationAudioProcessor::updateParameters()
{
    pluginIsEnabled       = apvts.getRawParameterValue ("pluginIsEnabled")->load();
    driveAmount           = apvts.getRawParameterValue ("driveAmount")->load();
    saturationMode        = static_cast<SaturationMode> (
        static_cast<int> (apvts.getRawParameterValue ("saturationMode")->load()));
    toneAmount            = apvts.getRawParameterValue ("toneAmount")->load();
    lowCutoffFrequency    = apvts.getRawParameterValue ("lowCutoffFrequency")->load();
    highCutoffFrequency   = apvts.getRawParameterValue ("highCutoffFrequency")->load();
    mixAmount             = apvts.getRawParameterValue ("mixAmount")->load();
}

void SaturnationAudioProcessor::precalculateAllValues()
{
	// Ensure driveAmount is within the expected range
	driveLinear = juce::jlimit (0.1f, 10.0f, driveAmount);

	const float maxTiltDb = 6.0f;
	// Ensure driveAmount is within the expected range
	// Convert toneAmount (-1.0 to 1.0) to a tilt in dB (-maxTiltDb to +maxTiltDb)
	const float tiltDb = juce::jlimit (-1.0f, 1.0f, toneAmount) * maxTiltDb;

	// Calculate the gain for the high and low frequencies based on the tilt amount
	// if toneAmount is positive, gHigh > 1 and gLow < 1, boosting highs and cutting lows
	// if toneAmount is negative, gHigh < 1 and gLow > 1, cutting highs and boosting lows
    gHigh = std::pow (10.0f,  tiltDb / 20.0f);
   	gLow  = std::pow (10.0f, -tiltDb / 20.0f);
	
	// Ensure mixAmount is within the expected range
	mixLinear = juce::jlimit (0.0f, 1.0f, mixAmount);
}