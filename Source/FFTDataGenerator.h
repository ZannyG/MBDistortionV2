/*
  ==============================================================================

    FFTDataGenerator.h
    Created: 13 Jun 2023 4:09:00pm
    Author:  xande

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Utilities.h"
#include "Fifo.h"
template<typename BlockType>
struct FFTDataGenerator
{

    void produceFFTDataForRendering(const juce::AudioBuffer<float>& audioData, const float negativeInfinity)
    {
        const auto fftSize = getFFTSize();

        fftData.assign(fftData.size(), 0);
        auto* readIndex = audioData.getReadPointer(0);
        std::copy(readIndex, readIndex + fftSize, fftData.begin());

        // first apply a windowing function to our data
        window->multiplyWithWindowingTable(fftData.data(), fftSize);

        // then render our FFT data..
        forwardFFT->performFrequencyOnlyForwardTransform(fftData.data());

        int numBins = (int)fftSize / 2;

        //normalize the fft values.
        for (int i = 0; i < numBins; ++i)
        {
            auto v = fftData[i];

            if (!std::isinf(v) && !std::isnan(v))
            {
                v /= float(numBins);
            }
            else
            {
                v = 0.f;
            }
            fftData[i] = v;
        }

        for (int i = 0; i < numBins; ++i)
        {
            fftData[i] = juce::Decibels::gainToDecibels(fftData[i], negativeInfinity);
        }

        fftDataFifo.push(fftData);
    }

    void changeOrder(FFTOrder newOrder)
    {

        order = newOrder;
        auto fftSize = getFFTSize();

        forwardFFT = std::make_unique<juce::dsp::FFT>(order);
        window = std::make_unique<juce::dsp::WindowingFunction<float>>(fftSize, juce::dsp::WindowingFunction<float>::blackmanHarris);

        fftData.clear();
        fftData.resize(fftSize * 2, 0);

        fftDataFifo.prepare(fftData.size());
    }
    //==============================================================================
    int getFFTSize() const { return 1 << order; }
    int getNumAvailableFFTDataBlocks() const { return fftDataFifo.getNumAvailableForReading(); }
    //==============================================================================
    bool getFFTData(BlockType& fftData) { return fftDataFifo.pull(fftData); }
private:
    FFTOrder order;
    BlockType fftData;
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;

    Fifo<BlockType> fftDataFifo;
};
