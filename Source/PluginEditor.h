/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include "LookAndFeel.h"
#include "GlobalControls.h"
#include "DistortionBandControls.h"
#include "UtilityComponents.h"
#include "CustomButtons.h"
#include "SpectrumAnalyzer.h"



//==============================================================================
/**
*/
class MBDistortionAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    MBDistortionAudioProcessorEditor(MBDistortionAudioProcessor&);
    ~MBDistortionAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    LookAndFeel lnf;

    MBDistortionAudioProcessor& audioProcessor;

    Placeholder controlBar;
    GlobalControls globalControls{ audioProcessor.apvts };
    DistortionBandControls bandControls{ audioProcessor.apvts };
    SpectrumAnalyzer analyzer{ audioProcessor };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MBDistortionAudioProcessorEditor)
};
