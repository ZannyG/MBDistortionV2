/*
  ==============================================================================

    GlobalControls.h
    Created: 12 Jun 2023 2:03:45pm
    Author:  xande

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "RotarySliderWithLabels.h"
struct GlobalControls : juce::Component
{
	GlobalControls(juce::AudioProcessorValueTreeState& apvts);
	void paint(juce::Graphics& g) override;

	void resized() override;
private:
	using RSWL = RotarySliderWithLabels;
	std::unique_ptr<RSWL> lowMidXoverSlider, midHighXoverSlider;

	using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
	std::unique_ptr<Attachment> lowMidXoverSliderAttachment, midHighXoverSliderAttachment;
};