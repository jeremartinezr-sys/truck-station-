#pragma once
#include <JuceHeader.h>

class TruckStationAudioProcessor : public juce::AudioProcessor
{
public:
    TruckStationAudioProcessor();
    ~TruckStationAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "TRUCK DEFAULT"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    void randomizeSound(float amount, int seed = -1);
    float getOutputActivity() const noexcept { return outputActivity.load(); }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::Synthesiser synth;
    juce::dsp::StateVariableTPTFilter<float> filterL, filterR;
    juce::Reverb reverb; // (el reverb de juce_dsp no tiene processStereo)
    juce::dsp::Chorus<float> chorus;
    juce::dsp::Limiter<float> limiter;
    juce::AudioBuffer<float> synthBuffer;
    std::atomic<float> outputActivity { 0.0f };
    double currentSampleRate = 44100.0;
    juce::Random random;
    int arpCounter = 0;
    int arpDivider = 0;
    juce::MidiBuffer heldMidi;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TruckStationAudioProcessor)
};
