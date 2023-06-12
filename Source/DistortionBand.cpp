/*
  ==============================================================================

    DistortionBand.cpp
    Created: 12 Jun 2023 2:12:27pm
    Author:  xande

  ==============================================================================
*/
#include "DistortionBand.h"
#include "Params.h"


void prepare(const juce::dsp::ProcessSpec& spec);

void process(const juce::dsp::ProcessContextReplacing<float>& context);

void updateDistortionSettings();

