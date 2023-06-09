/*
  ==============================================================================

    SpectrumAnalyzer.h
    Created: 13 Jun 2023 4:09:40pm
    Author:  xande

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PathProducer.h"
struct SpectrumAnalyzer : juce::Component,
    juce::AudioProcessorParameter::Listener,
    juce::Timer
{
    SpectrumAnalyzer(MBDistortionAudioProcessor&);
    ~SpectrumAnalyzer();

    void parameterValueChanged(int parameterIndex, float newValue) override;

    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }

    void timerCallback() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void toggleAnalysisEnablement(bool enabled)
    {
        shouldShowFFTAnalysis = enabled;
    }
private:
    MBDistortionAudioProcessor& audioProcessor;

    bool shouldShowFFTAnalysis = true;

    juce::Atomic<bool> parametersChanged { false };

    void drawBackgroundGrid(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawTextLabels(juce::Graphics& g, juce::Rectangle<int> bounds);

    std::vector<float> getFrequencies();
    std::vector<float> getGains();
    std::vector<float> getXs(const std::vector<float>& freqs, float left, float width);

    juce::Rectangle<int> getRenderArea(juce::Rectangle<int> bounds);

    juce::Rectangle<int> getAnalysisArea(juce::Rectangle<int> bounds);

    PathProducer leftPathProducer, rightPathProducer; 

    void drawFFTAnalysis(juce::Graphics& g, juce::Rectangle<int>bounds);

    void drawCrossovers(juce::Graphics& g, juce::Rectangle<int>bounds);

    juce::AudioParameterFloat* lowMidXoverParam {nullptr };
    juce::AudioParameterFloat* midHighXoverParam {nullptr };

    juce::AudioParameterFloat* lowDistParam {nullptr };
    juce::AudioParameterFloat* midDistParam {nullptr };
    juce::AudioParameterFloat* highDistParam {nullptr };
    



};