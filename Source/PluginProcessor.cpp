#include "PluginProcessor.h"
#include "PluginEditor.h"

TruckStationAudioProcessor::TruckStationAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    formatManager.registerBasicFormats();
    synth.setNoteStealingEnabled(true);
    synth.addSound(new TruckSound());
    for (int i = 0; i < 8; ++i)
        synth.addVoice(new TruckVoice(*this));
    // Separate sampler voices are required for loaded SamplerSound layers.
    for (int i = 0; i < 8; ++i)
        synth.addVoice(new juce::SamplerVoice());
}

juce::AudioProcessorValueTreeState::ParameterLayout
TruckStationAudioProcessor::createParameterLayout()
{
    using F = juce::AudioParameterFloat;
    using C = juce::AudioParameterChoice;
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    p.add(std::make_unique<C>("wave", "WAVE", juce::StringArray{"SINE", "TRI", "SAW", "SQUARE"}, 2));
    p.add(std::make_unique<F>("attack", "ATTACK", juce::NormalisableRange<float>(0.001f, 2.0f, 0.001f, 0.35f), 0.02f));
    p.add(std::make_unique<F>("release", "RELEASE", juce::NormalisableRange<float>(0.02f, 4.0f, 0.001f, 0.4f), 0.45f));
    p.add(std::make_unique<F>("color", "COLOR", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f));
    p.add(std::make_unique<F>("dust", "DUST", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
    p.add(std::make_unique<F>("space", "SPACE", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.15f));
    p.add(std::make_unique<F>("wobble", "WOBBLE", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
    p.add(std::make_unique<F>("drive", "DRIVE", juce::NormalisableRange<float>(0.0f, 18.0f, 0.01f), 0.0f));
    p.add(std::make_unique<F>("sampleMix", "SAMPLE MIX", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.7f));
    return p;
}

TruckStationAudioProcessor::TruckVoice::TruckVoice(TruckStationAudioProcessor& owner)
    : processor(owner)
{
}

bool TruckStationAudioProcessor::TruckVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<TruckSound*>(sound) != nullptr;
}

void TruckStationAudioProcessor::TruckVoice::startNote(int midiNote, float velocity,
                                                        juce::SynthesiserSound*, int)
{
    const auto sr = getSampleRate() > 0.0 ? getSampleRate() : 44100.0;
    const float semitones = (float)midiNote - 69.0f;
    const float color = processor.parameters.getRawParameterValue("color")->load();
    const float wobble = processor.parameters.getRawParameterValue("wobble")->load();
    const float cents = (color - 0.5f) * 18.0f;
    const float hz = 440.0f * std::pow(2.0f, (semitones + cents / 100.0f) / 12.0f);
    phaseDelta = juce::MathConstants<double>::twoPi * hz / sr;
    phase = 0.0;
    lfoPhase = 0.0;
    level = velocity;
    waveform = (int)processor.parameters.getRawParameterValue("wave")->load();
    envParams.attack = processor.parameters.getRawParameterValue("attack")->load();
    envParams.decay = 0.12f;
    envParams.sustain = 0.82f;
    envParams.release = processor.parameters.getRawParameterValue("release")->load();
    envelope.setSampleRate(sr);
    envelope.setParameters(envParams);
    envelope.noteOn();
    juce::ignoreUnused(wobble);
}

void TruckStationAudioProcessor::TruckVoice::stopNote(float, bool allowTailOff)
{
    if (allowTailOff)
        envelope.noteOff();
    else
    {
        envelope.reset();
        clearCurrentNote();
    }
}

void TruckStationAudioProcessor::TruckVoice::renderNextBlock(juce::AudioBuffer<float>& output,
                                                              int startSample, int numSamples)
{
    if (!isVoiceActive())
        return;

    const float wobble = processor.parameters.getRawParameterValue("wobble")->load();
    const float dust = processor.parameters.getRawParameterValue("dust")->load();
    const float color = processor.parameters.getRawParameterValue("color")->load();
    const float wobbleRate = 3.0f + wobble * 5.0f;
    const double sr = getSampleRate() > 0.0 ? getSampleRate() : 44100.0;

    for (int i = 0; i < numSamples; ++i)
    {
        const float env = envelope.getNextSample();
        const double lfo = std::sin(lfoPhase);
        lfoPhase += juce::MathConstants<double>::twoPi * wobbleRate / sr;
        if (lfoPhase > juce::MathConstants<double>::twoPi)
            lfoPhase -= juce::MathConstants<double>::twoPi;
        const double ph = phase + lfo * wobble * 0.035;
        float s = 0.0f;
        switch (waveform)
        {
            case 0: s = (float)std::sin(ph); break;
            case 1: s = (float)(2.0 / juce::MathConstants<double>::pi
                              * std::asin(std::sin(ph))); break;
            case 2: s = (float)(2.0 * (ph / juce::MathConstants<double>::twoPi
                              - std::floor(0.5 + ph / juce::MathConstants<double>::twoPi))); break;
            default: s = std::sin(ph) >= 0.0 ? 1.0f : -1.0f; break;
        }
        const float grit = 1.0f - dust * 0.25f;
        s = std::round(s * (32.0f + (1.0f - dust) * 2048.0f))
          / (32.0f + (1.0f - dust) * 2048.0f);
        s *= (0.65f + color * 0.45f) * grit * level * env * 0.18f;

        for (int ch = 0; ch < output.getNumChannels(); ++ch)
            output.addSample(ch, startSample + i, s);

        phase += phaseDelta;
        if (phase > juce::MathConstants<double>::twoPi)
            phase -= juce::MathConstants<double>::twoPi;
    }

    if (!envelope.isActive())
        clearCurrentNote();
}

void TruckStationAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    synth.setCurrentPlaybackSampleRate(sampleRate);
    juce::dsp::Reverb::Parameters rp;
    rp.roomSize = 0.55f;
    rp.damping = 0.45f;
    rp.wetLevel = 0.0f;
    rp.dryLevel = 1.0f;
    reverb.setParameters(rp);
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 };
    reverb.prepare(spec);
    reverb.reset();
}

void TruckStationAudioProcessor::releaseResources() {}

bool TruckStationAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void TruckStationAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    synth.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());

    const float driveDb = parameters.getRawParameterValue("drive")->load();
    const float drive = juce::Decibels::decibelsToGain(driveDb);
    const float dust = parameters.getRawParameterValue("dust")->load();
    const float space = parameters.getRawParameterValue("space")->load();
    const float mix = parameters.getRawParameterValue("sampleMix")->load();
    const float gain = 0.75f + mix * 0.25f;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float x = std::tanh(data[i] * drive) * gain;
            if (dust > 0.001f)
            {
                const float levels = 256.0f - dust * 248.0f;
                x = std::round(x * levels) / levels;
            }
            data[i] = x;
        }
    }

    if (buffer.getNumChannels() == 2 && space > 0.001f)
    {
        juce::dsp::Reverb::Parameters rp;
        rp.roomSize = 0.35f + space * 0.6f;
        rp.damping = 0.7f - space * 0.35f;
        rp.wetLevel = space * 0.45f;
        rp.dryLevel = 1.0f - space * 0.15f;
        reverb.setParameters(rp);
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        reverb.process(context);
    }
}

