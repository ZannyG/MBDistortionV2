/*
  ==============================================================================

    LookAndFeel.h
    Created: 12 Jun 2023 1:32:28pm
    Author:  xande

  ==============================================================================
*/

#pragma once
#include<JuceHeader.h>

struct LookAndFeel : juce::LookAndFeel_V4
{
	void drawRotarySlider(juce::Graphics&,
		int x, int y, int width, int height,
		float sliderPosProportional,
		float rotaryStartAngle,
		float rotaryEndAngle,
		juce::Slider&) override;

	void drawToggleButton(juce::Graphics& g,
		juce::ToggleButton& toggleButton,
		bool shouldDrawButtonAsHighlighted,
		bool shouldDrawButtonAsDown) override;
};