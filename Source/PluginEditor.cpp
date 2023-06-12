/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
template<typename T>
bool truncateKiloValue(T& value)
{
	if (value > static_cast<T>(999))
	{
		value /= static_cast<T>(1000);
		return true;
	}
	return false;
}

juce::String getValString(const juce::RangedAudioParameter& param, bool getLow, juce::String suffix)
{
	juce::String str;
	auto val = getLow ? param.getNormalisableRange().start : param.getNormalisableRange().end;
	bool useK = truncateKiloValue(val);
	str << val;
	if (useK)
	{
		str << "k";
	}
	str << suffix;

	return str;
}
void LookAndFeel::drawRotarySlider(juce::Graphics& g,
	int x,
	int y,
	int width,
	int height,
	float sliderPosProportional,
	float rotaryStartAngle,
	float rotaryEndAngle,
	juce::Slider& slider)
{
	using namespace juce;

	auto bounds = Rectangle<float>(x, y, width, height);

	auto enabled = slider.isEnabled();

	g.setColour(enabled ? Colour(97u, 18u, 167u) : Colours::darkgrey);
	g.fillEllipse(bounds);

	g.setColour(enabled ? Colour(255u, 154u, 1u) : Colours::grey);
	g.drawEllipse(bounds, 1.f);

	if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
	{
		auto center = bounds.getCentre();
		Path p;

		Rectangle<float> r;
		r.setLeft(center.getX() - 2);
		r.setRight(center.getX() + 2);
		r.setTop(bounds.getY());
		r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);

		p.addRoundedRectangle(r, 2.f);

		jassert(rotaryStartAngle < rotaryEndAngle);

		auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

		p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

		g.fillPath(p);

		g.setFont(rswl->getTextHeight());
		auto text = rswl->getDisplayString();
		auto strWidth = g.getCurrentFont().getStringWidth(text);

		r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
		r.setCentre(bounds.getCentre());

		g.setColour(enabled ? Colours::black : Colours::darkgrey);
		g.fillRect(r);

		g.setColour(enabled ? Colours::white : Colours::lightgrey);
		g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
	}
}

void LookAndFeel::drawToggleButton(juce::Graphics& g,
	juce::ToggleButton& toggleButton,
	bool shouldDrawButtonAsHighlighted,
	bool shouldDrawButtonAsDown)
{
	using namespace juce;

	if (auto* pb = dynamic_cast<PowerButton*>(&toggleButton))
	{
		Path powerButton;

		auto bounds = toggleButton.getLocalBounds();

		auto size = jmin(bounds.getWidth(), bounds.getHeight()) - 6;
		auto r = bounds.withSizeKeepingCentre(size, size).toFloat();

		float ang = 30.f; //30.f;

		size -= 6;

		powerButton.addCentredArc(r.getCentreX(),
			r.getCentreY(),
			size * 0.5,
			size * 0.5,
			0.f,
			degreesToRadians(ang),
			degreesToRadians(360.f - ang),
			true);

		powerButton.startNewSubPath(r.getCentreX(), r.getY());
		powerButton.lineTo(r.getCentre());

		PathStrokeType pst(2.f, PathStrokeType::JointStyle::curved);

		auto color = toggleButton.getToggleState() ? Colours::dimgrey : Colour(0u, 172u, 1u);

		g.setColour(color);
		g.strokePath(powerButton, pst);
		g.drawEllipse(r, 2);
	}
	else if (auto* analyzerButton = dynamic_cast<AnalyzerButton*>(&toggleButton))
	{
		auto color = !toggleButton.getToggleState() ? Colours::dimgrey : Colour(0u, 172u, 1u);

		g.setColour(color);

		auto bounds = toggleButton.getLocalBounds();
		g.drawRect(bounds);

		g.strokePath(analyzerButton->randomPath, PathStrokeType(1.f));
	}
	else
	{
		auto bounds = toggleButton.getLocalBounds().reduced(2);

		auto buttonIsOn = toggleButton.getToggleState();

		const int cornerSize = 4;

		g.setColour(buttonIsOn ? juce::Colours::white : juce::Colours::black);
		g.fillRoundedRectangle(bounds.toFloat(), cornerSize);

		g.setColour(buttonIsOn ? juce::Colours::black : juce::Colours::white);

		g.drawRoundedRectangle(bounds.toFloat(), cornerSize, 1);
		g.drawFittedText(toggleButton.getName(), bounds, juce::Justification::centred, 1);
	}

}
//==============================================================================
void RotarySliderWithLabels::paint(juce::Graphics& g)
{
	using namespace juce;

	auto startAng = degreesToRadians(180.f + 45.f);
	auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

	auto range = getRange();

	auto sliderBounds = getSliderBounds();

	auto bounds = getLocalBounds();

	g.setColour(Colours::blueviolet);
	g.drawFittedText(getName(), bounds.removeFromTop(getTextHeight() + 2), Justification::centredBottom, 1);

	getLookAndFeel().drawRotarySlider(g,
		sliderBounds.getX(),
		sliderBounds.getY(),
		sliderBounds.getWidth(),
		sliderBounds.getHeight(),
		jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
		startAng,
		endAng,
		*this);

	auto center = sliderBounds.toFloat().getCentre();
	auto radius = sliderBounds.getWidth() * 0.5f;

	g.setColour(Colour(0u, 172u, 1u));
	g.setFont(getTextHeight());

	auto numChoices = labels.size();
	for (int i = 0; i < numChoices; ++i)
	{
		auto pos = labels[i].pos;
		jassert(0.f <= pos);
		jassert(pos <= 1.f);

		auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);

		auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);

		Rectangle<float> r;
		auto str = labels[i].label;
		r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
		r.setCentre(c);
		r.setY(r.getY() + getTextHeight());

		g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
	}

}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
	auto bounds = getLocalBounds();
	bounds.removeFromTop(getTextHeight() * 1.5);

	auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

	size -= getTextHeight() * 1.5;
	juce::Rectangle<int> r;
	r.setSize(size, size);
	r.setCentre(bounds.getCentreX(), 0);
	r.setY(bounds.getY());

	return r;

}

