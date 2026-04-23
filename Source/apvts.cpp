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

	// Low Cut Frequency (20 to 1000 Hz)
	params.push_back (std::make_unique<juce::AudioParameterFloat> (
		"lowCutoffFrequency", "Low Cut", 20.0f, 1000.0f, 20.0f));

	// High Cut Frequency (1000 to 20000 Hz)
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