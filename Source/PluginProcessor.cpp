/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SaturnationAudioProcessor::SaturnationAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

SaturnationAudioProcessor::~SaturnationAudioProcessor()
{
}

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

    // Pivot frequency for tilt-style tone control (dark <-> bright)
    const auto lowpassCoefficients = juce::IIRCoefficients::makeLowPass (sampleRate, 1200.0);
    for (auto& filter : toneLowpass)
    {
        filter.setCoefficients (lowpassCoefficients);
        filter.reset();
    }

    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void SaturnationAudioProcessor::releaseResources()
{
    for (auto& filter : toneLowpass)
        filter.reset();

    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
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
	sample *= driveAmount; // apply drive amount to the input signal

	switch (saturationMode)
	{
		// Hard clip: limiter between -threshold and +threshold
		case SaturationMode::HardClip:
		return std::clamp(sample, -1.0f, 1.0f);
		
		// Tube: use the hyperbolic tangent function to create a soft clipping effect
		case SaturationMode::Tube:
			return std::tanh(sample);
		
		// Tape: use a formula that simulates the saturation characteristics of tape machines
		case SaturationMode::Tape: {
			float absX = std::abs(sample);
			return sample / (1.0f + absX * 2.0f);
		}
		
		default:
			return sample;
	}
}

float SaturnationAudioProcessor::applyToneControl(float sample, int channel)
{
	// Convert toneAmount (-1.0 to 1.0) to a tilt in dB (-maxTiltDb to +maxTiltDb)
    const float maxTiltDb = 6.0f;
    const float tiltDb = toneAmount * maxTiltDb;

	// Calculate the gain for the high and low frequencies based on the tilt amount
	// if toneAmount is positive, gHigh > 1 and gLow < 1, boosting highs and cutting lows
	// if toneAmount is negative, gHigh < 1 and gLow > 1, cutting highs and boosting lows
    const float gHigh = std::pow (10.0f,  tiltDb / 20.0f);
    const float gLow  = std::pow (10.0f, -tiltDb / 20.0f);

    const int safeChannel = juce::jlimit (0, static_cast<int> (toneLowpass.size()) - 1, channel);

	// Apply the low-pass filter to get the low-frequency content, and subtract this from the original signal to get the high-frequency content
    const float low = toneLowpass[static_cast<size_t> (safeChannel)].processSingleSampleRaw (sample);
    const float high = sample - low;

	// Combine the low and high parts with their respective gains to create the final output
    return (low * gLow) + (high * gHigh);
}

void SaturnationAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	auto totalNumInputChannels  = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();

	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear (i, 0, buffer.getNumSamples());

	for (int channel = 0; channel < totalNumInputChannels; ++channel)
	{
		auto* channelData = buffer.getWritePointer (channel);

		for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
		{
            const float tonedSample = applyToneControl (channelData[sample], channel);
            channelData[sample] = applySaturation (tonedSample);
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
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SaturnationAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SaturnationAudioProcessor();
}
