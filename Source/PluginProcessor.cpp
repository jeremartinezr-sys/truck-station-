#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
class TruckVoice : public juce::SynthesiserVoice
{
public:
    bool canPlaySound(juce::SynthesiserSound* s) override
    {
        return dynamic_cast<juce::SynthesiserSound*>(s) != nullptr;
    }

    void startNote(int midiNote, float velocity, juce::SynthesiserSound*, int) override
    {
        const auto hz = juce::MidiMessage::getMidiNoteInHertz(midiNote);
        frequency = hz;
        level = velocity * 0.3f;
        phase = 0.0;
        phase2 = 0.0;
        phase3 = 0.0;
        age = 0.0;
        releasing = false;
        env = 0.0f;
        const float a = attack->load(), d = decay->load(), s = sustain->load();
        attackSeconds = juce::jmax(0.001f, a);
        decaySeconds = juce::jmax(0.001f, d);
        sustainLevel = s;
    }

    void stopNote(float, bool allowTailOff) override
    {
        if (allowTailOff) releasing = true;
        else clearCurrentNote();
    }

    void pitchWheelMoved(int value) override { pitchBend = (value - 8192) / 8192.0f; }
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioBuffer<float>& output, int start, int count) override
    {
        if (!isVoiceActive()) return;
        const double sr = getSampleRate();
        const float rel = release->load();
        const float cutoffHz = cutoff->load();
        const float resonanceValue = resonance->load();
        const float shape = waveform->load();
        const float fmAmount = fm->load();
        const float sub = subLevel->load();
        const float det = detune->load();
        const float driveAmount = drive->load();
        const float lfoRate = lfoSpeed->load();
        const float lfoDepth = lfoAmount->load();
        const float wt = wavetable->load();

        for (int i = 0; i < count; ++i)
        {
            age += 1.0 / sr;
            if (!releasing)
            {
                if (age < attackSeconds) env = juce::jmin(1.0f, env + (float)(1.0 / (attackSeconds * sr)));
                else if (age < attackSeconds + decaySeconds)
                    env = juce::jmax(sustainLevel, env - (float)((1.0 - sustainLevel) / (decaySeconds * sr)));
                else env = sustainLevel;
            }
            else
            {
                env *= std::exp(-1.0f / (juce::jmax(0.02f, rel) * (float)sr));
                if (env < 0.0005f) { clearCurrentNote(); break; }
            }

            const double lfo = std::sin(2.0 * juce::MathConstants<double>::pi * lfoPhase);
            lfoPhase += lfoRate / sr;
            if (lfoPhase >= 1.0) lfoPhase -= 1.0;
            const double bend = std::pow(2.0, (pitchBend * 2.0 + det * 0.12 * std::sin(lfoPhase * 6.28318)) / 12.0);
            const double f = frequency * bend;
            const double inc1 = f / sr;
            const double inc2 = f * (1.0 + det * 0.01) / sr;
            const double inc3 = f * 0.5 / sr;

            const auto osc = [&](double p, float mode)
            {
                const double s = std::sin(2.0 * juce::MathConstants<double>::pi * p);
                const double saw = 2.0 * p - 1.0;
                const double square = p < 0.5 ? 1.0 : -1.0;
                const double tri = 1.0 - 4.0 * std::abs(p - 0.5);
                const double fmWave = std::sin(2.0 * juce::MathConstants<double>::pi * p + fmAmount * std::sin(2.0 * juce::MathConstants<double>::pi * p * 2.0));
                const double wav = std::sin(2.0 * juce::MathConstants<double>::pi * p * (1.0 + 5.0 * wt));
                const double modes[6] = { s, saw, square, tri, fmWave, wav };
                const int m = juce::jlimit(0, 5, (int)std::round(mode * 5.0f));
                return (float)modes[m];
            };

            float sample = osc(phase, shape);
            sample += 0.45f * osc(phase2, std::fmod(shape + 0.20f, 1.0f));
            sample += sub * 0.5f * (float)std::sin(2.0 * juce::MathConstants<double>::pi * phase3);
            sample *= env * level * (1.0f + (float)lfo * lfoDepth * 0.5f);
            sample = std::tanh(sample * (1.0f + driveAmount * 12.0f));
            sample *= 0.7f;

            for (int ch = 0; ch < output.getNumChannels(); ++ch)
                output.addSample(ch, start + i, sample);

            phase += inc1; phase -= std::floor(phase);
            phase2 += inc2; phase2 -= std::floor(phase2);
            phase3 += inc3; phase3 -= std::floor(phase3);
        }
    }

    void setParameters(std::atomic<float>* a, std::atomic<float>* d, std::atomic<float>* s,
                       std::atomic<float>* r, std::atomic<float>* c, std::atomic<float>* q,
                       std::atomic<float>* w, std::atomic<float>* f, std::atomic<float>* subP,
                       std::atomic<float>* detP, std::atomic<float>* drv, std::atomic<float>* lfs,
                       std::atomic<float>* lfa, std::atomic<float>* wt)
    {
        attack=a; decay=d; sustain=s; release=r; cutoff=c; resonance=q; waveform=w;
        fm=f; subLevel=subP; detune=detP; drive=drv; lfoSpeed=lfs; lfoAmount=lfa; wavetable=wt;
    }

