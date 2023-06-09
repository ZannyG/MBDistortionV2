/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
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

	bypassed = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter("Bypassed"));
	jassert(bypassed != nullptr);

	lowCrossover = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Low-MidLow Frequency"));
	jassert(lowCrossover != nullptr);
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

	distortion.prepare(spec);

	LP.prepare(spec);
	HP.prepare(spec);

	for (auto& buffer : filterBuffers)
	{
		buffer.setSize(spec.numChannels, samplesPerBlock);
	}
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

void MBDistortionAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
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

	updateDistortionSettings(apvts);
	//distortion.updateDistortionSettings();
	//distortion.process(buffer);

	for (auto& fb : filterBuffers)
	{
		fb = buffer;
	}

	auto cutoff = lowCrossover->get();
	LP.setCutoffFrequency(cutoff);
	HP.setCutoffFrequency(cutoff);

	auto fb0Block = juce::dsp::AudioBlock<float>(filterBuffers[0]);
	auto fb1Block = juce::dsp::AudioBlock<float>(filterBuffers[1]);

	auto fb0Ctx = juce::dsp::ProcessContextReplacing<float>(fb0Block);
	auto fb1Ctx = juce::dsp::ProcessContextReplacing<float>(fb1Block);

	LP.process(fb0Ctx);
	HP.process(fb1Ctx);

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
	//return new MBDistortionAudioProcessorEditor (*this);
	return new juce::GenericAudioProcessorEditor(*this);

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

void MBDistortionAudioProcessor::updateDistortionSettings(juce::AudioProcessorValueTreeState& apvts)
{
	using namespace Params;
	const auto& params = GetParams();

	auto floatHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName)
	{
		auto* parameter = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(params.at(paramName)));
		jassert(parameter != nullptr);
	};

	floatHelper(distortion.inputGainInDecibels, Names::InputGain_Low_Band);
	floatHelper(distortion.drive, Names::Distortion_Low_Band);
	floatHelper(distortion.outputGainInDecibels, Names::OutputGain_Low_Band);
	floatHelper(lowCrossover, Names::Low_MidLow_Freq);

	LP.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
	HP.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
}


juce::AudioProcessorValueTreeState::ParameterLayout MBDistortionAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;
	using namespace Params;
	const auto& params = GetParams();

	auto driveRange = NormalisableRange<float>(1, 100, 1, 1);
	auto gainRange = NormalisableRange<float>(-24, 24, 0.5, 1);

	layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::InputGain_Low_Band), params.at(Names::InputGain_Low_Band), gainRange, 0));
	layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Distortion_Low_Band), params.at(Names::Distortion_Low_Band), driveRange, 0));
	layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::OutputGain_Low_Band), params.at(Names::OutputGain_Low_Band), gainRange, 0));
	layout.add(std::make_unique <AudioParameterBool>("Bypassed", "Bypassed", false));

	layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Low_MidLow_Freq), params.at(Names::Low_MidLow_Freq), NormalisableRange<float>(20, 20000, 1, 1), 500));
	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new MBDistortionAudioProcessor();
}
