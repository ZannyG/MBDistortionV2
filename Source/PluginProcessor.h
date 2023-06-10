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
		Low_Mid_Freq,
		Mid_High_Freq,

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
			{Low_Mid_Freq, "Low-Mid Frequency"},
			{Mid_High_Freq, "Mid-High Frequency"},

			{InputGain_Low_Band, "Low Input Gain"},
			{Distortion_Low_Band, "Low Distortion"},
			{OutputGain_Low_Band, "Low OutputGain"},

			{InputGain_Mid_Band, "Mid Input Gain"},
			{Distortion_Mid_Band, "Mid Distortiony"},
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
	float inputGainInDecibels{ 0.0f }, outputGainInDecibels{ 0.0f }, drive{ 1.0f };
	bool bypassed{false};
	void prepare(const juce::dsp::ProcessSpec& spec)
	{
		processorChain.prepare(spec);
	}

	void process(juce::AudioBuffer<float>& buffer)
	{
		//Input gain
		juce::dsp::AudioBlock<float> block(buffer);
		juce::dsp::ProcessContextReplacing<float> context(block);
		
		buffer.applyGain(juce::Decibels::decibelsToGain(DistortionBand::inputGainInDecibels));
		buffer.applyGain(juce::Decibels::decibelsToGain(DistortionBand::outputGainInDecibels));

		
		DistortionBand::processorChain.process(context);
	}

	void updateDistortionSettings()
	{
		//Distortion
		
		auto& waveshaper = DistortionBand::processorChain.template get<0>();
		float drive = DistortionBand::drive;
		float clipping{ 300.f };
		waveshaper.functionToUse = [drive, clipping](float x)
		{
			return juce::jlimit(float(-clipping), float(clipping), x * (drive * 2));
		};
	}
public:
	juce::dsp::ProcessorChain<WaveShaper> processorChain;
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
	//juce::AudioParameterBool* bypassed{ nullptr };

private:
	DistortionBand distortion;

	using Filter = juce::dsp::LinkwitzRileyFilter<float>;
	//		fc0	fc1
	Filter LP1, AP2,
		   HP1, LP2,
				HP2;

	Filter invAP1, invAP2;
	juce::AudioBuffer<float> invAPBuffer;

	juce::AudioParameterFloat* lowMidCrossover { nullptr };
	juce::AudioParameterFloat* midHighCrossover { nullptr };

	std::array<juce::AudioBuffer<float>, 3> filterBuffers;
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MBDistortionAudioProcessor);
	//void updateDistortionSettings(juce::AudioProcessorValueTreeState& apvts);
};
