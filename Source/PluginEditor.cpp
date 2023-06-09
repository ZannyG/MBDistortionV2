/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Params.h"
//==============================================================================

MBDistortionAudioProcessorEditor::MBDistortionAudioProcessorEditor(MBDistortionAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setLookAndFeel(&lnf);
    addAndMakeVisible(analyzer);
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
    using namespace juce;
    g.fillAll(juce::Colours::black);

    g.fillAll(Colours::black);

    Path curve;

    auto bounds = getLocalBounds();
    auto center = bounds.getCentre();

    g.setFont(Font("Iosevka Term Slab", 30, 0)); //https://github.com/be5invis/Iosevka

    String title{ "XG-MBD 1200" };
    g.setFont(30);
    auto titleWidth = g.getCurrentFont().getStringWidth(title);

    curve.startNewSubPath(center.x, 32);
    curve.lineTo(center.x - titleWidth * 0.45f, 32);

    auto cornerSize = 20;
    auto curvePos = curve.getCurrentPosition();
    curve.quadraticTo(curvePos.getX() - cornerSize, curvePos.getY(),
        curvePos.getX() - cornerSize, curvePos.getY() - 16);
    curvePos = curve.getCurrentPosition();
    curve.quadraticTo(curvePos.getX(), 2,
        curvePos.getX() - cornerSize, 2);

    curve.lineTo({ 0.f, 2.f });
    curve.lineTo(0.f, 0.f);
    curve.lineTo(center.x, 0.f);

    g.setColour(ColorScheme::getModuleBorderColor());
    g.fillPath(curve);
    g.setColour(ColorScheme::getModuleBorderColor());
    g.strokePath(curve, PathStrokeType(2));


    curve.applyTransform(AffineTransform().scaled(-1, 1));
    curve.applyTransform(AffineTransform().translated(getWidth(), 0));
    g.setColour(ColorScheme::getModuleBorderColor());
    g.fillPath(curve);
    g.setColour(ColorScheme::getModuleBorderColor());
    g.strokePath(curve, PathStrokeType(2));

    g.setColour(ColorScheme::getTitleColor());
    g.drawFittedText(title, bounds, juce::Justification::centredTop, 1);

}

void MBDistortionAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    controlBar.setBounds(bounds.removeFromTop(32));

    bandControls.setBounds(bounds.removeFromBottom(135));

    analyzer.setBounds(bounds.removeFromTop(255));
    globalControls.setBounds(bounds);
}
