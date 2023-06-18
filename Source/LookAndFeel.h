/*
  ==============================================================================

	LookAndFeel.h
	Created: 12 Jun 2023 1:32:28pm
	Author:  xande

  ==============================================================================
*/

#pragma once

#include<JuceHeader.h>

#define USE_LIVE_CONSTANT true

#if USE_LIVE_CONSTANT
#define colorHelper(c) JUCE_LIVE_CONSTANT(c);
#else
#define colorHelper(c) c;
#endif

namespace ColorScheme
{
	inline juce::Colour getSliderBorderColor()
	{
		return colorHelper(juce::Colour(0xff00d4ff));
	}

	inline juce::Colour getModuleBorderColor()
	{
		return colorHelper(juce::Colour(0xff1251c4));
	}
	inline juce::Colour getBackgroundColor()
	{
		return colorHelper(juce::Colour(0xff030a16));
	}
	inline juce::Colour getTextColor()
	{
		return colorHelper(juce::Colour(0xff17afbb));
	}
	inline juce::Colour getGridColor()
	{
		return colorHelper(juce::Colour(0xff4f6a78));
	}
	inline juce::Colour getFreqAndDBColor()
	{
		return colorHelper(juce::Colour(0xff20d442));
	}
	inline juce::Colour getCrossoverColor()
	{
		return colorHelper(juce::Colour(0xffe16406));
	}
	inline juce::Colour getDistColor()
	{
		return colorHelper(juce::Colour(0xffdadf0b));
	}
	inline juce::Colour getTitleColor()
	{
		return colorHelper(juce::Colour(0xffef9221));
	}
}
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