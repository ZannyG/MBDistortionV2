/*
  ==============================================================================

    DistortionBandControls.cpp
    Created: 12 Jun 2023 2:00:12pm
    Author:  xande

  ==============================================================================
*/

#include "DistortionBandControls.h"
#include "Utilities.h"
#include "Params.h"
#include "LookAndFeel.h"

DistortionBandControls::DistortionBandControls(juce::AudioProcessorValueTreeState& apv) :
	apvts(apv),
	inputGainSlider(nullptr, "dB", "INPUT"),
	distortionSlider(nullptr, "%", "DRIVE"),
	outputGainSlider(nullptr, "dB", "OUTPUT")
{
	addAndMakeVisible(inputGainSlider);
	addAndMakeVisible(outputGainSlider);
	addAndMakeVisible(distortionSlider);

	bypassButton.addListener(this);

	bypassButton.setName("X");
	bypassButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::red);
	bypassButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

	addAndMakeVisible(bypassButton);

	lowBandButton.setName("Low");
	lowBandButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::limegreen);
	lowBandButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

	midBandButton.setName("Mid");
	midBandButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::limegreen);
	midBandButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

	highBandButton.setName("High");
	highBandButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::limegreen);
	highBandButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

	lowBandButton.setRadioGroupId(1);
	midBandButton.setRadioGroupId(1);
	highBandButton.setRadioGroupId(1);

	auto buttonSwitcher = [safePtr = this->safePtr]()
	{
		if (auto* c = safePtr.getComponent())
		{
			c->updateAttachments();
		}
	};

	lowBandButton.onClick = buttonSwitcher;
	midBandButton.onClick = buttonSwitcher;
	highBandButton.onClick = buttonSwitcher;

	lowBandButton.setToggleState(true, juce::NotificationType::dontSendNotification);

	updateAttachments();
	updateSliderEnablements();
	updateBandSelectButtonStates();

	addAndMakeVisible(lowBandButton);
	addAndMakeVisible(midBandButton);
	addAndMakeVisible(highBandButton);
}

DistortionBandControls::~DistortionBandControls()
{
	bypassButton.removeListener(this);

}

void DistortionBandControls::resized()
{
	using namespace juce;
	auto bounds = getLocalBounds().reduced(5);

	auto createBandButtonControlBox = [](std::vector<Component*> comps)
	{
		FlexBox flexBox;
		flexBox.flexDirection = FlexBox::Direction::column;
		flexBox.flexWrap = FlexBox::Wrap::noWrap;

		auto spacer = FlexItem().withHeight(2);

		for (auto* comp : comps)
		{
			flexBox.items.add(spacer);
			flexBox.items.add(FlexItem(*comp).withFlex(1.f));
		}
		flexBox.items.add(spacer);
		return flexBox;
	};

	auto bandButtonControlBox = createBandButtonControlBox({ &bypassButton });
	auto bandSelectControlBox = createBandButtonControlBox({ &lowBandButton, &midBandButton, &highBandButton });
	auto spacer = FlexItem().withWidth(4);

	FlexBox flexBox;
	flexBox.flexDirection = FlexBox::Direction::row;
	flexBox.flexWrap = FlexBox::Wrap::noWrap;

	flexBox.items.add(spacer);
	flexBox.items.add(FlexItem(bandSelectControlBox).withWidth(50));
	flexBox.items.add(spacer);
	flexBox.items.add(FlexItem(inputGainSlider).withFlex(1.f));
	flexBox.items.add(spacer);
	flexBox.items.add(FlexItem(distortionSlider).withFlex(1.f));
	flexBox.items.add(spacer);
	flexBox.items.add(FlexItem(outputGainSlider).withFlex(1.f));
	flexBox.items.add(spacer);
	flexBox.items.add(FlexItem(bandButtonControlBox).withWidth(30));
	flexBox.performLayout(bounds);
}


void DistortionBandControls::paint(juce::Graphics& g)
{
	auto bounds = getLocalBounds();
	drawModuleBackground(g, bounds);
}

void DistortionBandControls::buttonClicked(juce::Button* button)
{
	updateSliderEnablements();
	updateActiveBandFillColors(*button);
}

void DistortionBandControls::updateActiveBandFillColors(juce::Button& clickedButton)
{
	jassert(activeBand != nullptr);
	if (clickedButton.getToggleState() == false)
	{
		resetActiveBandColors();
	}
	else
	{
		refreshBandButtonColors(*activeBand, clickedButton);
	}
}

