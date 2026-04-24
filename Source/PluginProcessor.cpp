/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
#if JucePlugin_PreferredChannelConfigurations
SaturnationAudioProcessor::SaturnationAudioProcessor()
    : apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}
#else
SaturnationAudioProcessor::SaturnationAudioProcessor()
    : AudioProcessor (BusesProperties()
    #if ! JucePlugin_IsMidiEffect
        #if ! JucePlugin_IsSynth
            .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        #endif
            .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
    #endif
      ),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}
#endif

SaturnationAudioProcessor::~SaturnationAudioProcessor() {}

//==============================================================================
const juce::String SaturnationAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SaturnationAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SaturnationAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SaturnationAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SaturnationAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SaturnationAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SaturnationAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SaturnationAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SaturnationAudioProcessor::getProgramName (int index)
{
    return {};
}

void SaturnationAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SaturnationAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);
    currentSampleRate = sampleRate;

    // Pivot frequency for tilt-style tone control (dark <-> bright)
    const auto lowpassCoefficients = juce::IIRCoefficients::makeLowPass (sampleRate, 1200.0);
    for (auto& filter : toneLowpass)
    {
        filter.setCoefficients (lowpassCoefficients);
        filter.reset();
    }

    updateCutoffFilterCoefficients();

    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void SaturnationAudioProcessor::updateCutoffFilterCoefficients()
{
    const float lowCut = juce::jlimit (20.0f, 1000.0f, lowCutoffFrequency);
    const float highCut = juce::jlimit (1000.0f, 20000.0f, highCutoffFrequency);
    const float adjustedHighCut = juce::jmax (highCut, lowCut);

    const bool newLowCutIsActive = lowCut > 20.0001f;        // auto-off at minimum
    const bool newHighCutIsActive = adjustedHighCut < 19999.9f; // auto-off at maximum

    if (newLowCutIsActive)
    {
        if (!lowCutIsActive || std::abs (lowCut - lastLowCutoffFrequency) > 0.001f)
        {
            auto hp = juce::IIRCoefficients::makeHighPass (currentSampleRate, lowCut);
            for (auto& filter : lowCutFilters)
            {
                filter.setCoefficients (hp);
                if (!lowCutIsActive)
                    filter.reset();
            }
        }
    }
    else if (lowCutIsActive)
    {
        for (auto& filter : lowCutFilters)
            filter.reset();
    }

    if (newHighCutIsActive)
    {
        if (!highCutIsActive || std::abs (adjustedHighCut - lastHighCutoffFrequency) > 0.001f)
        {
            auto lp = juce::IIRCoefficients::makeLowPass (currentSampleRate, adjustedHighCut);
            for (auto& filter : highCutFilters)
            {
                filter.setCoefficients (lp);
                if (!highCutIsActive)
                    filter.reset();
            }
        }
    }
    else if (highCutIsActive)
    {
        for (auto& filter : highCutFilters)
            filter.reset();
    }

    lastLowCutoffFrequency = lowCut;
    lastHighCutoffFrequency = adjustedHighCut;
    lowCutIsActive = newLowCutIsActive;
    highCutIsActive = newHighCutIsActive;
}

void SaturnationAudioProcessor::releaseResources()
{
    for (auto& filter : toneLowpass)
        filter.reset();

	for (auto& filter : lowCutFilters)
		filter.reset();
	
	for (auto& filter : highCutFilters)
		filter.reset();

}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SaturnationAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

float SaturnationAudioProcessor::applySaturation(float sample)
{
	if (driveLinear <= 0.0001f)
		return sample;

	const float preGain = 1.0f + (driveLinear * 2.0f);
	// apply drive amount to the input signal
	float x = sample * preGain;
	float y = x;

	switch (saturationMode)
	{
		// Hard clip: limiter between -threshold and +threshold
		case SaturationMode::HardClip:
			y = std::clamp (x, -1.0f, 1.0f);
			break;
		
		// Tube: use the hyperbolic tangent function to create a soft clipping effect
		case SaturationMode::Tube:
			y = std::tanh (x);
			break;
		
		// Tape: use a formula that simulates the saturation characteristics of tape machines
		case SaturationMode::Tape: {
			const float absX = std::abs (x);
			y = x / (1.0f + absX * 2.0f);
			break;
		}
		
		default:
			y = x;
			break;
	}
	// compensate for perceived loudness increase
	const float loundessCompensation = 1.0f + (driveLinear * 0.10f);
	return y * loundessCompensation;
}

