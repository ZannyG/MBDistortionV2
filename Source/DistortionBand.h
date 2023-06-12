/*
  ==============================================================================

    DistortionBand.h
    Created: 12 Jun 2023 2:12:27pm
    Author:  xande

  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include "Params.h"
struct DistortionBand
{
public:
	enum class BandFreq
	{
		lowBand,
		midBand,
		highBand,
	};

	DistortionBand(juce::AudioProcessorValueTreeState* p_apvts, BandFreq bandFreq);


	void prepare(const juce::dsp::ProcessSpec& spec)
	{
		processorChain.prepare(spec);
	}

	void process(const juce::dsp::ProcessContextReplacing<float>& context)
	{
		updateDistortionSettings();
		if (!bypassed)
		{
			processorChain.process(context);
		}
	}
private:
	juce::AudioProcessorValueTreeState* p_apvts;
	BandFreq bandFreq;
	juce::dsp::ProcessorChain<juce::dsp::Gain<float>, WaveShaper, juce::dsp::Gain<float>> processorChain;
	float drive{ 0.f };
	float inputGainInDecibels{ 0.0f }, outputGainInDecibels{ 0.0f };
	bool bypassed{ false };
	enum
	{
		preGainIndex,
		waveShaperIndex,
		postGainIndex,
	};
	void updateDistortionSettings()
	{
		using namespace Params;
		const auto& params = GetParams();
		switch (bandFreq)
		{
		case DistortionBand::BandFreq::lowBand:
			drive = p_apvts->getRawParameterValue(params.at(Names::Distortion_Low_Band))->load();
			inputGainInDecibels = p_apvts->getRawParameterValue(params.at(Names::InputGain_Low_Band))->load();
			outputGainInDecibels = p_apvts->getRawParameterValue(params.at(Names::OutputGain_Low_Band))->load();
			bypassed = p_apvts->getRawParameterValue(params.at(Names::Bypassed_Low_Band))->load();
			break;
		case DistortionBand::BandFreq::midBand:
			drive = p_apvts->getRawParameterValue(params.at(Names::Distortion_Mid_Band))->load();
			inputGainInDecibels = p_apvts->getRawParameterValue(params.at(Names::InputGain_Mid_Band))->load();
			outputGainInDecibels = p_apvts->getRawParameterValue(params.at(Names::OutputGain_Mid_Band))->load();
			bypassed = p_apvts->getRawParameterValue(params.at(Names::Bypassed_Mid_Band))->load();
			break;
		case DistortionBand::BandFreq::highBand:
			drive = p_apvts->getRawParameterValue(params.at(Names::Distortion_High_Band))->load();
			inputGainInDecibels = p_apvts->getRawParameterValue(params.at(Names::InputGain_High_Band))->load();
			outputGainInDecibels = p_apvts->getRawParameterValue(params.at(Names::OutputGain_High_Band))->load();
			bypassed = p_apvts->getRawParameterValue(params.at(Names::Bypassed_High_Band))->load();
			break;
		default:
			break;
		}

		auto& preGain = processorChain.template get<preGainIndex>();
		preGain.setGainDecibels(inputGainInDecibels);
		auto& postGain = processorChain.template get<postGainIndex>();
		postGain.setGainDecibels(outputGainInDecibels);

		float clipping{ 0.5f };
		float driveInGain{ juce::Decibels::decibelsToGain(drive) };
		auto& waveshaper = processorChain.template get<waveShaperIndex>();
		waveshaper.functionToUse = [driveInGain, clipping](float x)
		{
			return juce::jlimit(float(-clipping), float(clipping), x * (driveInGain / 10)) * 1 / clipping;
		};
	}
};