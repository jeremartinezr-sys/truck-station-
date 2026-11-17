#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class TruckStationAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit TruckStationAudioProcessorEditor(TruckStationAudioProcessor&);
    ~TruckStationAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    TruckStationAudioProcessor& audioProcessor;
    juce::OwnedArray<juce::Slider> sliders;
    juce::OwnedArray<juce::Label> labels;
    std::vector<std::unique_ptr<SliderAttachment>> attachments;
    juce::TextButton mutateButton { "MUTATE" }, chaosButton { "CHAOS" }, randomButton { "RANDOMIZE" };
    juce::Label title, subtitle, status;
    float activity = 0.0f;
    int blink = 0;

    void addKnob(juce::Component&, const juce::String&, const juce::String&, int, int, int, int);
    void timerCallback() override;
    void drawCat(juce::Graphics&, juce::Rectangle<float>, float);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TruckStationAudioProcessorEditor)
};