juce::String RotarySliderWithLabels::getDisplayString() const
{
	if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
		return choiceParam->getCurrentChoiceName();

	juce::String str;
	bool addK = false;

	if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
	{
		float val = getValue();

		//if (val > 999.f)
		//{
		//    val /= 1000.f; //1001 / 1000 = 1.001
		//    addK = true;
		//}
		addK = truncateKiloValue(val);
		str = juce::String(val, (addK ? 2 : 0));
	}
	else
	{
		jassertfalse;
	}

	if (suffix.isNotEmpty())
	{
		str << " ";
		if (addK)
			str << "k";

		str << suffix;
	}

	return str;
}

void RotarySliderWithLabels::changeParam(juce::RangedAudioParameter* p)
{
	param = p;
	repaint();
}
//==============================================================================



//==============================================================================
Placeholder::Placeholder()
{

	juce::Random r;
	customColor = juce::Colour(r.nextInt(255), r.nextInt(255), r.nextInt(255));
}
//==============================================================================
DistortionBandControls::DistortionBandControls(juce::AudioProcessorValueTreeState& apv) :
	apvts(apv),
	inputGainSlider(nullptr, "dB", "INPUT"),
	distortionSlider(nullptr, "%", "DRIVE"),
	outputGainSlider(nullptr, "dB", "OUTPUT")
{
	/*using namespace Params;
	const auto& params = GetParams();*/

	/*auto getParamHelper = [&params, &apvts = this-> apvts](const auto& name) -> auto&
	{
		return getParam(apvts, params, name);
	};

	inputGainSlider.changeParam(&getParamHelper(Names::InputGain_Mid_Band));
	outputGainSlider.changeParam(&getParamHelper(Names::OutputGain_Mid_Band));
	distortionSlider.changeParam(&getParamHelper(Names::Distortion_Mid_Band));

	addLabelPairs(inputGainSlider.labels, getParamHelper(Names::InputGain_Mid_Band), "dB");
	addLabelPairs(distortionSlider.labels, getParamHelper(Names::Distortion_Mid_Band), "%");
	addLabelPairs(inputGainSlider.labels, getParamHelper(Names::OutputGain_Mid_Band), "dB");*/

	//auto makeAttachmentHelper = [&params, &apvts = this-> apvts](auto& attachment, const auto& name, auto& slider) {
	//	makeAttachment(attachment, apvts, params, name, slider);
	//};

	//makeAttachmentHelper(inputGainSliderAttachment, Names::InputGain_Mid_Band, inputGainSlider);
	//makeAttachmentHelper(outputGainSliderAttachment, Names::OutputGain_Mid_Band, outputGainSlider);
	//makeAttachmentHelper(distortionSliderAttachment, Names::Distortion_Mid_Band, distortionSlider);
	addAndMakeVisible(inputGainSlider);
	addAndMakeVisible(outputGainSlider);
	addAndMakeVisible(distortionSlider);

	bypassButton.addListener(this);

	bypassButton.setName("X");

	addAndMakeVisible(bypassButton);

	//makeAttachmentHelper(bypassButtonAttachment, Names::Bypassed_Mid_Band, bypassButton);

	lowBandButton.setName("Low");
	midBandButton.setName("Mid");
	highBandButton.setName("High");

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
		//auto endCap = FlexItem().withWidth(6);

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
	//auto endCap = FlexItem().withWidth(6);

	FlexBox flexBox;
	flexBox.flexDirection = FlexBox::Direction::row;
	flexBox.flexWrap = FlexBox::Wrap::noWrap;

	flexBox.items.add(spacer);
	//flexBox.items.add(endCap);
	flexBox.items.add(FlexItem(bandSelectControlBox).withWidth(50));
	flexBox.items.add(spacer);
	flexBox.items.add(FlexItem(inputGainSlider).withFlex(1.f));
	flexBox.items.add(spacer);
	flexBox.items.add(FlexItem(distortionSlider).withFlex(1.f));
	flexBox.items.add(spacer);
	flexBox.items.add(FlexItem(outputGainSlider).withFlex(1.f));
	flexBox.items.add(spacer);
	//flexBox.items.add(endCap);
	flexBox.items.add(FlexItem(bandButtonControlBox).withWidth(30));
	flexBox.performLayout(bounds);
}

