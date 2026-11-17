#include "PluginEditor.h"

namespace
{
const juce::Colour bg(0xff111419), panel(0xff20252c), panel2(0xff2b3139);
const juce::Colour silver(0xffc8d0d8), acid(0xffb8f35b), orange(0xffff8a38), purple(0xffa98bff);
}

TruckStationAudioProcessorEditor::TruckStationAudioProcessorEditor(TruckStationAudioProcessor& p)
 : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(900, 650);
    title.setText("TRUCK STATION", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(30.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId, silver);
    addAndMakeVisible(title);
    subtitle.setText("SOUND SYSTEM 002  //  SYNTHESIS ONLY  //  NO SAMPLES", juce::dontSendNotification);
    subtitle.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    subtitle.setColour(juce::Label::textColourId, acid);
    addAndMakeVisible(subtitle);
    status.setText("TRUCK CAT: LISTENING", juce::dontSendNotification);
    status.setColour(juce::Label::textColourId, acid);
    addAndMakeVisible(status);

    auto add = [this](const juce::String& name, const juce::String& id, int x, int y)
    {
        auto s = std::make_unique<juce::Slider>();
        s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
        s->setColour(juce::Slider::rotarySliderFillColourId, acid);
        s->setColour(juce::Slider::thumbColourId, orange);
        s->setColour(juce::Slider::rotarySliderOutlineColourId, 0xff4c535c);
        s->setColour(juce::Slider::textBoxTextColourId, silver);
        s->setColour(juce::Slider::textBoxBackgroundColourId, bg);
        s->setColour(juce::Slider::textBoxOutlineColourId, panel2);
        addAndMakeVisible(*s);
        auto l = std::make_unique<juce::Label>();
        l->setText(name, juce::dontSendNotification);
        l->setJustificationType(juce::Justification::centred);
        l->setColour(juce::Label::textColourId, silver);
        l->setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::bold)));
        addAndMakeVisible(*l);
        attachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.parameters, id, *s));
        sliders.add(s.release()); labels.add(l.release());
    };

    // The first page presents the essential performance controls.
    add("ATTACK","attack",0,0); add("DECAY","decay",0,0);
    add("SUSTAIN","sustain",0,0); add("RELEASE","release",0,0);
    add("CUTOFF","cutoff",0,0); add("RESONANCE","resonance",0,0);
    add("WAVEFORM","waveform",0,0); add("FM","fm",0,0);
    add("SUB","sub",0,0); add("DETUNE","detune",0,0);
    add("DRIVE","drive",0,0); add("LFO RATE","lfoSpeed",0,0);
    add("LFO AMOUNT","lfoAmount",0,0); add("WT SHAPE","wavetable",0,0);
    add("REVERB","reverb",0,0); add("CHORUS","chorus",0,0);
    add("OUTPUT","output",0,0); add("CHAOS","chaos",0,0);

    for (auto* b : { &mutateButton, &chaosButton, &randomButton })
    {
        b->setColour(juce::TextButton::buttonColourId, panel2);
        b->setColour(juce::TextButton::textColourOffId, acid);
        b->setColour(juce::TextButton::buttonOnColourId, orange);
        addAndMakeVisible(*b);
    }
    mutateButton.onClick = [this] { audioProcessor.randomizeSound(0.25f); };
    chaosButton.onClick = [this]
    {
        auto* p = audioProcessor.parameters.getParameter("chaos");
        const float now = audioProcessor.parameters.getRawParameterValue("chaos")->load();
        p->setValueNotifyingHost(p->convertTo0to1(now > 0.05f ? 0.0f : 0.75f));
    };
    randomButton.onClick = [this] { audioProcessor.randomizeSound(1.0f); };
    startTimerHz(24);
}

TruckStationAudioProcessorEditor::~TruckStationAudioProcessorEditor() { stopTimer(); }

void TruckStationAudioProcessorEditor::timerCallback()
{
    activity = juce::jmax(audioProcessor.getOutputActivity(), activity * 0.88f);
    ++blink;
    status.setText(activity > 0.015f ? "TRUCK CAT: HEARING SOMETHING..." : "TRUCK CAT: WAITING FOR NOISE", juce::dontSendNotification);
    repaint();
}

