/*
  ==============================================================================

	SpectrumAnalyzer.cpp
	Created: 13 Jun 2023 4:09:40pm
	Author:  xande

  ==============================================================================
*/

#include "SpectrumAnalyzer.h"
#include "PathProducer.h"
#include "Utilities.h"
#include "Params.h"
#include "LookAndFeel.h"
SpectrumAnalyzer::SpectrumAnalyzer(MBDistortionAudioProcessor& p) :
	audioProcessor(p),
	leftPathProducer(audioProcessor.leftChannelFifo),
	rightPathProducer(audioProcessor.rightChannelFifo)
{
	const auto& params = audioProcessor.getParameters();
	for (auto param : params)
	{
		param->addListener(this);
	}
	using namespace Params;
	const auto& paramNames = GetParams();

	auto floatHelper = [&apvts = audioProcessor.apvts, &paramNames](auto& param, const auto& paramName)
	{
		param = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(paramNames.at(paramName)));
		jassert(param != nullptr);
	};
	floatHelper(lowMidXoverParam, Names::Low_Mid_Crossover_Freq);
	floatHelper(midHighXoverParam, Names::Mid_High_Crossover_Freq);

	floatHelper(lowDistParam, Names::Distortion_Low_Band);
	floatHelper(midDistParam, Names::Distortion_Mid_Band);
	floatHelper(highDistParam, Names::Distortion_High_Band);

	startTimerHz(60);
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
	const auto& params = audioProcessor.getParameters();
	for (auto param : params)
	{
		param->removeListener(this);
	}
}

void SpectrumAnalyzer::drawFFTAnalysis(juce::Graphics& g, juce::Rectangle<int>bounds)

{

	using namespace juce;
	auto responseArea = getAnalysisArea(bounds);

	Graphics::ScopedSaveState sss(g);
	g.reduceClipRegion(responseArea);

	auto leftChannelFFTPath = leftPathProducer.getPath();
	leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), 0));

	g.setColour(ColorScheme::getSliderBorderColor());
	g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));

	auto rightChannelFFTPath = rightPathProducer.getPath();
	rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), 0));

	g.setColour(Colour(215u, 201u, 134u));
	g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));
}

void SpectrumAnalyzer::paint(juce::Graphics& g)
{
	using namespace juce;
	g.fillAll(ColorScheme::getBackgroundColor());

	auto bounds = drawModuleBackground(g, getLocalBounds());

	drawBackgroundGrid(g, bounds);

	if (shouldShowFFTAnalysis)
	{
		drawFFTAnalysis(g, bounds);
	}

	drawCrossovers(g, bounds);

	drawTextLabels(g, bounds);

}

void SpectrumAnalyzer::drawCrossovers(juce::Graphics& g, juce::Rectangle<int>bounds)
{
	using namespace juce;
	bounds = getAnalysisArea(bounds);

	const auto top = bounds.getY();
	const auto bottom = bounds.getBottom();
	const auto left = bounds.getX();
	const auto right = bounds.getRight();


	auto mapX = [left = bounds.getX(), width = bounds.getWidth()](float frequency) {
		auto normX = juce::mapFromLog10(frequency, MIN_FREQUENCY, MAX_FREQUENCY);
		return left + width * normX;
	};
	auto lowMidX = mapX(lowMidXoverParam->get());
	g.setColour(ColorScheme::getCrossoverColor());
	g.drawVerticalLine(lowMidX, top, bottom);

	auto midHighX = mapX(midHighXoverParam->get());
	g.drawVerticalLine(midHighX, top, bottom);

	auto mapY = [bottom, top](float db) {
		return jmap(db, NEGATIVE_INFINITY, MAX_DECIBELS, float(bottom), float(top));
	};
	g.setColour(ColorScheme::getDistColor());
	g.drawHorizontalLine(mapY(lowDistParam->get()/1.05f-72), left, lowMidX);
	g.drawHorizontalLine(mapY(midDistParam->get() / 1.05f - 72), lowMidX, midHighX);
	g.drawHorizontalLine(mapY(highDistParam->get() / 1.05f - 72), midHighX, right);
}

std::vector<float> SpectrumAnalyzer::getFrequencies()
{
	return std::vector<float>
	{
		20,  50, 100,
			200,  500, 1000,
			2000,  5000, 10000,
			20000
	};
}

std::vector<float> SpectrumAnalyzer::getGains()
{
	
	std::vector<float> values;
	auto increment = MAX_DECIBELS - 12;
	for (auto db = NEGATIVE_INFINITY; db <= MAX_DECIBELS; db += increment)
	{
		values.push_back(db);
	}
	return values;
}

std::vector<float> SpectrumAnalyzer::getXs(const std::vector<float>& freqs, float left, float width)
{
	std::vector<float> xs;
	for (auto f : freqs)
	{
		auto normX = juce::mapFromLog10(f, MIN_FREQUENCY, MAX_FREQUENCY);
		xs.push_back(left + width * normX);
	}

	return xs;
}

