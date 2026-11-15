#pragma once
#include "PluginProcessor.h"

// ---------- cute 2000s-anime look: pink / lavender / sky gradients, glossy knobs ----------
class KawaiiLookAndFeel : public juce::LookAndFeel_V4
{
public:
    KawaiiLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
};

class Knob : public juce::Component
{
public:
    Knob (juce::AudioProcessorValueTreeState& state, const juce::String& paramId, const juce::String& text);
    void resized() override;

private:
    juce::Slider slider;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

class KawaiiEditor : public juce::AudioProcessorEditor
{
public:
    explicit KawaiiEditor (KawaiiProcessor&);
    ~KawaiiEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Panel { juce::String title; juce::Rectangle<int> area; };

    void drawFace (juce::Graphics&, juce::Rectangle<float>) const;
    void stepPreset (int delta);

    KawaiiProcessor& proc;
    KawaiiLookAndFeel laf;

    Knob sawK, pulseK, sineK, uniK, detK, widK,
         cutK, resK, fenvK, fdecK,
         aK, dK, sK, rK,
         pAmtK, pDecK,
         drvK, gainK,
         dutyK;

    juce::ComboBox presetBox, rateBox;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" };
    juce::ToggleButton gateBtn { "Trance Gate" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   gateAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> rateAtt;

    std::vector<Panel> panels;
    struct Star { float x, y, size, alpha; };
    std::vector<Star> stars;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KawaiiEditor)
};
