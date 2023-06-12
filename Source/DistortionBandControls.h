/*
  ==============================================================================

    DistortionBandControls.h
    Created: 12 Jun 2023 2:00:12pm
    Author:  xande

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "RotarySliderWithLabels.h"
struct DistortionBandControls : juce::Component, juce::Button::Listener
{
	DistortionBandControls(juce::AudioProcessorValueTreeState& apvts);
	~DistortionBandControls() override;
	void resized() override;
	void paint(juce::Graphics& g) override;
	void buttonClicked(juce::Button* button) override;


private:
	juce::AudioProcessorValueTreeState& apvts;

	RotarySliderWithLabels inputGainSlider, distortionSlider, outputGainSlider;

	using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
	std::unique_ptr<Attachment> inputGainSliderAttachment, distortionSliderAttachment, outputGainSliderAttachment;

	juce::ToggleButton bypassButton, lowBandButton, midBandButton, highBandButton;

	using BtnAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
	std::unique_ptr<BtnAttachment> bypassButtonAttachment;

	juce::Component::SafePointer<DistortionBandControls> safePtr{this};

	void updateAttachments();
	void updateSliderEnablements();
};