float SaturnationAudioProcessor::applyToneControl(float sample, int channel)
{
	const int safeChannel = juce::jlimit (0, static_cast<int> (toneLowpass.size()) - 1, channel);

	// Apply the low-pass filter to get the low-frequency content, and subtract this from the original signal to get the high-frequency content
	const float low = toneLowpass[static_cast<size_t> (safeChannel)].processSingleSampleRaw (sample);
	const float high = sample - low;

	// Combine the low and high parts with their respective gains to create the final output
	return (low * gLow) + (high * gHigh);
}

float	SaturnationAudioProcessor::applyCutoff(float sample, int channel)
{
	// Ensure that the channel index is within the bounds of the filter arrays
	const int safeChannel = juce::jlimit (0, static_cast<int> (lowCutFilters.size()) - 1, channel);

    float y = sample;

    if (lowCutIsActive)
        y = lowCutFilters[(size_t) safeChannel].processSingleSampleRaw (y);

    if (highCutIsActive)
        y = highCutFilters[(size_t) safeChannel].processSingleSampleRaw (y);

	return y;
}

float	SaturnationAudioProcessor::applyMix(float drySample, float wetSample)
{
	// Linear crossfade between dry and wet signals based on mixAmount
	return (drySample * (1.0f - mixLinear)) + (wetSample * mixLinear);
}

void SaturnationAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused (midiMessages);
	auto totalNumInputChannels  = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();

	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear (i, 0, buffer.getNumSamples());


	this->updateParameters();
	this->precalculateAllValues();

    const float currentLowCut = juce::jlimit (20.0f, 1000.0f, lowCutoffFrequency);
    const float currentHighCut = juce::jmax (juce::jlimit (1000.0f, 20000.0f, highCutoffFrequency), currentLowCut);
    const bool currentLowCutIsActive = currentLowCut > 20.0001f;
    const bool currentHighCutIsActive = currentHighCut < 19999.9f;
    if (std::abs (currentLowCut - lastLowCutoffFrequency) > 0.001f
     || std::abs (currentHighCut - lastHighCutoffFrequency) > 0.001f
     || currentLowCutIsActive != lowCutIsActive
     || currentHighCutIsActive != highCutIsActive)
    {
        updateCutoffFilterCoefficients();
    }

	if (!pluginIsEnabled)
		return ;

	for (int channel = 0; channel < totalNumInputChannels; ++channel)
	{
		auto* channelData = buffer.getWritePointer (channel);

		for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
		{
			const float drySample   = channelData[sample];
			const float tonedSample = applyToneControl (drySample, channel);
			const float satSample   = applySaturation (tonedSample);
			const float wetSample   = applyCutoff (satSample, channel);
			channelData[sample]		= applyMix (drySample, wetSample);
		}
	}
}

//==============================================================================
bool SaturnationAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SaturnationAudioProcessor::createEditor()
{
    return new SaturnationAudioProcessorEditor (*this);
}

//==============================================================================
void SaturnationAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
	// use to restore your parameters from this memory block, whose contents will have been created by the getStateInformation() call.
    if (auto state = apvts.copyState(); state.isValid())
    {
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        copyXmlToBinary (*xml, destData);
    }
}

void SaturnationAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
    {
        auto newTree = juce::ValueTree::fromXml (*xmlState);

        if (newTree.isValid() && newTree.hasType (apvts.state.getType()))
            apvts.replaceState (newTree);
    }

	// After loading new state, update internal parameters to match the new values
    updateParameters();
	precalculateAllValues();
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SaturnationAudioProcessor();
}
