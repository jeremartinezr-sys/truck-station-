#include "PluginEditor.h"

TruckStationAudioProcessorEditor::TruckStationAudioProcessorEditor(TruckStationAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(820, 590);
    addKnob(attack, "ATTACK"); addKnob(release, "RELEASE");
    addKnob(color, "COLOR"); addKnob(dust, "DUST");
    addKnob(space, "SPACE"); addKnob(wobble, "WOBBLE");
    addKnob(drive, "DRIVE"); addKnob(sampleMix, "SAMPLE MIX");

    const juce::StringArray names {
        "01 - Sleepy Keys", "02 - Bubblegum Pad", "03 - Dusty Pluck",
        "04 - Broken Toy", "05 - Cloud Window", "06 - Rainy Internet",
        "07 - Pixel Soup", "08 - Tiny Robot", "09 - Tape Ghost",
        "10 - Plastic Thunder", "11 - Jelly Bass", "12 - Dream Printer",
        "13 - VHS Melt", "14 - Wobbly Room", "15 - Alien Gum",
        "16 - Basement FM", "17 - Soft Arcade", "18 - Ghost Frequency",
        "19 - Rubber Lead", "20 - Cat Nap", "21 - Digital Puddle",
        "22 - Rusty Bells", "23 - Night Bus", "24 - Weird Angel",
        "25 - Crushed Candy", "26 - Melted Arcade", "27 - Toybox Choir",
        "28 - Cloudy Keys", "29 - Plastic Dreams", "30 - Random Toybox"
    };
    presets.addItemList(names, 1);
    presets.setSelectedId(1);
    presets.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xfffff8e9));
    presets.setColour(juce::ComboBox::textColourId, juce::Colour(0xff493d59));
    presets.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xffc8b2d0));
    presets.onChange = [this] { audioProcessor.setPreset(presets.getSelectedId() - 1); };
    addAndMakeVisible(presets);

    addAndMakeVisible(loadSample);
    loadSample.setColour(juce::TextButton::buttonColourId, juce::Colour(0xfff3c978));
    loadSample.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff493d59));
    loadSample.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser>("Choose a sample", juce::File{}, "*.wav;*.aif;*.aiff;*.flac;*.ogg");
        chooser->launchAsync(juce::FileBrowserComponent::openMode
                           | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                const auto file = fc.getResult();
                if (file.existsAsFile() && audioProcessor.loadSample(file))
                    sampleName.setText(audioProcessor.getSampleName(), juce::dontSendNotification);
            });
    };

    addAndMakeVisible(sampleName);
    sampleName.setText(audioProcessor.getSampleName(), juce::dontSendNotification);
    sampleName.setColour(juce::Label::textColourId, juce::Colour(0xff51455d));
    sampleName.setJustificationType(juce::Justification::centredLeft);
    sampleName.setFont(juce::Font(13.0f, juce::Font::bold));

    wave.addItemList(juce::StringArray{"SINE", "TRI", "SAW", "SQUARE"}, 1);
    wave.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xfffff8e9));
    wave.setColour(juce::ComboBox::textColourId, juce::Colour(0xff493d59));
    wave.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xffc8b2d0));
    addAndMakeVisible(wave);
    waveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.parameters, "wave", wave);

    applyAttachments();
    startTimerHz(12);
}

void TruckStationAudioProcessorEditor::addKnob(juce::Slider& s, const juce::String& name)
{
    addAndMakeVisible(s);
    s.setName(name);
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 66, 19);
    s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff9a75c5));
    s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xffd8c6df));
    s.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff0b96e));
    s.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xff493d59));
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x00000000));
}

void TruckStationAudioProcessorEditor::applyAttachments()
{
    const char* ids[] = {"attack","release","color","dust","space","wobble","drive","sampleMix"};
    juce::Slider* sliders[] = {&attack,&release,&color,&dust,&space,&wobble,&drive,&sampleMix};
    for (int i = 0; i < 8; ++i)
        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, ids[i], *sliders[i]);
}

void TruckStationAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xfffff0d8));
    g.setColour(juce::Colour(0xffe7d7ed));
    g.fillRoundedRectangle(16, 16, 788, 558, 25);
    g.setColour(juce::Colour(0xffb99acb));
    g.drawRoundedRectangle(16, 16, 788, 558, 25, 3);

    g.setColour(juce::Colour(0xff51415f));
    g.setFont(juce::Font("Arial Rounded MT Bold", 30.0f, juce::Font::bold));
    g.drawText("TRUCK STATION", 38, 28, 430, 40, juce::Justification::left);
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText("PLASTIC SOUND LIBRARY  •  INSTRUMENT BUILD 001", 40, 68, 420, 18, juce::Justification::left);

    g.setColour(juce::Colour(0xfffff8e9));
    g.fillRoundedRectangle(38, 100, 540, 74, 15);
    g.setColour(juce::Colour(0xffc8b2d0));
    g.drawRoundedRectangle(38, 100, 540, 74, 15, 2);
    g.setColour(juce::Colour(0xff51415f));
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText("PRESET LIBRARY", 54, 106, 180, 20, juce::Justification::left);

    // Clay cat mascot, drawn directly in JUCE and gently bobbing/blinking.
    const float bob = std::sin(catPhase) * 3.0f;
    g.setColour(juce::Colour(0xfffff8e9));
    g.fillRoundedRectangle(602, 100, 180, 220, 18);
    g.setColour(juce::Colour(0xffc8b2d0));
    g.drawRoundedRectangle(602, 100, 180, 220, 18, 2);
    g.setColour(juce::Colour(0xffb7a0c9));
    g.fillEllipse(645, 151 + bob, 95, 92);
    juce::Path ears;
    ears.addTriangle(648, 181 + bob, 655, 133 + bob, 684, 166 + bob);
    ears.addTriangle(700, 166 + bob, 733, 135 + bob, 738, 185 + bob);
    g.fillPath(ears);
    g.setColour(juce::Colour(0xffe6c2d2));
    g.fillEllipse(658, 148 + bob, 12, 22);
    g.fillEllipse(716, 148 + bob, 12, 22);
    g.setColour(juce::Colour(0xff493d59));
    if (blink)
    {
        g.drawLine(670, 190 + bob, 685, 190 + bob, 3);
        g.drawLine(708, 190 + bob, 723, 190 + bob, 3);
    }
    else
    {
        g.fillEllipse(673, 184 + bob, 7, 11);
        g.fillEllipse(711, 184 + bob, 7, 11);
    }
    g.setColour(juce::Colour(0xffd98c9b));
    juce::Path nose;
    nose.addTriangle(691, 201 + bob, 699, 201 + bob, 695, 207 + bob);
    g.fillPath(nose);
    g.setColour(juce::Colour(0xff77627f));
    g.drawLine(695, 207 + bob, 695, 214 + bob, 1.5f);
    g.drawLine(695, 214 + bob, 687, 218 + bob, 1.5f);
    g.drawLine(695, 214 + bob, 703, 218 + bob, 1.5f);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText("mrrp... choose a sound", 611, 278, 162, 22, juce::Justification::centred);

    g.setColour(juce::Colour(0xff51415f));
    g.setFont(juce::Font(15.0f, juce::Font::bold));
    g.drawText("THE CLAY CONTROLS", 40, 205, 300, 24, juce::Justification::left);
    g.setFont(juce::Font(11.0f));
    g.drawText("Shape the sound. Break it. Make something new.", 40, 228, 440, 18, juce::Justification::left);
    g.drawText("KEYS  •  SYNTHS  •  BASS  •  TEXTURES  •  ODD TOYS", 40, 535, 550, 18, juce::Justification::left);
    g.drawText("TRUCK STATION AUDIO TOYS", 40, 552, 300, 14, juce::Justification::left);
}

void TruckStationAudioProcessorEditor::resized()
{
    presets.setBounds(54, 130, 500, 30);

    // Columna derecha, debajo del gato (antes el boton se superponia con los knobs).
    loadSample.setBounds(602, 335, 180, 36);
    sampleName.setBounds(602, 376, 180, 28);
    wave.setBounds(602, 414, 180, 30);

    const int y1 = 250, y2 = 385, w = 105, h = 125;
    attack.setBounds(35, y1, w, h);
    release.setBounds(145, y1, w, h);
    color.setBounds(255, y1, w, h);
    dust.setBounds(365, y1, w, h);
    space.setBounds(35, y2, w, h);
    wobble.setBounds(145, y2, w, h);
    drive.setBounds(255, y2, w, h);
    sampleMix.setBounds(365, y2, w, h);
}

void TruckStationAudioProcessorEditor::timerCallback()
{
    catPhase += 0.16f;
    blink = random.nextInt(26) == 0;
    repaint(625, 125, 140, 175);
}