void TruckStationAudioProcessorEditor::drawCat(juce::Graphics& g, juce::Rectangle<float> r, float energy)
{
    auto cx = r.getCentreX(), cy = r.getCentreY();
    float wobble = std::sin((float)blink * 0.12f) * 2.0f + energy * 12.0f;
    juce::Path body;
    body.startNewSubPath(cx-48,cy+45); body.quadraticTo(cx-63,cy-5,cx-30,cy-39);
    body.lineTo(cx-38,cy-75); body.lineTo(cx-8,cy-54);
    body.quadraticTo(cx+18,cy-64,cx+37,cy-43);
    body.lineTo(cx+52,cy-68); body.lineTo(cx+57,cy-15);
    body.quadraticTo(cx+68,cy+45,cx+20,cy+58);
    body.quadraticTo(cx-25,cy+68,cx-48,cy+45);
    g.setColour(juce::Colour(0xff9b83ae)); g.fillPath(body);
    g.setColour(juce::Colour(0xffd8c6e5)); g.fillEllipse(cx-31,cy-37,13,25);
    g.fillEllipse(cx+27,cy-35,12,24);
    g.setColour(juce::Colour(0xff111419));
    g.fillEllipse(cx-20-wobble*0.15f,cy-4,8,12);
    g.fillEllipse(cx+21+wobble*0.15f,cy-6,8,12);
    g.setColour(orange);
    juce::Path nose; nose.addTriangle(cx-4,cy+18,cx+5,cy+18,cx,cy+25); g.fillPath(nose);
    g.setColour(juce::Colour(0xff302538));
    g.drawLine(cx,cy+25,cx-13,cy+34,2.0f); g.drawLine(cx,cy+25,cx+14,cy+33,2.0f);
    if (energy > 0.08f)
    {
        g.setColour(acid.withAlpha(0.65f));
        g.drawEllipse(cx-67-energy*15,cy-84-energy*12,134+energy*30,165+energy*24,2.0f);
    }
}

void TruckStationAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    g.setColour(panel2);
    for (int x=0;x<getWidth();x+=24) g.drawVerticalLine(x,0.0f,(float)getHeight());
    for (int y=0;y<getHeight();y+=24) g.drawHorizontalLine(y,0.0f,(float)getWidth());
    g.setColour(silver); g.drawRoundedRectangle(12,12,(float)getWidth()-24,(float)getHeight()-24,12,1.2f);
    g.setColour(acid); g.drawLine(24,91,(float)getWidth()-24,91,2.0f);
    auto catArea = juce::Rectangle<float>((float)getWidth()-190,125,150,170);
    g.setColour(panel); g.fillRoundedRectangle(catArea.expanded(10),8);
    g.setColour(orange); g.drawRoundedRectangle(catArea.expanded(10),8,1.0f);
    drawCat(g,catArea,activity);
    g.setColour(silver);
    g.setFont(juce::Font(juce::FontOptions(10.0f,juce::Font::bold)));
    g.drawFittedText("TRUCK CAT // AUDIO REACTIVE", (int)catArea.getX()-10, 302, 170, 18, juce::Justification::centred, 1);
    g.setColour(acid.withAlpha(0.8f));
    for (int i=0;i<9;++i)
    {
        float a = (float)i/8.0f;
        g.fillEllipse(35.0f+i*13.0f, getHeight()-36.0f, 5.0f, 5.0f + activity*18.0f*a);
    }
    g.setColour(orange);
    g.setFont(juce::Font(juce::FontOptions(9.0f,juce::Font::bold)));
    g.drawText("Y2K TRIBAL SYNTHESIS UNIT  //  BUILD 002",24,getHeight()-22,getWidth()-48,12,juce::Justification::centredLeft);
}

void TruckStationAudioProcessorEditor::resized()
{
    title.setBounds(28,20,430,42);
    subtitle.setBounds(30,62,500,20);
    status.setBounds(getWidth()-260,60,230,20);
    auto area = juce::Rectangle<int>(24, 112, getWidth()-48, getHeight()-205);
    area.removeFromRight(185);
    const int cols=4, rows=5, cellW=area.getWidth()/cols, cellH=area.getHeight()/rows;
    for (int i=0;i<sliders.size();++i)
    {
        int row=i/cols, col=i%cols;
        auto x=area.getX()+col*cellW, y=area.getY()+row*cellH;
        labels[i]->setBounds(x,y,cellW,18);
        sliders[i]->setBounds(x+4,y+17,cellW-8,cellH-18);
    }
    auto buttonY=getHeight()-68;
    mutateButton.setBounds(30,buttonY,110,30);
    chaosButton.setBounds(150,buttonY,110,30);
    randomButton.setBounds(270,buttonY,130,30);
}
