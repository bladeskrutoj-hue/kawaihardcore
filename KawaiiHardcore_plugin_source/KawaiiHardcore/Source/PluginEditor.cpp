#include "PluginEditor.h"

namespace
{
    const juce::Colour kInk    (0xff7a2f6b);   // deep plum text
    const juce::Colour kPink   (0xffff6fb5);
    const juce::Colour kPinkLt (0xffffc2e2);
    const juce::Colour kCyan   (0xff52d6ff);
}

// ============================ Look & Feel ============================
KawaiiLookAndFeel::KawaiiLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId,       kInk);
    setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId,               kInk);
    setColour (juce::ComboBox::backgroundColourId,      juce::Colours::white);
    setColour (juce::ComboBox::textColourId,            kInk);
    setColour (juce::ComboBox::outlineColourId,         kPink);
    setColour (juce::ComboBox::arrowColourId,           kPink);
    setColour (juce::PopupMenu::backgroundColourId,     juce::Colour (0xffffeaf6));
    setColour (juce::PopupMenu::textColourId,           kInk);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, kPink);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::TextButton::buttonColourId,        kPink);
    setColour (juce::TextButton::textColourOffId,       juce::Colours::white);
    setColour (juce::ToggleButton::textColourId,        kInk);
}

void KawaiiLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                          float startAngle, float endAngle, juce::Slider&)
{
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (4.0f);
    const float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
    const auto c = b.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);

    juce::Path track;
    track.addCentredArc (c.x, c.y, r, r, 0.0f, startAngle, endAngle, true);
    g.setColour (juce::Colours::white.withAlpha (0.7f));
    g.strokePath (track, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path val;
    val.addCentredArc (c.x, c.y, r, r, 0.0f, startAngle, angle, true);
    g.setColour (kCyan);
    g.strokePath (val, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float kr = r * 0.74f;
    g.setGradientFill (juce::ColourGradient (kPinkLt, c.x, c.y - kr, kPink, c.x, c.y + kr, false));
    g.fillEllipse (c.x - kr, c.y - kr, kr * 2.0f, kr * 2.0f);
    g.setColour (juce::Colours::white);
    g.drawEllipse (c.x - kr, c.y - kr, kr * 2.0f, kr * 2.0f, 2.0f);

    g.setColour (juce::Colours::white.withAlpha (0.55f));                       // gloss highlight
    g.fillEllipse (c.x - kr * 0.62f, c.y - kr * 0.92f, kr * 1.24f, kr * 0.68f);

    juce::Path ptr;
    ptr.addRoundedRectangle (-2.0f, -kr + 2.0f, 4.0f, kr * 0.42f, 2.0f);
    g.setColour (juce::Colours::white);
    g.fillPath (ptr, juce::AffineTransform::rotation (angle).translated (c.x, c.y));
}

void KawaiiLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool, bool)
{
    auto r = b.getLocalBounds().toFloat().reduced (2.0f);
    auto box = r.removeFromLeft (r.getHeight());
    g.setColour (juce::Colours::white);
    g.fillRoundedRectangle (box, 6.0f);
    g.setColour (kPink);
    g.drawRoundedRectangle (box, 6.0f, 2.0f);

    if (b.getToggleState())
    {
        juce::Path star;
        star.addStar (box.getCentre(), 5, box.getWidth() * 0.18f, box.getWidth() * 0.42f);
        g.setColour (kPink);
        g.fillPath (star);
    }
    g.setColour (kInk);
    g.setFont (juce::Font (14.0f, juce::Font::bold));
    g.drawText (b.getButtonText(), r.withTrimmedLeft (6.0f), juce::Justification::centredLeft);
}

// ============================ Knob ============================
Knob::Knob (juce::AudioProcessorValueTreeState& state, const juce::String& paramId, const juce::String& text)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 16);
    slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
    addAndMakeVisible (slider);

    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::Font (13.0f, juce::Font::bold));
    addAndMakeVisible (label);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, paramId, slider);
}

