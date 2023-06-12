/*
  ==============================================================================

    CustomButtons.h
    Created: 12 Jun 2023 1:41:14pm
    Author:  xande

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct PowerButton : juce::ToggleButton { };

struct AnalyzerButton : juce::ToggleButton
{
	void resized() override;
	
	juce::Path randomPath;
};