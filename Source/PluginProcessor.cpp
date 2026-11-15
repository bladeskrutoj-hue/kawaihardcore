#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

using Range = juce::NormalisableRange<float>;

juce::AudioProcessorValueTreeState::ParameterLayout KawaiiProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto addF = [&layout] (const char* id, const char* name, Range r, float def)
    {
        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { id, 1 }, name, r, def));
    };

    addF ("sawLvl",   "Supersaw",   { 0.0f, 1.0f }, 0.8f);
    addF ("pulseLvl", "Pulse",      { 0.0f, 1.0f }, 0.0f);
    addF ("sineLvl",  "Sine",       { 0.0f, 1.0f }, 0.0f);
    layout.add (std::make_unique<juce::AudioParameterInt> (juce::ParameterID { "unison", 1 }, "Unison", 1, 7, 5));
    addF ("detune",   "Detune",     { 0.0f, 1.0f }, 0.4f);
    addF ("width",    "Width",      { 0.0f, 1.0f }, 0.7f);

    addF ("cutoff",   "Cutoff",     { 20.0f, 20000.0f, 0.0f, 0.3f }, 8000.0f);
    addF ("reso",     "Reso",       { 0.0f, 1.0f }, 0.15f);
    addF ("fenv",     "Env Amount", { -1.0f, 1.0f }, 0.3f);
    addF ("fdecay",   "Env Decay",  { 0.02f, 2.0f, 0.0f, 0.4f }, 0.3f);

    addF ("aA",       "Attack",     { 0.001f, 2.0f, 0.0f, 0.4f }, 0.003f);
    addF ("aD",       "Decay",      { 0.01f, 3.0f, 0.0f, 0.4f }, 0.3f);
    addF ("aS",       "Sustain",    { 0.0f, 1.0f }, 0.8f);
    addF ("aR",       "Release",    { 0.005f, 3.0f, 0.0f, 0.4f }, 0.15f);

    addF ("pitchAmt", "Pitch Drop", { 0.0f, 48.0f }, 0.0f);
    addF ("pitchDec", "Drop Time",  { 0.005f, 1.0f, 0.0f, 0.4f }, 0.08f);

    addF ("drive",    "Drive",      { 0.0f, 1.0f }, 0.1f);
    addF ("gain",     "Volume",     { 0.0f, 1.0f }, 0.7f);

    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "gate", 1 }, "Trance Gate", false));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "gateRate", 1 }, "Gate Rate",
                                                              juce::StringArray { "1/8", "1/16", "1/32" }, 1));
    addF ("gateDuty", "Gate Length", { 0.1f, 0.9f }, 0.5f);

    return layout;
}

KawaiiProcessor::KawaiiProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    vp.bind (apvts);
    synth.addSound (new HardcoreSound());
    for (int i = 0; i < 12; ++i)
        synth.addVoice (new HardcoreVoice (vp));
}

void KawaiiProcessor::prepareToPlay (double sampleRate, int)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    gateEnv = 1.0f;
    fallbackPpq = 0.0;
}

bool KawaiiProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void KawaiiProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int numCh = buffer.getNumChannels();
    buffer.clear();

    synth.renderNextBlock (buffer, midi, 0, numSamples);

    const double sr = getSampleRate();

    // ---- trance gate (synced to host tempo) ----
    if (apvts.getRawParameterValue ("gate")->load() > 0.5f)
    {
        double bpm = 140.0, ppq = 0.0;
        bool havePpq = false;

        if (auto* ph = getPlayHead())
            if (auto pos = ph->getPosition())
            {
                if (auto b = pos->getBpm())
                    bpm = *b;
                if (pos->getIsPlaying())
                    if (auto q = pos->getPpqPosition())
                    {
                        ppq = *q;
                        havePpq = true;
                    }
            }

        static const double stepsPerBeat[] = { 2.0, 4.0, 8.0 };
        const double steps = stepsPerBeat[juce::jlimit (0, 2, (int) std::round (apvts.getRawParameterValue ("gateRate")->load()))];
        const double duty = apvts.getRawParameterValue ("gateDuty")->load();
        const double ppqPerSample = bpm / 60.0 / sr;

        for (int i = 0; i < numSamples; ++i)
        {
            const double pos = havePpq ? ppq + i * ppqPerSample : fallbackPpq + i * ppqPerSample;
            const double ph = pos * steps - std::floor (pos * steps);
            const float target = (ph < duty) ? 1.0f : 0.0f;
            gateEnv += (target - gateEnv) * 0.02f;
            for (int c = 0; c < numCh; ++c)
                buffer.getWritePointer (c)[i] *= gateEnv;
        }
        fallbackPpq += numSamples * ppqPerSample;
    }

    // ---- drive (tanh saturation) + master volume ----
    const float drive = apvts.getRawParameterValue ("drive")->load();
    const float gain  = apvts.getRawParameterValue ("gain")->load();
    const float pre   = 1.0f + drive * 20.0f;
    const float post  = gain / (1.0f + drive * 0.5f);

    for (int c = 0; c < numCh; ++c)
    {
        auto* d = buffer.getWritePointer (c);
        for (int i = 0; i < numSamples; ++i)
            d[i] = std::tanh (d[i] * pre) * post;
    }
}

juce::AudioProcessorEditor* KawaiiProcessor::createEditor()
{
    return new KawaiiEditor (*this);
}

int KawaiiProcessor::getNumPrograms() { return (int) getPresets().size(); }

const juce::String KawaiiProcessor::getProgramName (int index)
{
    const auto& ps = getPresets();
    return juce::isPositiveAndBelow (index, (int) ps.size()) ? juce::String (ps[(size_t) index].name) : juce::String();
}

void KawaiiProcessor::setCurrentProgram (int index)
{
    const auto& ps = getPresets();
    if (! juce::isPositiveAndBelow (index, (int) ps.size()))
        return;
    currentPreset = index;

    for (auto* prm : getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (prm))
            rp->setValueNotifyingHost (rp->getDefaultValue());

    for (auto& kv : ps[(size_t) index].values)
        if (auto* rp = apvts.getParameter (kv.first))
            rp->setValueNotifyingHost (rp->convertTo0to1 (kv.second));
}

void KawaiiProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}

void KawaiiProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KawaiiProcessor();
}