void Knob::resized()
{
    auto r = getLocalBounds();
    label.setBounds (r.removeFromTop (16));
    slider.setBounds (r);
}

// ============================ Editor ============================
KawaiiEditor::KawaiiEditor (KawaiiProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      sawK   (p.apvts, "sawLvl",   "Supersaw"),
      pulseK (p.apvts, "pulseLvl", "Pulse"),
      sineK  (p.apvts, "sineLvl",  "Sine"),
      uniK   (p.apvts, "unison",   "Unison"),
      detK   (p.apvts, "detune",   "Detune"),
      widK   (p.apvts, "width",    "Width"),
      cutK   (p.apvts, "cutoff",   "Cutoff"),
      resK   (p.apvts, "reso",     "Reso"),
      fenvK  (p.apvts, "fenv",     "Env Amt"),
      fdecK  (p.apvts, "fdecay",   "Env Decay"),
      aK     (p.apvts, "aA",       "Attack"),
      dK     (p.apvts, "aD",       "Decay"),
      sK     (p.apvts, "aS",       "Sustain"),
      rK     (p.apvts, "aR",       "Release"),
      pAmtK  (p.apvts, "pitchAmt", "Pitch Drop"),
      pDecK  (p.apvts, "pitchDec", "Drop Time"),
      drvK   (p.apvts, "drive",    "Drive"),
      gainK  (p.apvts, "gain",     "Volume"),
      dutyK  (p.apvts, "gateDuty", "Gate Len")
{
    setLookAndFeel (&laf);
    setSize (1040, 480);

    for (auto* k : { &sawK, &pulseK, &sineK, &uniK, &detK, &widK, &cutK, &resK, &fenvK, &fdecK,
                     &aK, &dK, &sK, &rK, &pAmtK, &pDecK, &drvK, &gainK, &dutyK })
        addAndMakeVisible (*k);

    for (int i = 0; i < proc.getNumPrograms(); ++i)
        presetBox.addItem (proc.getProgramName (i), i + 1);
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this] { proc.setCurrentProgram (presetBox.getSelectedId() - 1); };
    addAndMakeVisible (presetBox);

    prevBtn.onClick = [this] { stepPreset (-1); };
    nextBtn.onClick = [this] { stepPreset (+1); };
    addAndMakeVisible (prevBtn);
    addAndMakeVisible (nextBtn);

    rateBox.addItemList ({ "1/8", "1/16", "1/32" }, 1);
    addAndMakeVisible (rateBox);
    addAndMakeVisible (gateBtn);
    gateAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>   (proc.apvts, "gate", gateBtn);
    rateAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "gateRate", rateBox);

    juce::Random rng (2000);   // twinkly stars, same every time
    for (int i = 0; i < 60; ++i)
        stars.push_back ({ rng.nextFloat() * 1040.0f, rng.nextFloat() * 480.0f,
                           3.0f + rng.nextFloat() * 7.0f, 0.25f + rng.nextFloat() * 0.5f });
}

KawaiiEditor::~KawaiiEditor()
{
    setLookAndFeel (nullptr);
}

void KawaiiEditor::stepPreset (int delta)
{
    const int n = proc.getNumPrograms();
    const int next = (presetBox.getSelectedId() - 1 + delta + n) % n;
    presetBox.setSelectedId (next + 1, juce::sendNotification);
}

