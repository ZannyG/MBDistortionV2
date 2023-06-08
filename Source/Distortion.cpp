#pragma once
//Baue helped
#include "Distortion.h"

Distortion::Distortion()
{

}
Distortion::~Distortion()
{

}

void Distortion::Prepare(const juce::dsp::ProcessSpec& spec)
{
	processorChain.prepare(spec);
}
void Distortion::Process(const ProcessContext& context) noexcept
{

}
void Distortion::Reset() noexcept
{
	processorChain.reset();
}



