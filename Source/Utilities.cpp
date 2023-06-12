/*
  ==============================================================================

    Utilities.cpp
    Created: 12 Jun 2023 1:53:18pm
    Author:  xande

  ==============================================================================
*/

#include "Utilities.h"

#include "LookAndFeel.h"

juce::String getValString(const juce::RangedAudioParameter& param, bool getLow, juce::String suffix)
{
	juce::String str;
	auto val = getLow ? param.getNormalisableRange().start : param.getNormalisableRange().end;
	bool useK = truncateKiloValue(val);
	str << val;
	if (useK)
	{
		str << "K";
	}
	str << suffix;

	return str;
}

void drawModuleBackground(juce::Graphics& g, juce::Rectangle<int> bounds)
{
	using namespace juce;
	g.setColour(Colours::blueviolet);
	g.fillAll();
	auto localBound = bounds;
	bounds.reduce(3, 3);
	g.setColour(Colours::black);
	g.fillRoundedRectangle(bounds.toFloat(), 3);
}