void KawaiiEditor::resized()
{
    const int m = 12, gap = 10, top = 98, rowH = 168;
    panels.clear();

    auto placeKnobs = [] (juce::Rectangle<int> area, std::initializer_list<Knob*> ks)
    {
        auto inner = area.withTrimmedTop(26).reduced (6, 2);
        const int w = inner.getWidth() / (int) ks.size();
        for (auto* k : ks)
            k->setBounds (inner.removeFromLeft (w));
    };

    const int total = getWidth() - 2 * m;

    // row 1: OSC (6 knobs) | FILTER (4 knobs)
    {
        const int oscW = (total - gap) * 6 / 10;
        juce::Rectangle<int> osc (m, top, oscW, rowH), fil (m + oscW + gap, top, total - oscW - gap, rowH);
        panels.push_back ({ "OSCILLATORS", osc });
        panels.push_back ({ "FILTER", fil });
        placeKnobs (osc, { &sawK, &pulseK, &sineK, &uniK, &detK, &widK });
        placeKnobs (fil, { &cutK, &resK, &fenvK, &fdecK });
    }

    // row 2: AMP | PITCH | OUT | GATE
    {
        const int y = top + rowH + gap;
        const double wts[] = { 4.0, 2.0, 2.0, 2.4 };
        const double sum = 10.4;
        const int avail = total - 3 * gap;
        int x = m;
        juce::Rectangle<int> r[4];
        for (int i = 0; i < 4; ++i)
        {
            const int w = (i == 3) ? (m + total - x) : (int) (avail * wts[i] / sum);
            r[i] = { x, y, w, rowH };
            x += w + gap;
        }
        panels.push_back ({ "AMP ENVELOPE", r[0] });
        panels.push_back ({ "PITCH DROP / KICK", r[1] });
        panels.push_back ({ "OUTPUT", r[2] });
        panels.push_back ({ "TRANCE GATE", r[3] });
        placeKnobs (r[0], { &aK, &dK, &sK, &rK });
        placeKnobs (r[1], { &pAmtK, &pDecK });
        placeKnobs (r[2], { &drvK, &gainK });

        auto inner = r[3].withTrimmedTop (30).reduced (8, 2);
        auto left = inner.removeFromLeft (inner.getWidth() / 2);
        gateBtn.setBounds (left.removeFromTop (28).withTrimmedTop (2));
        left.removeFromTop (10);
        rateBox.setBounds (left.removeFromTop (28));
        dutyK.setBounds (inner);
    }

    // header: preset browser
    auto hdr = juce::Rectangle<int> (getWidth() - 360, 28, 340, 30);
    prevBtn.setBounds (hdr.removeFromLeft (30));
    nextBtn.setBounds (hdr.removeFromRight (30));
    presetBox.setBounds (hdr.reduced (6, 0));
}

