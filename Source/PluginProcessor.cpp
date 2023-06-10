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

	floatHelper(lowMidCrossover, Names::Low_Mid_Freq);
	floatHelper(midHighCrossover, Names::Mid_High_Freq);



	lowMidCrossover = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Low-Mid Frequency"));
	jassert(lowMidCrossover != nullptr);
	midHighCrossover = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Mid-High Frequency"));
	jassert(lowMidCrossover != nullptr);

	LP1.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
	HP1.setType(juce::dsp::LinkwitzRileyFilterType::highpass);

	AP2.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
	
	LP2.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
	HP2.setType(juce::dsp::LinkwitzRileyFilterType::highpass);

	invAP1.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
	invAP2.setType(juce::dsp::LinkwitzRileyFilterType::allpass);

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

	LP1.prepare(spec);
	HP1.prepare(spec);

	AP2.prepare(spec);

	LP2.prepare(spec);
	HP2.prepare(spec);


	invAP1.prepare(spec);
	invAP2.prepare(spec);

	invAPBuffer.setSize(spec.numChannels, samplesPerBlock);
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

	//updateDistortionSettings(apvts);
	const auto params = GetParams();
	distortion.updateDistortionSettings();
	distortion.bypassed = apvts.getRawParameterValue("Low Bypass")->load();
	distortion.process(buffer);

	for (auto& fb : filterBuffers)
	{
		fb = buffer;
	}

	invAPBuffer = buffer;

	auto lowMidCutoffFreq = lowMidCrossover->get();
	LP1.setCutoffFrequency(lowMidCutoffFreq);
	HP1.setCutoffFrequency(lowMidCutoffFreq);
	invAP1.setCutoffFrequency(lowMidCutoffFreq);

	auto midHighCutoffFreq = midHighCrossover->get();

	AP2.setCutoffFrequency(midHighCutoffFreq);
	LP2.setCutoffFrequency(midHighCutoffFreq);
	HP2.setCutoffFrequency(midHighCutoffFreq);
	invAP2.setCutoffFrequency(midHighCutoffFreq);

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

	auto invAPBlock = juce::dsp::AudioBlock<float>(invAPBuffer);
	auto invAPCtx = juce::dsp::ProcessContextReplacing<float>(invAPBlock);

	invAP1.process(invAPCtx);
	invAP2.process(invAPCtx);

	auto numSamples = buffer.getNumSamples();
	auto numChannels = buffer.getNumChannels();
	//if (distortion.bypassed)
	//{
	//	return;
	//}
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

	if (distortion.bypassed)
	{
		for (auto ch = 0; ch < numChannels; ++ch)
		{
			juce::FloatVectorOperations::multiply(invAPBuffer.getWritePointer(ch), -1.f, numSamples);
		}
		addFilterBand(buffer, invAPBuffer);
	}

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

//void MBDistortionAudioProcessor::updateDistortionSettings(juce::AudioProcessorValueTreeState& apvts)
//{
//	using namespace Params;
//	const auto& params = GetParams();
//
//	auto floatHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName)
//	{
//		auto* parameter = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(params.at(paramName)));
//		jassert(parameter != nullptr);
//	};
//
//	floatHelper(distortion.inputGainInDecibels, Names::InputGain_Low_Band);
//	floatHelper(distortion.drive, Names::Distortion_Low_Band);
//	floatHelper(distortion.outputGainInDecibels, Names::OutputGain_Low_Band);
//	floatHelper(lowCrossover, Names::Low_MidLow_Freq);
//
//	LP.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
//	HP.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
//}


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
	layout.add(std::make_unique <AudioParameterBool>(params.at(Names::Bypassed_Low_Band), params.at(Names::Bypassed_Low_Band), false));

	layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Low_Mid_Freq), params.at(Names::Low_Mid_Freq), NormalisableRange<float>(20, 999, 1, 1), 400));
	layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Mid_High_Freq), params.at(Names::Mid_High_Freq), NormalisableRange<float>(1000, 20000, 1, 1), 2000));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new MBDistortionAudioProcessor();
}
