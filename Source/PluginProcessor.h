/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
//==============================================================================
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
	juce::dsp::ProcessorChain<juce::dsp::Gain<float>,WaveShaper, juce::dsp::Gain<float>> processorChain;
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
		float driveInGain{juce::Decibels::decibelsToGain(drive)};
		auto& waveshaper = processorChain.template get<waveShaperIndex>();
		waveshaper.functionToUse = [driveInGain, clipping](float x)
		{
			return juce::jlimit(float(-clipping), float(clipping), x * (driveInGain/10))* 1/clipping;
		};
	}
};


class MBDistortionAudioProcessor : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
	, public juce::AudioProcessorARAExtension
#endif
{
public:
	//==============================================================================
	MBDistortionAudioProcessor();
	~MBDistortionAudioProcessor() override;

	//==============================================================================
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;


#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

	//==============================================================================
	juce::AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override;

	//==============================================================================
	const juce::String getName() const override;

	bool acceptsMidi() const override;
	bool producesMidi() const override;
	bool isMidiEffect() const override;
	double getTailLengthSeconds() const override;

	//==============================================================================
	int getNumPrograms() override;
	int getCurrentProgram() override;
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;

	//==============================================================================
	void getStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	using APVTS = juce::AudioProcessorValueTreeState;
	static APVTS::ParameterLayout createParameterLayout();
	APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };

private:
	DistortionBand* p_lowBandDist;
	DistortionBand* p_midBandDist ;
	DistortionBand* p_highBandDist;

	using Filter = juce::dsp::LinkwitzRileyFilter<float>;

	Filter LP1, AP2,
		HP1, LP2,
		HP2;

	juce::AudioParameterFloat* lowMidCrossover { nullptr };
	juce::AudioParameterFloat* midHighCrossover { nullptr };

	std::array<juce::AudioBuffer<float>, 3> filterBuffers;

	void updateState();
	void splitBands(const juce::AudioBuffer<float>& inputBuffer);
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MBDistortionAudioProcessor);
};
