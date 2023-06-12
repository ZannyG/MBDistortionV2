/*
  ==============================================================================

    Params.h
    Created: 12 Jun 2023 2:08:37pm
    Author:  xande

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
using WaveShaper = juce::dsp::WaveShaper<float, std::function<float(float)>>;
namespace Params
{
	enum Names
	{
		Low_Mid_Crossover_Freq,
		Mid_High_Crossover_Freq,

		InputGain_Low_Band,
		Distortion_Low_Band,
		OutputGain_Low_Band,

		InputGain_Mid_Band,
		Distortion_Mid_Band,
		OutputGain_Mid_Band,

		InputGain_High_Band,
		Distortion_High_Band,
		OutputGain_High_Band,

		Bypassed_Low_Band,
		Bypassed_Mid_Band,
		Bypassed_High_Band,
	};

	inline const std::map<Names, juce::String>& GetParams()
	{
		static std::map<Names, juce::String> params =
		{
			{Low_Mid_Crossover_Freq, "Low-Mid Frequency"},
			{Mid_High_Crossover_Freq, "Mid-High Frequency"},

			{InputGain_Low_Band, "Low Input Gain"},
			{Distortion_Low_Band, "Low Distortion"},
			{OutputGain_Low_Band, "Low OutputGain"},

			{InputGain_Mid_Band, "Mid Input Gain"},
			{Distortion_Mid_Band, "Mid Distortion"},
			{OutputGain_Mid_Band, "Mid OutputGain"},


			{InputGain_High_Band, "High Input Gain"},
			{Distortion_High_Band, "High Distortion"},
			{OutputGain_High_Band, "High OutputGain"},

			{Bypassed_Low_Band, "Low Bypass"},
			{Bypassed_Mid_Band, "Mid Bypass"},
			{Bypassed_High_Band, "High Bypass"},
		};
		return params;
	}
}