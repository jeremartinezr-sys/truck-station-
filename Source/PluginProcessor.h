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

    const juce::String getName() const override { return "TRUCK STATION"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    bool loadSample(const juce::File&);
    juce::String getSampleName() const;
    void setPreset(int index);
    juce::AudioProcessorValueTreeState parameters;

private:
    class TruckSound : public juce::SynthesiserSound
    {
    public:
        bool appliesToNote(int) override { return true; }
        bool appliesToChannel(int) override { return true; }
    };

    class TruckVoice : public juce::SynthesiserVoice
    {
    public:
        explicit TruckVoice(TruckStationAudioProcessor& owner);
        bool canPlaySound(juce::SynthesiserSound*) override;
        void startNote(int midiNote, float velocity, juce::SynthesiserSound*, int) override;
        void stopNote(float velocity, bool allowTailOff) override;
        void pitchWheelMoved(int) override {}
        void controllerMoved(int, int) override {}
        void renderNextBlock(juce::AudioBuffer<float>&, int startSample, int numSamples) override;
    private:
        TruckStationAudioProcessor& processor;
        juce::ADSR envelope;
        juce::ADSR::Parameters envParams;
        double phase = 0.0;
        double phaseDelta = 0.0;
        double lfoPhase = 0.0;
        float level = 0.0f;
        int waveform = 0;
    };

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioFormatManager formatManager;
    juce::Synthesiser synth;
    juce::String sampleName { "NO SAMPLE LOADED" };
    juce::dsp::Reverb reverb;
    double currentSampleRate = 44100.0;
    juce::Random random;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TruckStationAudioProcessor)
};