void DistortionBandControls::refreshBandButtonColors(juce::Button& band, juce::Button& colorSource)
{
	band.setColour(juce::TextButton::ColourIds::buttonOnColourId, colorSource.findColour(juce::TextButton::ColourIds::buttonOnColourId));

	band.setColour(juce::TextButton::ColourIds::buttonColourId, colorSource.findColour(juce::TextButton::ColourIds::buttonOnColourId));
	band.repaint();
}

void DistortionBandControls::resetActiveBandColors()
{
	activeBand->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::limegreen);
	activeBand->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
	activeBand->repaint();
}

void DistortionBandControls::updateBandSelectButtonStates()
{
	using namespace Params;

	std::vector<std::array<Names, 1>> paramsToCheck
	{
		{Names::Bypassed_Low_Band},
		{ Names::Bypassed_Mid_Band },
		{ Names::Bypassed_High_Band },
	};
	const auto& params = GetParams();
	auto paramHelper = [&params, this](const auto& name)
	{
		return dynamic_cast<juce::AudioParameterBool*>(&getParam(apvts,params,name));
	};
	for (size_t i = 0; i < paramsToCheck.size(); ++i)
	{
		auto& list = paramsToCheck[i];

		auto* bandButton = (i == 0) ? &lowBandButton : i == 1 ? &midBandButton : &highBandButton;

		if (auto * bypassed = paramHelper(list[0]);
			bypassed->get())
		{
			refreshBandButtonColors(*bandButton, bypassButton);
		}
	}
}

void DistortionBandControls::updateSliderEnablements()
{
	auto disabled = bypassButton.getToggleState();
	inputGainSlider.setEnabled(!disabled);
	distortionSlider.setEnabled(!disabled);
	outputGainSlider.setEnabled(!disabled);

}

void DistortionBandControls::updateAttachments()
{
	enum BandType
	{
		Low,
		Mid,
		High
	};

	BandType bandType = [this]()
	{
		if (lowBandButton.getToggleState())
		{
			return BandType::Low;
		}
		if (midBandButton.getToggleState())
		{
			return BandType::Mid;
		}

		return BandType::High;
	}();

	using namespace Params;
	std::vector<Names> names;

	switch (bandType)
	{
	case Low:
	{
		names = std::vector<Names>
		{
			Names::InputGain_Low_Band,
			Names::Distortion_Low_Band,
			Names::OutputGain_Low_Band,
			Names::Bypassed_Low_Band,
		};
		activeBand = &lowBandButton;
		break;
	}
	case Mid:
	{
		names = std::vector<Names>
		{
			Names::InputGain_Mid_Band,
			Names::Distortion_Mid_Band,
			Names::OutputGain_Mid_Band,
			Names::Bypassed_Mid_Band,
		};
		activeBand = &midBandButton;

		break;

	}
	case High:
	{
		names = std::vector<Names>
		{
			Names::InputGain_High_Band,
			Names::Distortion_High_Band,
			Names::OutputGain_High_Band,
			Names::Bypassed_High_Band,
		};
		activeBand = &highBandButton;

		break;
	}
	}
	enum Pos
	{
		InputGain,
		Distortion,
		OutputGain,
		Bypass
	};

	const auto& params = GetParams();
	auto getParamHelper = [&params, &apvts = this-> apvts, &names](const auto& pos) -> auto&
	{

		return getParam(apvts, params, names.at(pos));
	};

	inputGainSliderAttachment.reset();
	outputGainSliderAttachment.reset();
	distortionSliderAttachment.reset();
	bypassButtonAttachment.reset();

	auto& inputGainParam = getParamHelper(Pos::InputGain);
	addLabelPairs(inputGainSlider.labels, inputGainParam, "dB");
	inputGainSlider.changeParam(&inputGainParam);

	auto& distortionParam = getParamHelper(Pos::Distortion);
	addLabelPairs(distortionSlider.labels, distortionParam, "%");
	distortionSlider.changeParam(&distortionParam);

	auto& outputGainParam = getParamHelper(Pos::OutputGain);
	addLabelPairs(outputGainSlider.labels, outputGainParam, "dB");
	outputGainSlider.changeParam(&outputGainParam);

	auto makeAttachmentHelper = [&params, &apvts = this->apvts](auto& attachment, const auto& name, auto& slider) {
		makeAttachment(attachment, apvts, params, name, slider);
	};

	makeAttachmentHelper(inputGainSliderAttachment, names[Pos::InputGain], inputGainSlider);
	makeAttachmentHelper(distortionSliderAttachment, names[Pos::Distortion], distortionSlider);
	makeAttachmentHelper(outputGainSliderAttachment, names[Pos::OutputGain], outputGainSlider);
	makeAttachmentHelper(bypassButtonAttachment, names[Pos::Bypass], bypassButton);

}