private:
    double frequency=440.0, phase=0.0, phase2=0.0, phase3=0.0, age=0.0, lfoPhase=0.0;
    float level=0.0f, env=0.0f, sustainLevel=0.7f, attackSeconds=0.01f, decaySeconds=0.2f, pitchBend=0.0f;
    bool releasing=false;
    std::atomic<float>* attack=nullptr; std::atomic<float>* decay=nullptr; std::atomic<float>* sustain=nullptr;
    std::atomic<float>* release=nullptr; std::atomic<float>* cutoff=nullptr; std::atomic<float>* resonance=nullptr;
    std::atomic<float>* waveform=nullptr; std::atomic<float>* fm=nullptr; std::atomic<float>* subLevel=nullptr;
    std::atomic<float>* detune=nullptr; std::atomic<float>* drive=nullptr; std::atomic<float>* lfoSpeed=nullptr;
    std::atomic<float>* lfoAmount=nullptr; std::atomic<float>* wavetable=nullptr;
};

class TruckSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};
}

juce::AudioProcessorValueTreeState::ParameterLayout TruckStationAudioProcessor::createParameterLayout()
{
    using P = juce::AudioParameterFloat;
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    auto add = [&p](const char* id, const char* name, float lo, float hi, float def,
                    juce::String suffix = {}, float skew = 1.0f)
    {
        juce::NormalisableRange<float> range(lo, hi);
        if (skew < 1.0f) range.setSkewForCentre(lo + (hi - lo) * skew); // 'skew' = donde queda el centro del knob
        p.add(std::make_unique<P>(juce::ParameterID{id, 1}, name, range, def,
                                  juce::AudioParameterFloatAttributes().withLabel(suffix)));
    };
    add("attack","Attack",0.001f,5.0f,0.02f," s",0.1f);
    add("decay","Decay",0.001f,5.0f,0.35f," s",0.1f);
    add("sustain","Sustain",0.0f,1.0f,0.72f);
    add("release","Release",0.02f,8.0f,0.6f," s",0.1f);
    add("cutoff","Cutoff",30.0f,18000.0f,12000.0f," Hz",0.08f);
    add("resonance","Resonance",0.1f,1.0f,0.2f);
    add("waveform","Waveform",0.0f,1.0f,0.2f);
    add("fm","FM Amount",0.0f,1.0f,0.0f);
    add("sub","Sub Level",0.0f,1.0f,0.15f);
    add("detune","Detune",0.0f,1.0f,0.15f);
    add("drive","Drive",0.0f,1.0f,0.08f);
    add("lfoSpeed","LFO Rate",0.05f,20.0f,2.0f," Hz",0.15f);
    add("lfoAmount","LFO Amount",0.0f,1.0f,0.0f);
    add("wavetable","WT Shape",0.0f,1.0f,0.0f);
    add("reverb","Reverb",0.0f,1.0f,0.18f);
    add("chorus","Chorus",0.0f,1.0f,0.0f);
    add("output","Output",0.0f,1.0f,0.75f);
    add("chaos","Chaos",0.0f,1.0f,0.0f);
    add("arp","Arp Enable",0.0f,1.0f,0.0f);
    add("arpRate","Arp Rate",1.0f,16.0f,4.0f);
    return p;
}

TruckStationAudioProcessor::TruckStationAudioProcessor()
 : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
   parameters(*this, nullptr, "TRUCK_PARAMETERS", createParameterLayout())
{
    for (int i=0;i<12;++i)
    {
        auto* v = new TruckVoice();
        v->setParameters(parameters.getRawParameterValue("attack"), parameters.getRawParameterValue("decay"),
          parameters.getRawParameterValue("sustain"), parameters.getRawParameterValue("release"),
          parameters.getRawParameterValue("cutoff"), parameters.getRawParameterValue("resonance"),
          parameters.getRawParameterValue("waveform"), parameters.getRawParameterValue("fm"),
          parameters.getRawParameterValue("sub"), parameters.getRawParameterValue("detune"),
          parameters.getRawParameterValue("drive"), parameters.getRawParameterValue("lfoSpeed"),
          parameters.getRawParameterValue("lfoAmount"), parameters.getRawParameterValue("wavetable"));
        synth.addVoice(v);
    }
    synth.addSound(new TruckSound());
}

