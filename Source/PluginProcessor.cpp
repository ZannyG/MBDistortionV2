/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Params.h"
#include "DistortionBand.h"
//==============================================================================
DistortionBand::DistortionBand(juce::AudioProcessorValueTreeState* apvts, BandFreq bandFrequency) :
	bandFreq{ bandFrequency },
	p_apvts{ apvts }
{
	updateDistortionSettings();
}

MBDistortionAudioProcessor::MBDistortionAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	)
#endif
{
	p_lowBandDist = new DistortionBand(&apvts, DistortionBand::BandFreq::lowBand);
	p_midBandDist = new DistortionBand(&apvts, DistortionBand::BandFreq::midBand);
	p_highBandDist = new DistortionBand(&apvts, DistortionBand::BandFreq::highBand);

	using namespace Params;
	const auto& params = GetParams();

	lowMidCrossover = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Low-Mid Frequency"));
	jassert(lowMidCrossover != nullptr);
	midHighCrossover = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Mid-High Frequency"));
	jassert(lowMidCrossover != nullptr);

	LP1.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
	HP1.setType(juce::dsp::LinkwitzRileyFilterType::highpass);

	AP2.setType(juce::dsp::LinkwitzRileyFilterType::allpass);

	LP2.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
	HP2.setType(juce::dsp::LinkwitzRileyFilterType::highpass);

}

MBDistortionAudioProcessor::~MBDistortionAudioProcessor()
{
}


//==============================================================================
const juce::String MBDistortionAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool MBDistortionAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool MBDistortionAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool MBDistortionAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double MBDistortionAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int MBDistortionAudioProcessor::getNumPrograms()
{
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
	// so this should be at least 1, even if you're not really implementing programs.
}

int MBDistortionAudioProcessor::getCurrentProgram()
{
	return 0;
}

void MBDistortionAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String MBDistortionAudioProcessor::getProgramName(int index)
{
	return {};
}

void MBDistortionAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void MBDistortionAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	juce::dsp::ProcessSpec spec;
	spec.maximumBlockSize = samplesPerBlock;
	spec.numChannels = getTotalNumInputChannels();
	spec.sampleRate = sampleRate;

	LP1.prepare(spec);
	HP1.prepare(spec);

	AP2.prepare(spec);

	LP2.prepare(spec);
	HP2.prepare(spec);

	for (auto& buffer : filterBuffers)
	{
		buffer.setSize(spec.numChannels, samplesPerBlock);
	}

	leftChannelFifo.prepare(samplesPerBlock);	
	rightChannelFifo.prepare(samplesPerBlock);


}

void MBDistortionAudioProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MBDistortionAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
	return true;
#else
	// This is the place where you check if the layout is supported.
	// In this template code we only support mono or stereo.
	// Some plugin hosts, such as certain GarageBand versions, will only
	// load plugins that support stereo bus layouts.
	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
		&& layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
		return false;

	// This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
#endif

	return true;
#endif
}
#endif
void MBDistortionAudioProcessor::updateState()
{
	auto lowMidCutoffFreq = lowMidCrossover->get();
	LP1.setCutoffFrequency(lowMidCutoffFreq);
	HP1.setCutoffFrequency(lowMidCutoffFreq);

	auto midHighCutoffFreq = midHighCrossover->get();

	AP2.setCutoffFrequency(midHighCutoffFreq);
	LP2.setCutoffFrequency(midHighCutoffFreq);
	HP2.setCutoffFrequency(midHighCutoffFreq);
}

void MBDistortionAudioProcessor::splitBands(const juce::AudioBuffer<float>& inputBuffer)
{
	for (auto& fb : filterBuffers)
	{
		fb = inputBuffer;
	}

	auto fb0Block = juce::dsp::AudioBlock<float>(filterBuffers[0]);
	auto fb1Block = juce::dsp::AudioBlock<float>(filterBuffers[1]);
	auto fb2Block = juce::dsp::AudioBlock<float>(filterBuffers[2]);

	auto fb0Ctx = juce::dsp::ProcessContextReplacing<float>(fb0Block);
	auto fb1Ctx = juce::dsp::ProcessContextReplacing<float>(fb1Block);
	auto fb2Ctx = juce::dsp::ProcessContextReplacing<float>(fb2Block);

	LP1.process(fb0Ctx);
	AP2.process(fb0Ctx);

	HP1.process(fb1Ctx);
	filterBuffers[2] = filterBuffers[1];
	LP2.process(fb1Ctx);
	HP2.process(fb2Ctx);

	p_lowBandDist->process(fb0Ctx);
	p_midBandDist->process(fb1Ctx);
	p_highBandDist->process(fb2Ctx);
}

void MBDistortionAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	using namespace Params;
	juce::ScopedNoDenormals noDenormals;
	auto totalNumInputChannels = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();

	// In case we have more outputs than inputs, this code clears any output
	// channels that didn't contain input data, (because these aren't
	// guaranteed to be empty - they may contain garbage).
	// This is here to avoid people getting screaming feedback
	// when they first compile a plugin, but obviously you don't need to keep
	// this code if your algorithm always overwrites all the output channels.
	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear(i, 0, buffer.getNumSamples());

	
	updateState();
	leftChannelFifo.update(buffer);
	leftChannelFifo.update(buffer);
	splitBands(buffer);

	auto numSamples = buffer.getNumSamples();
	auto numChannels = buffer.getNumChannels();

	buffer.clear();


	auto addFilterBand = [nc = numChannels, ns = numSamples](auto& inputBuffer, const auto& source)
	{
		for (auto i = 0; i < nc; ++i)
		{
			inputBuffer.addFrom(i, 0, source, i, 0, ns);
		}
	};

	addFilterBand(buffer, filterBuffers[0]);
	addFilterBand(buffer, filterBuffers[1]);
	addFilterBand(buffer, filterBuffers[2]);

	// This is the place where you'd normally do the guts of your plugin's
	// audio processing...
	// Make sure to reset the state if your inner loop is processing
	// the samples and the outer loop is handling the channels.
	// Alternatively, you can process the samples with the channels
	// interleaved by keeping the same state.
	for (int channel = 0; channel < totalNumInputChannels; ++channel)
	{
		auto* channelData = buffer.getWritePointer(channel);

		// ..do something to the data...
	}
}

//==============================================================================
bool MBDistortionAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MBDistortionAudioProcessor::createEditor()
{
	return new MBDistortionAudioProcessorEditor(*this);
	//return new juce::GenericAudioProcessorEditor(*this);

}

//==============================================================================
void MBDistortionAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.
	juce::MemoryOutputStream mos(destData, true);
	apvts.state.writeToStream(mos);
}

void MBDistortionAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
	if (tree.isValid())
	{
		apvts.replaceState(tree);
	}
	// You should use this method to restore your parameters from this memory block,
	// whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout MBDistortionAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;
	using namespace Params;
	const auto& params = GetParams();

	auto driveRange = NormalisableRange<float>(MIN_DIST, 100, 1, 1);
	auto gainRange = NormalisableRange<float>(-MAX_DECIBELS, MAX_DECIBELS, 0.5, 1);

	layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::InputGain_Low_Band), params.at(Names::InputGain_Low_Band), gainRange, 0));
	layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Distortion_Low_Band), params.at(Names::Distortion_Low_Band), driveRange, 0));
	layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::OutputGain_Low_Band), params.at(Names::OutputGain_Low_Band), gainRange, 0));

	layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::InputGain_Mid_Band), params.at(Names::InputGain_Mid_Band), gainRange, 0));
	layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Distortion_Mid_Band), params.at(Names::Distortion_Mid_Band), driveRange, 0));
	layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::OutputGain_Mid_Band), params.at(Names::OutputGain_Mid_Band), gainRange, 0));

	layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::InputGain_High_Band), params.at(Names::InputGain_High_Band), gainRange, 0));
	layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Distortion_High_Band), params.at(Names::Distortion_High_Band), driveRange, 0));
	layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::OutputGain_High_Band), params.at(Names::OutputGain_High_Band), gainRange, 0));

	layout.add(std::make_unique <AudioParameterBool>(params.at(Names::Bypassed_Low_Band), params.at(Names::Bypassed_Low_Band), false));
	layout.add(std::make_unique <AudioParameterBool>(params.at(Names::Bypassed_Mid_Band), params.at(Names::Bypassed_Mid_Band), false));
	layout.add(std::make_unique <AudioParameterBool>(params.at(Names::Bypassed_High_Band), params.at(Names::Bypassed_High_Band), false));

	layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Low_Mid_Crossover_Freq), params.at(Names::Low_Mid_Crossover_Freq), NormalisableRange<float>(20, 999, 1, 1), 400));
	layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Mid_High_Crossover_Freq), params.at(Names::Mid_High_Crossover_Freq), NormalisableRange<float>(1000, 20000, 1, 1), 2000));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new MBDistortionAudioProcessor();
}