void KawaiiEditor::drawFace (juce::Graphics& g, juce::Rectangle<float> r) const
{
    const auto c = r.getCentre();
    const float w = r.getWidth(), h = r.getHeight();

    g.setColour (juce::Colour (0xffff4fa3));                                    // hair back
    g.fillEllipse (r.expanded (5.0f));
    g.setColour (juce::Colour (0xffffe8d8));                                    // face
    g.fillEllipse (r.translated (0.0f, h * 0.06f).reduced (2.0f));

    g.setColour (juce::Colour (0xffff4fa3));                                    // bangs
    for (int i = 0; i < 4; ++i)
    {
        juce::Path t;
        const float x0 = r.getX() + w * (0.08f + 0.22f * i);
        t.addTriangle (x0, r.getY() + h * 0.05f, x0 + w * 0.30f, r.getY() + h * 0.05f, x0 + w * 0.12f, r.getY() + h * 0.42f);
        g.fillPath (t);
    }

    for (int s = -1; s <= 1; s += 2)                                            // eyes
    {
        const float ex = c.x + s * w * 0.22f, ey = c.y + h * 0.12f;
        g.setColour (juce::Colours::white);
        g.fillEllipse (ex - w * 0.11f, ey - h * 0.15f, w * 0.22f, h * 0.30f);
        g.setGradientFill (juce::ColourGradient (kCyan, ex, ey - h * 0.1f, juce::Colour (0xff3a5bd9), ex, ey + h * 0.14f, false));
        g.fillEllipse (ex - w * 0.085f, ey - h * 0.12f, w * 0.17f, h * 0.26f);
        g.setColour (juce::Colours::white);
        g.fillEllipse (ex - w * 0.05f, ey - h * 0.09f, w * 0.06f, h * 0.08f);
        g.fillEllipse (ex + w * 0.01f, ey + h * 0.03f, w * 0.03f, h * 0.04f);

        g.setColour (kPink.withAlpha (0.45f));                                  // blush
        g.fillEllipse (ex - w * 0.10f, ey + h * 0.17f, w * 0.20f, h * 0.09f);
    }

    juce::Path mouth;                                                           // smile
    mouth.addCentredArc (c.x, c.y + h * 0.30f, w * 0.07f, h * 0.05f, 0.0f,
                         juce::MathConstants<float>::pi * 0.6f, juce::MathConstants<float>::pi * 1.4f, true);
    g.setColour (kInk);
    g.strokePath (mouth, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void KawaiiEditor::paint (juce::Graphics& g)
{
    const float W = (float) getWidth(), H = (float) getHeight();

    juce::ColourGradient bg (juce::Colour (0xffffd1ec), 0.0f, 0.0f, juce::Colour (0xffa9e4ff), 0.0f, H, false);
    bg.addColour (0.5, juce::Colour (0xffe6c8ff));
    g.setGradientFill (bg);
    g.fillAll();

    g.setColour (juce::Colours::white.withAlpha (0.22f));                       // polka dots
    for (int row = 0; row * 30 < (int) H; ++row)
        for (int col = 0; col * 30 < (int) W; ++col)
            g.fillEllipse ((float) col * 30 + (row % 2 ? 15 : 0), (float) row * 30, 7.0f, 7.0f);

    for (auto& s : stars)                                                       // sparkles
    {
        juce::Path st;
        st.addStar ({ s.x, s.y }, 4, s.size * 0.28f, s.size);
        g.setColour (juce::Colours::white.withAlpha (s.alpha));
        g.fillPath (st);
    }

    // header ribbon
    juce::ColourGradient hg (juce::Colour (0xffff8fcb), 0.0f, 0.0f, juce::Colour (0xffff4fa3), 0.0f, 84.0f, false);
    g.setGradientFill (hg);
    g.fillRect (0.0f, 0.0f, W, 84.0f);
    g.setColour (juce::Colours::white.withAlpha (0.3f));
    g.fillRect (0.0f, 0.0f, W, 30.0f);
    g.setColour (juce::Colours::white);
    g.fillRect (0.0f, 84.0f, W, 3.0f);

    drawFace (g, { 18.0f, 10.0f, 62.0f, 62.0f });

    g.setColour (juce::Colour (0xff9b2b78));
    g.setFont (juce::Font (34.0f, juce::Font::bold | juce::Font::italic));
    g.drawText ("Kawaii Hardcore", 102, 11, 480, 40, juce::Justification::centredLeft);
    g.setColour (juce::Colours::white);
    g.drawText ("Kawaii Hardcore", 100, 9, 480, 40, juce::Justification::centredLeft);

    g.setFont (juce::Font (14.0f, juce::Font::bold));
    g.setColour (juce::Colours::white);
    g.drawText ("gabber  /  happy hardcore  /  old school  /  trance", 102, 50, 520, 22, juce::Justification::centredLeft);

    for (auto& p : panels)
    {
        auto a = p.area.toFloat();
        g.setColour (juce::Colours::white.withAlpha (0.58f));
        g.fillRoundedRectangle (a, 16.0f);
        g.setColour (kPink);
        g.drawRoundedRectangle (a.reduced (1.0f), 16.0f, 2.5f);
        g.setColour (kInk);
        g.setFont (juce::Font (13.0f, juce::Font::bold));
        g.drawText (p.title, p.area.withHeight (26).reduced (14, 0), juce::Justification::centredLeft);
    }
}