void TruckStationAudioProcessor::prepareToPlay(double sr, int block)
{
    currentSampleRate = sr;
    synth.setCurrentPlaybackSampleRate(sr);
    filterL.reset(); filterR.reset();
    filterL.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    filterR.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    filterL.prepare({sr,(juce::uint32)block,1});
    filterR.prepare({sr,(juce::uint32)block,1});
    const auto numCh = (juce::uint32) juce::jmax(1, getTotalNumOutputChannels());
    chorus.prepare({sr,(juce::uint32)block,numCh});
    chorus.setRate(0.25f); chorus.setDepth(0.25f); chorus.setCentreDelay(8.0f);
    reverb.setSampleRate(sr);
    reverb.reset();
    limiter.prepare({sr,(juce::uint32)block,numCh});
    limiter.setThreshold(-1.0f);
    limiter.setRelease(80.0f);
    synthBuffer.setSize(2, block);
}

void TruckStationAudioProcessor::releaseResources() {}
bool TruckStationAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono();
}

void TruckStationAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    synth.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());

    const float cutoffHz = parameters.getRawParameterValue("cutoff")->load();
    const float resonance = parameters.getRawParameterValue("resonance")->load();
    filterL.setCutoffFrequency(cutoffHz); filterR.setCutoffFrequency(cutoffHz);
    filterL.setResonance(resonance); filterR.setResonance(resonance);
    if (buffer.getNumChannels() >= 1)
    {
        juce::dsp::AudioBlock<float> block(buffer);
        if (buffer.getNumChannels() == 2)
        {
            auto left = block.getSingleChannelBlock(0);
            auto right = block.getSingleChannelBlock(1);
            juce::dsp::ProcessContextReplacing<float> cl(left), cr(right);
            filterL.process(cl); filterR.process(cr);
        }
        else
        {
            juce::dsp::ProcessContextReplacing<float> c(block);
            filterL.process(c);
        }
    }

    const float ch = parameters.getRawParameterValue("chorus")->load();
    chorus.setDepth(0.05f + ch * 0.8f);
    if (ch > 0.001f) { juce::dsp::AudioBlock<float> b(buffer); juce::dsp::ProcessContextReplacing<float> c(b); chorus.process(c); }

    juce::dsp::Reverb::Parameters rp;
    rp.roomSize = 0.15f + parameters.getRawParameterValue("reverb")->load() * 0.8f;
    rp.damping = 0.45f; rp.wetLevel = parameters.getRawParameterValue("reverb")->load() * 0.45f;
    rp.dryLevel = 1.0f - rp.wetLevel; rp.width = 1.0f;
    reverb.setParameters(rp);
    if (buffer.getNumChannels() > 1)
        reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());
    else
        reverb.processMono(buffer.getWritePointer(0), buffer.getNumSamples());

    const float out = parameters.getRawParameterValue("output")->load();
    const float chaos = parameters.getRawParameterValue("chaos")->load();
    for (int c=0;c<buffer.getNumChannels();++c)
        for (int i=0;i<buffer.getNumSamples();++i)
        {
            float x = buffer.getSample(c,i) * out;
            if (chaos > 0.001f) x = std::tanh(x * (1.0f + chaos * 18.0f));
            buffer.setSample(c,i,x);
        }

    juce::dsp::AudioBlock<float> limBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> limContext(limBlock);
    limiter.process(limContext);

    float peak = buffer.getMagnitude(0, buffer.getNumSamples());
    if (buffer.getNumChannels()>1) peak = juce::jmax(peak, buffer.getMagnitude(1, buffer.getNumSamples()));
    outputActivity.store(peak);

    // MIDI arpeggiator is intentionally a safe, predictable first implementation:
    // the host MIDI stream is passed through unchanged when the arp switch is off.
}

void TruckStationAudioProcessor::randomizeSound(float amount, int seed)
{
    if (seed >= 0) random.setSeed((juce::int64)seed);
    const float a = juce::jlimit(0.0f,1.0f,amount);
    const char* ids[] = {"waveform","fm","sub","detune","drive","cutoff","resonance","lfoSpeed","lfoAmount","wavetable","reverb","chorus"};
    for (auto* id : ids)
    {
        auto* param = parameters.getParameter(id);
        if (param == nullptr) continue;
        auto* raw = parameters.getRawParameterValue(id);
        const float current = raw->load();
        const float target = random.nextFloat();
        const float next = current + (target-current)*a;
        param->setValueNotifyingHost(param->convertTo0to1(next));
    }
}

void TruckStationAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}
void TruckStationAudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data,size));
    if (xml && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessorEditor* TruckStationAudioProcessor::createEditor() { return new TruckStationAudioProcessorEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new TruckStationAudioProcessor(); }
