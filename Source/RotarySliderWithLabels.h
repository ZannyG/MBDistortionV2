/*
  ==============================================================================

    RotarySliderWithLabels.h
    Created: 12 Jun 2023 1:37:23pm
    Author:  xande

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
struct RotarySliderWithLabels : juce::Slider
{
	RotarySliderWithLabels(juce::RangedAudioParameter* rap, const juce::String& unitSuffix, const juce::String& title = "NO TITLE") :
		juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
			juce::Slider::TextEntryBoxPosition::NoTextBox),
		param(rap),
		suffix(unitSuffix)
	{
		setName(title);
	}

	struct LabelPos
	{
		float pos;
		juce::String label;
	};

	juce::Array<LabelPos> labels;

	void paint(juce::Graphics& g) override;
	juce::Rectangle<int> getSliderBounds() const;
	int getTextHeight() const { return 14; }
	juce::String getDisplayString() const;

	void changeParam(juce::RangedAudioParameter* p);
private:
	juce::RangedAudioParameter* param;
	juce::String suffix;
};