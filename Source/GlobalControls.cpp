/*
  ==============================================================================

    GlobalControls.cpp
    Created: 12 Jun 2023 2:03:45pm
    Author:  xande

  ==============================================================================
*/

#include "GlobalControls.h"
#include "Params.h"
#include "Utilities.h"
GlobalControls::GlobalControls(juce::AudioProcessorValueTreeState& apvts)
{
	using namespace Params;
	const auto& params = GetParams();

	auto getParamHelper = [&params, &apvts](const auto& name) -> auto&
	{
		return getParam(apvts, params, name);
	};

	auto& lowMidParam = getParamHelper(Names::Low_Mid_Crossover_Freq);
	auto& midHighParam = getParamHelper(Names::Mid_High_Crossover_Freq);

	lowMidXoverSlider = std::make_unique<RSWL>(&lowMidParam, "Hz", "LOW_MID X-OVER");
	midHighXoverSlider = std::make_unique<RSWL>(&midHighParam, "Hz", "MID_HIGH X-OVER");

	auto makeAttachmentHelper = [&params, &apvts](auto& attachment, const auto& name, auto& slider) {
		makeAttachment(attachment, apvts, params, name, slider);
	};

	makeAttachmentHelper(lowMidXoverSliderAttachment, Names::Low_Mid_Crossover_Freq, *lowMidXoverSlider);
	makeAttachmentHelper(midHighXoverSliderAttachment, Names::Mid_High_Crossover_Freq, *midHighXoverSlider);

	addLabelPairs(lowMidXoverSlider->labels, lowMidParam, "Hz");
	addLabelPairs(midHighXoverSlider->labels, midHighParam, "Hz");

	addAndMakeVisible(*lowMidXoverSlider);
	addAndMakeVisible(*midHighXoverSlider);
}
void GlobalControls::paint(juce::Graphics& g)
{
	auto bounds = getLocalBounds();
	drawModuleBackground(g, bounds);
}
void GlobalControls::resized()
{
	using namespace juce;
	auto bounds = getLocalBounds().reduced(5);

	auto spacer = FlexItem().withWidth(4);
	auto endCap = FlexItem().withWidth(6);

	FlexBox flexBox;
	flexBox.flexDirection = FlexBox::Direction::row;
	flexBox.flexWrap = FlexBox::Wrap::noWrap;

	flexBox.items.add(endCap);
	flexBox.items.add(FlexItem(*lowMidXoverSlider).withFlex(1.f));
	flexBox.items.add(spacer);
	flexBox.items.add(FlexItem(*midHighXoverSlider).withFlex(1.f));
	flexBox.items.add(endCap);

	flexBox.performLayout(bounds);
}