/*
  ==============================================================================

    UtilityComponents.h
    Created: 12 Jun 2023 1:45:26pm
    Author:  xande

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct Placeholder : juce::Component
{
	Placeholder();
	void paint(juce::Graphics& g) override;

	juce::Colour customColor;
};
//==============================================================================
struct RotarySlider : juce::Slider
{
	RotarySlider();
};