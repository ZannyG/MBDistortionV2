/*
  ==============================================================================

    RotarySliderWithLabels.cpp
    Created: 12 Jun 2023 1:37:23pm
    Author:  xande

  ==============================================================================
*/

#include "RotarySliderWithLabels.h"
#include "Utilities.h"
#include "LookAndFeel.h"
void RotarySliderWithLabels::paint(juce::Graphics& g)
{
	using namespace juce;

	auto startAng = degreesToRadians(180.f + 45.f);
	auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

	auto range = getRange();

	auto sliderBounds = getSliderBounds();

	auto bounds = getLocalBounds();

	g.setColour(ColorScheme::getTextColor());
	g.drawFittedText(getName(), bounds.removeFromTop(getTextHeight() + 2), Justification::centredBottom, 1);

	getLookAndFeel().drawRotarySlider(g,
		sliderBounds.getX(),
		sliderBounds.getY(),
		sliderBounds.getWidth(),
		sliderBounds.getHeight(),
		jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
		startAng,
		endAng,
		*this);

	auto center = sliderBounds.toFloat().getCentre();
	auto radius = sliderBounds.getWidth() * 0.5f;

	g.setColour(ColorScheme::getTextColor());
	g.setFont(getTextHeight());

	auto numChoices = labels.size();
	for (int i = 0; i < numChoices; ++i)
	{
		auto pos = labels[i].pos;
		jassert(0.f <= pos);
		jassert(pos <= 1.f);

		auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);

		auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);

		Rectangle<float> r;
		auto str = labels[i].label;
		r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
		r.setCentre(c);
		r.setY(r.getY() + getTextHeight());

		g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
	}

}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
	auto bounds = getLocalBounds();
	bounds.removeFromTop(getTextHeight() * 1.5);

	auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

	size -= getTextHeight() * 1.5;
	juce::Rectangle<int> r;
	r.setSize(size, size);
	r.setCentre(bounds.getCentreX(), 0);
	r.setY(bounds.getY());

	return r;

}

juce::String RotarySliderWithLabels::getDisplayString() const
{
	if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
		return choiceParam->getCurrentChoiceName();

	juce::String str;
	bool addK = false;

	if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
	{
		float val = getValue();
		addK = truncateKiloValue(val);
		str = juce::String(val, (addK ? 2 : 0));
	}
	else
	{
		jassertfalse;
	}

	if (suffix.isNotEmpty())
	{
		str << " ";
		if (addK)
			str << "k";

		str << suffix;
	}

	return str;
}

void RotarySliderWithLabels::changeParam(juce::RangedAudioParameter* p)
{
	param = p;
	repaint();
}