bool TruckStationAudioProcessor::loadSample(const juce::File& file)
{
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader == nullptr || reader->lengthInSamples <= 0)
        return false;

    // Quita el sample anterior (si habia) para no acumular capas.
    for (int i = synth.getNumSounds(); --i >= 0;)
        if (dynamic_cast<juce::SamplerSound*>(synth.getSound(i).get()) != nullptr)
            synth.removeSound(i);

    juce::BigInteger notes;
    notes.setRange(0, 128, true);
    auto* sound = new juce::SamplerSound(file.getFileName(), *reader, notes, 60, 0.01, 0.15, 20.0);
    // Keep the built-in synth sound and add this sample as a playable layer.
    synth.addSound(sound);
    sampleName = file.getFileName();
    return true;
}

juce::String TruckStationAudioProcessor::getSampleName() const { return sampleName; }

void TruckStationAudioProcessor::setPreset(int index)
{
    struct P { int wave; float attack, release, color, dust, space, wobble, drive, sampleMix; };
    static const P presets[] = {
        {2,.02f,.45f,.50f,.05f,.15f,.08f,1.0f,.7f}, {0,.04f,1.2f,.72f,.02f,.55f,.12f,0,.8f},
        {1,.01f,.35f,.30f,.22f,.10f,.55f,3,.5f}, {3,.005f,.22f,.85f,.50f,.05f,.10f,8,.6f},
        {2,.15f,2.5f,.45f,.10f,.80f,.25f,1,.9f}, {0,.3f,3.0f,.25f,.05f,.95f,.18f,0,.9f},
        {1,.01f,.18f,.65f,.65f,.08f,.80f,12,.5f}, {3,.001f,.10f,.95f,.8f,.02f,.30f,15,.4f},
        {2,.06f,1.8f,.38f,.18f,.45f,.70f,4,.8f}, {0,.5f,3.5f,.18f,.02f,1.0f,.10f,0,.9f}
    };
    const int count = (int)(sizeof(presets) / sizeof(presets[0]));
    const auto& p = presets[juce::jlimit(0, count - 1, index % count)];
    auto set = [this](const juce::String& id, float v)
    {
        if (auto* param = parameters.getParameter(id))
            param->setValueNotifyingHost(param->convertTo0to1(v));
    };
    set("wave", (float)p.wave); set("attack", p.attack); set("release", p.release);
    set("color", p.color); set("dust", p.dust); set("space", p.space);
    set("wobble", p.wobble); set("drive", p.drive); set("sampleMix", p.sampleMix);
}

void TruckStationAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void TruckStationAudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* TruckStationAudioProcessor::createEditor()
{
    return new TruckStationAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TruckStationAudioProcessor();
}
