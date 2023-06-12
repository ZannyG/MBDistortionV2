/*
  ==============================================================================

    UtilityComponents.cpp
    Created: 12 Jun 2023 1:45:26pm
    Author:  xande

  ==============================================================================
*/

#include "UtilityComponents.h"
Placeholder::Placeholder()
{

    juce::Random r;
    customColor = juce::Colour(r.nextInt(255), r.nextInt(255), r.nextInt(255));
}

void Placeholder::paint(juce::Graphics& g)
{
    g.fillAll(customColor);
}
//==============================================================================

RotarySlider::RotarySlider() :
    juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox)
{}