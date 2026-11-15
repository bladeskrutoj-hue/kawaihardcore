#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>

struct HardcoreSound : public juce::SynthesiserSound
{
    bool appliesToNote (int) override    { return true; }
    bool appliesToChannel (int) override { return true; }
};

// Pointers to the raw parameter values (read once per block / note).
struct VoiceParams
{
    std::atomic<float> *saw, *pulse, *sine, *unison, *detune, *width,
                       *cutoff, *reso, *fenv, *fdecay,
                       *attack, *decay, *sustain, *release,
                       *pitchAmt, *pitchDec;

    void bind (juce::AudioProcessorValueTreeState& s)
    {
        auto get = [&s] (const char* id) { return s.getRawParameterValue (id); };
        saw = get ("sawLvl");     pulse = get ("pulseLvl"); sine = get ("sineLvl");
        unison = get ("unison");  detune = get ("detune");  width = get ("width");
        cutoff = get ("cutoff");  reso = get ("reso");      fenv = get ("fenv");
        fdecay = get ("fdecay");
        attack = get ("aA");  decay = get ("aD");  sustain = get ("aS");  release = get ("aR");
        pitchAmt = get ("pitchAmt");  pitchDec = get ("pitchDec");
    }
};

class HardcoreVoice : public juce::SynthesiserVoice
{
public:
    explicit HardcoreVoice (const VoiceParams& vp) : p (vp) {}

    bool canPlaySound (juce::SynthesiserSound* s) override
    {
        return dynamic_cast<HardcoreSound*> (s) != nullptr;
    }

    void startNote (int midiNote, float velocity, juce::SynthesiserSound*, int) override
    {
        sr       = getSampleRate();
        baseFreq = juce::MidiMessage::getMidiNoteInHertz (midiNote);
        vel      = velocity;
        nUni     = juce::jlimit (1, 7, (int) std::round (p.unison->load()));

        const float cents = p.detune->load() * 50.0f;
        const float width = p.width->load();

        for (int u = 0; u < nUni; ++u)
        {
            const float off = (nUni == 1) ? 0.0f
                                          : ((float) u - (nUni - 1) * 0.5f) / ((nUni - 1) * 0.5f);
            uniRatio[u] = std::pow (2.0, (double) (off * cents) / 1200.0);
            const float angle = (off * width + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
            gainL[u] = std::cos (angle) * 1.4142f;
            gainR[u] = std::sin (angle) * 1.4142f;
            phase[u] = rng.nextDouble();
        }
        sinePhase = 0.0;

        adsr.setSampleRate (sr);
        adsr.setParameters ({ p.attack->load(), p.decay->load(), p.sustain->load(), p.release->load() });
        adsr.noteOn();

        pitchEnv = 1.0f;
        fEnv     = 1.0f;
        coefCounter = 0;
        fL.reset();
        fR.reset();
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff)
            adsr.noteOff();
        else
        {
            adsr.reset();
            clearCurrentNote();
        }
    }

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& out, int startSample, int numSamples) override
    {
        if (! isVoiceActive())
            return;

        const float sawL   = p.saw->load();
        const float pulseL = p.pulse->load();
        const float sineL  = p.sine->load();
        const float fenvAmt = p.fenv->load();
        const float baseCut = p.cutoff->load();
        const float k       = 2.0f - 1.95f * p.reso->load();
        const float pAmt    = p.pitchAmt->load();
        const double invSr  = 1.0 / sr;
        const float pitchCoef = std::exp (-1.0f / (std::max (0.005f, p.pitchDec->load()) * (float) sr));
        const float fCoef     = std::exp (-1.0f / (std::max (0.01f,  p.fdecay->load())   * (float) sr));
        const float maxCut    = (float) (sr * 0.45);
        const float uniNorm   = 1.0f / std::sqrt ((float) nUni);
        const bool  stereo    = out.getNumChannels() > 1;

        for (int i = 0; i < numSamples; ++i)
        {
            const float amp = adsr.getNextSample();
            if (! adsr.isActive())
            {
                clearCurrentNote();
                break;
            }

            if ((coefCounter++ & 7) == 0)
            {
                float fc = baseCut * std::exp2 (fenvAmt * 5.0f * fEnv);
                fc = juce::jlimit (20.0f, maxCut, fc);
                const float g = std::tan (juce::MathConstants<float>::pi * fc / (float) sr);
                a1 = 1.0f / (1.0f + g * (g + k));
                a2 = g * a1;
                a3 = g * a2;
            }

            const double f = baseFreq * std::pow (2.0, (double) (pAmt * pitchEnv) / 12.0);

            float l = 0.0f, r = 0.0f;
            for (int u = 0; u < nUni; ++u)
            {
                const double dt = f * uniRatio[u] * invSr;
                double& ph = phase[u];
                float s = 0.0f;

                if (sawL > 0.0f)
                    s += sawL * (float) (2.0 * ph - 1.0 - polyBlep (ph, dt));

                if (pulseL > 0.0f)
                {
                    double t2 = ph + 0.5;
                    if (t2 >= 1.0) t2 -= 1.0;
                    float sq = (ph < 0.5) ? 1.0f : -1.0f;
                    sq += (float) polyBlep (ph, dt);
                    sq -= (float) polyBlep (t2, dt);
                    s += pulseL * sq;
                }

                ph += dt;
                if (ph >= 1.0) ph -= 1.0;

                l += s * gainL[u];
                r += s * gainR[u];
            }
            l *= uniNorm;
            r *= uniNorm;

            if (sineL > 0.0f)
            {
                const float sn = sineL * (float) std::sin (juce::MathConstants<double>::twoPi * sinePhase);
                sinePhase += f * invSr;
                if (sinePhase >= 1.0) sinePhase -= 1.0;
                l += sn;
                r += sn;
            }

            l = fL.process (l, a1, a2, a3);
            r = fR.process (r, a1, a2, a3);

            const float a = amp * vel * 0.3f;
            out.addSample (0, startSample + i, l * a);
            if (stereo)
                out.addSample (1, startSample + i, r * a);

            pitchEnv *= pitchCoef;
            fEnv     *= fCoef;
        }
    }

private:
    static double polyBlep (double t, double dt) noexcept
    {
        if (t < dt)         { t /= dt;               return t + t - t * t - 1.0; }
        if (t > 1.0 - dt)   { t = (t - 1.0) / dt;    return t * t + t + t + 1.0; }
        return 0.0;
    }

    struct LowpassSVF
    {
        float ic1 = 0.0f, ic2 = 0.0f;
        void reset() noexcept { ic1 = ic2 = 0.0f; }
        float process (float x, float a1, float a2, float a3) noexcept
        {
            const float v3 = x - ic2;
            const float v1 = a1 * ic1 + a2 * v3;
            const float v2 = ic2 + a2 * ic1 + a3 * v3;
            ic1 = 2.0f * v1 - ic1;
            ic2 = 2.0f * v2 - ic2;
            return v2;
        }
    };

    const VoiceParams& p;
    juce::ADSR adsr;
    juce::Random rng;
    LowpassSVF fL, fR;

    double sr = 44100.0, baseFreq = 440.0, sinePhase = 0.0;
    double phase[7] {}, uniRatio[7] {};
    float  gainL[7] {}, gainR[7] {};
    float  vel = 1.0f, pitchEnv = 1.0f, fEnv = 1.0f;
    float  a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
    int    nUni = 1, coefCounter = 0;
};
