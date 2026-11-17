#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class TruckStationAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          private juce::Timer
{
public:
    explicit TruckStationAudioProcessorEditor(TruckStationAudioProcessor&);
    ~TruckStationAudioProcessorEditor() override { stopTimer(); }
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    void addKnob(juce::Slider&, const juce::String&);
    void applyAttachments();

    TruckStationAudioProcessor& audioProcessor;
    juce::Slider attack, release, color, dust, space, wobble, drive, sampleMix;
    juce::ComboBox presets, wave;
    juce::TextButton loadSample { "LOAD SAMPLE +" };
    juce::Label sampleName;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachments[8];
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveAttachment;
    juce::Random random;
    float catPhase = 0.0f;
    bool blink = false;
    std::unique_ptr<juce::FileChooser> chooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TruckStationAudioProcessorEditor)
};