void SpectrumAnalyzer::drawBackgroundGrid(juce::Graphics& g, juce::Rectangle<int> bounds)
{
	using namespace juce;
	auto freqs = getFrequencies();

	auto renderArea = getAnalysisArea(bounds);
	auto left = renderArea.getX();
	auto right = renderArea.getRight();
	auto top = renderArea.getY();
	auto bottom = renderArea.getBottom();
	auto width = renderArea.getWidth();

	auto xs = getXs(freqs, left, width);

	g.setColour(ColorScheme::getGridColor());
	for (auto x : xs)
	{
		g.drawVerticalLine(x, top, bottom);
	}

	auto gain = getGains();

	for (auto gDb : gain)
	{
	
		auto y = jmap(gDb, NEGATIVE_INFINITY, MAX_DECIBELS, float(bottom), float(top));
	
		g.setColour(gDb == 0.f ? ColorScheme::getFreqAndDBColor() : ColorScheme::getGridColor());
		g.drawHorizontalLine(y, left, right);
	}
}

void SpectrumAnalyzer::drawTextLabels(juce::Graphics& g, juce::Rectangle<int> bounds)
{
	using namespace juce;
	g.setColour(ColorScheme::getFreqAndDBColor());
	const int fontHeight = 10;
	g.setFont(fontHeight);

	auto renderArea = getAnalysisArea(bounds);
	auto left = renderArea.getX();

	auto top = renderArea.getY();
	auto bottom = renderArea.getBottom();
	auto width = renderArea.getWidth();

	auto freqs = getFrequencies();
	auto xs = getXs(freqs, left, width);

	for (int i = 0; i < freqs.size(); ++i)
	{
		auto f = freqs[i];
		auto x = xs[i];

		bool addK = false;
		String str;
		if (f > 999.f)
		{
			addK = true;
			f /= 1000.f;
		}

		str << f;
		if (addK)
			str << "k";
		str << "Hz";

		auto textWidth = g.getCurrentFont().getStringWidth(str);

		Rectangle<int> r;

		r.setSize(textWidth, fontHeight);
		r.setCentre(x, 0);
		

		r.setY(bounds.getY());

		g.drawFittedText(str, r, juce::Justification::centred, 1);
	}

	auto gain = getGains();

	for (auto gDb : gain)
	{

		auto y = jmap(gDb, NEGATIVE_INFINITY, MAX_DECIBELS, float(bottom), float(top));

		String str;
		if (gDb > 0)
			str << "+";
		str << gDb;

		auto textWidth = g.getCurrentFont().getStringWidth(str);

		Rectangle<int> r;
		r.setSize(textWidth, fontHeight);
		r.setX(bounds.getRight() - textWidth);
		r.setCentre(r.getCentreX(), y);

		g.setColour(gDb == 0.f ? ColorScheme::getFreqAndDBColor() : ColorScheme::getTextColor());

		g.drawFittedText(str, r, juce::Justification::centredLeft, 1);

		r.setX(bounds.getX() - 1);
		g.drawFittedText(str, r, juce::Justification::centredLeft, 1);
	}
}

void SpectrumAnalyzer::resized()
{
	using namespace juce;
	auto bounds = getLocalBounds();
	auto fftBounds = getAnalysisArea(bounds).toFloat();
	auto negInf = jmap(bounds.toFloat().getBottom(), fftBounds.getBottom(), fftBounds.getY(), NEGATIVE_INFINITY, MAX_DECIBELS);
	leftPathProducer.updateNegativeInfinity(negInf);
	rightPathProducer.updateNegativeInfinity(negInf);

}

void SpectrumAnalyzer::parameterValueChanged(int parameterIndex, float newValue)
{
	parametersChanged.set(true);
}

void SpectrumAnalyzer::timerCallback()
{
	if (shouldShowFFTAnalysis)
	{
		auto bounds = getLocalBounds();
		auto fftBounds = getAnalysisArea(bounds).toFloat();
		fftBounds.setBottom(getLocalBounds().getBottom());
		auto sampleRate = audioProcessor.getSampleRate();

		leftPathProducer.process(fftBounds, sampleRate);
		rightPathProducer.process(fftBounds, sampleRate);
	}

	if (parametersChanged.compareAndSetBool(false, true))
	{

	}

	repaint();
}

juce::Rectangle<int> SpectrumAnalyzer::getRenderArea(juce::Rectangle<int> bounds)
{

	bounds.removeFromTop(12);
	bounds.removeFromBottom(2);
	bounds.removeFromLeft(20);
	bounds.removeFromRight(20);

	return bounds;
}

juce::Rectangle<int> SpectrumAnalyzer::getAnalysisArea(juce::Rectangle<int> bounds)
{
	bounds = getRenderArea(bounds);
	bounds.removeFromTop(4);
	bounds.removeFromBottom(4);
	return bounds;
}