void drawModuleBackground(juce::Graphics& g, juce::Rectangle<int> bounds)
{
	using namespace juce;
	g.setColour(Colours::blueviolet);
	g.fillAll();
	auto localBound = bounds;
	bounds.reduce(3, 3);
	g.setColour(Colours::black);
	g.fillRoundedRectangle(bounds.toFloat(), 3);
}

void DistortionBandControls::paint(juce::Graphics& g)
{
	auto bounds = getLocalBounds();
	drawModuleBackground(g, bounds);
}

void DistortionBandControls::buttonClicked(juce::Button* button)
{
	updateSliderEnablements();
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

GlobalControls::GlobalControls(juce::AudioProcessorValueTreeState& apvts)
{
	using namespace Params;
	const auto& params = GetParams();

	auto getParamHelper = [&params, &apvts](const auto& name) -> auto&
	{
		return getParam(apvts, params, name);
	};

	auto& lowMidParam = getParamHelper(Names::Low_Mid_Crossover_Freq);
	auto& midHighParam = getParamHelper(Names::Mid_High_Crossover_Freq);

	lowMidXoverSlider = std::make_unique<RSWL>(&lowMidParam, "Hz", "LOW_MID X-OVER");
	midHighXoverSlider = std::make_unique<RSWL>(&midHighParam, "Hz", "MID_HIGH X-OVER");

	auto makeAttachmentHelper = [&params, &apvts](auto& attachment, const auto& name, auto& slider) {
		makeAttachment(attachment, apvts, params, name, slider);
	};

	makeAttachmentHelper(lowMidXoverSliderAttachment, Names::Low_Mid_Crossover_Freq, *lowMidXoverSlider);
	makeAttachmentHelper(midHighXoverSliderAttachment, Names::Mid_High_Crossover_Freq, *midHighXoverSlider);

	addLabelPairs(lowMidXoverSlider->labels, lowMidParam, "Hz");
	addLabelPairs(midHighXoverSlider->labels, midHighParam, "Hz");

	addAndMakeVisible(*lowMidXoverSlider);
	addAndMakeVisible(*midHighXoverSlider);
}
void GlobalControls::paint(juce::Graphics& g)
{
	auto bounds = getLocalBounds();
	drawModuleBackground(g, bounds);
}

void GlobalControls::resized()
{
	using namespace juce;
	auto bounds = getLocalBounds().reduced(5);

	auto spacer = FlexItem().withWidth(4);
	auto endCap = FlexItem().withWidth(6);

	FlexBox flexBox;
	flexBox.flexDirection = FlexBox::Direction::row;
	flexBox.flexWrap = FlexBox::Wrap::noWrap;

	flexBox.items.add(endCap);
	flexBox.items.add(FlexItem(*lowMidXoverSlider).withFlex(1.f));
	flexBox.items.add(spacer);
	flexBox.items.add(FlexItem(*midHighXoverSlider).withFlex(1.f));
	flexBox.items.add(endCap);

	flexBox.performLayout(bounds);
}
//==============================================================================

MBDistortionAudioProcessorEditor::MBDistortionAudioProcessorEditor(MBDistortionAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	// Make sure that before the constructor has finished, you've set the
	// editor's size to whatever you need it to be.
	setLookAndFeel(&lnf);
	//addAndMakeVisible(controlBar);
	//addAndMakeVisible(analyzer);
	addAndMakeVisible(globalControls);
	addAndMakeVisible(bandControls);

	setSize(650, 550);
}

MBDistortionAudioProcessorEditor::~MBDistortionAudioProcessorEditor()
{
	setLookAndFeel(nullptr);
}

//==============================================================================
void MBDistortionAudioProcessorEditor::paint(juce::Graphics& g)
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)

	g.fillAll(juce::Colours::black);
}

void MBDistortionAudioProcessorEditor::resized()
{
	// This is generally where you'll want to lay out the positions of any
	// subcomponents in your editor..

	auto bounds = getLocalBounds();

	controlBar.setBounds(bounds.removeFromTop(32));

	bandControls.setBounds(bounds.removeFromBottom(135));

	analyzer.setBounds(bounds.removeFromTop(255));
	globalControls.setBounds(bounds);
}
