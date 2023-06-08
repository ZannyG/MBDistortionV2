/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
//#include "Distortion.h"
//==============================================================================
/**
*/
using WaveShaper = juce::dsp::WaveShaper<float, std::function<float(float)>>;
struct DistortionBand
{

	float inputGainInDecibels{ 0.0f }, outputGainInDecibels{ 0.0f }, drive{ 1.0f };
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

		//context.isBypassed = DistortionBand::bypassed->get();
		DistortionBand::processorChain.process(context);
	}

	void updateDistortionSettings()
	{
		//Distortion
		auto& waveshaper = DistortionBand::processorChain.template get<0>();
		float drive = DistortionBand::drive;
		float clipping{ 2.f };
		waveshaper.functionToUse = [drive, clipping](float x)
		{
			return juce::jlimit(float(-clipping), float(clipping), x * drive);
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
	juce::AudioParameterBool* bypassed{ nullptr };
private:

	//Distortion* p_distortion = new Distortion;
	DistortionBand distortion;
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MBDistortionAudioProcessor);
	void updateDistortionSettings(juce::AudioProcessorValueTreeState& apvts);
};
