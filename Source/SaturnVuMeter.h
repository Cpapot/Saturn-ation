#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class SaturnVuMeter  : public juce::Component,
                       private juce::Timer
{
public:
    explicit SaturnVuMeter (SaturnationAudioProcessor& processorToUse);
    ~SaturnVuMeter() override;

    void paint (juce::Graphics& g) override;

    void setMeterBackgroundColour (juce::Colour newColour);
    void setMeterFillColour (juce::Colour newColour);
    void setMeterPeakColour (juce::Colour newColour);
    void setMeterBorderColour (juce::Colour newColour);

private:
    void timerCallback() override;

    SaturnationAudioProcessor& processor;

    juce::Colour backgroundColour { juce::Colours::black.withAlpha (0.55f) };
    juce::Colour fillColour       { juce::Colours::limegreen };
    juce::Colour peakColour       { juce::Colours::red };
    juce::Colour borderColour     { juce::Colours::white.withAlpha (0.20f) };

    float displayedLevel = 0.0f;
    float peakHoldLevel  = 0.0f;
    juce::Image baseImage;    // contour (vuMeter.png)
    juce::Image colorImage;   // fill (vuMeterColor.png)
    bool hasBaseImage = false;
    bool hasColorImage = false;
};
