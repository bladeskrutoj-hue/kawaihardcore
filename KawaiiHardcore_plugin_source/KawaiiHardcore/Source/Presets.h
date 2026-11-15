#pragma once
#include <vector>
#include <utility>

struct Preset
{
    const char* name;
    std::vector<std::pair<const char*, float>> values;   // everything not listed = parameter default
};

inline const std::vector<Preset>& getPresets()
{
    static const std::vector<Preset> presets =
    {
        { "S3RL Supersaw Lead", {
            { "sawLvl", 1.0f }, { "unison", 7 }, { "detune", 0.45f }, { "width", 0.9f },
            { "cutoff", 9000.0f }, { "reso", 0.2f }, { "fenv", 0.25f }, { "fdecay", 0.25f },
            { "aA", 0.002f }, { "aD", 0.3f }, { "aS", 0.85f }, { "aR", 0.15f }, { "drive", 0.1f } } },

        { "Old School Hoover", {
            { "sawLvl", 0.8f }, { "pulseLvl", 0.6f }, { "unison", 5 }, { "detune", 0.85f },
            { "cutoff", 2500.0f }, { "reso", 0.5f }, { "fenv", 0.7f }, { "fdecay", 0.4f },
            { "pitchAmt", 12.0f }, { "pitchDec", 0.25f },
            { "aA", 0.005f }, { "aS", 1.0f }, { "aR", 0.2f }, { "drive", 0.4f } } },

        { "Gabber Kick", {
            { "sawLvl", 0.0f }, { "sineLvl", 1.0f }, { "unison", 1 },
            { "cutoff", 14000.0f }, { "fenv", 0.0f },
            { "pitchAmt", 40.0f }, { "pitchDec", 0.07f },
            { "aA", 0.001f }, { "aD", 0.35f }, { "aS", 0.0f }, { "aR", 0.05f },
            { "drive", 0.9f } } },

        { "Happy Core Bass", {
            { "sawLvl", 0.6f }, { "pulseLvl", 0.4f }, { "sineLvl", 0.8f }, { "unison", 1 },
            { "cutoff", 1200.0f }, { "reso", 0.3f }, { "fenv", 0.5f }, { "fdecay", 0.15f },
            { "aA", 0.002f }, { "aS", 0.9f }, { "aR", 0.08f }, { "drive", 0.3f } } },

        { "Trance Pluck", {
            { "sawLvl", 0.5f }, { "pulseLvl", 0.8f }, { "unison", 3 }, { "detune", 0.25f },
            { "cutoff", 1500.0f }, { "reso", 0.35f }, { "fenv", 0.8f }, { "fdecay", 0.18f },
            { "aA", 0.001f }, { "aD", 0.25f }, { "aS", 0.2f }, { "aR", 0.2f }, { "drive", 0.05f } } },

        { "Trance Gate Pad", {
            { "sawLvl", 1.0f }, { "unison", 7 }, { "detune", 0.6f }, { "width", 1.0f },
            { "cutoff", 4000.0f }, { "fenv", 0.1f },
            { "aA", 0.15f }, { "aS", 1.0f }, { "aR", 0.6f },
            { "gate", 1.0f }, { "gateRate", 1.0f }, { "gateDuty", 0.5f }, { "drive", 0.0f } } },

        { "Euro Stab", {
            { "sawLvl", 0.9f }, { "pulseLvl", 0.5f }, { "unison", 3 }, { "detune", 0.3f },
            { "cutoff", 3500.0f }, { "fenv", 0.6f }, { "fdecay", 0.12f },
            { "aA", 0.001f }, { "aD", 0.15f }, { "aS", 0.0f }, { "aR", 0.1f }, { "drive", 0.2f } } },

        { "Rave Screech Lead", {
            { "sawLvl", 1.0f }, { "pulseLvl", 0.5f }, { "unison", 5 }, { "detune", 0.9f },
            { "cutoff", 6000.0f }, { "reso", 0.7f }, { "fenv", 0.4f }, { "fdecay", 0.3f },
            { "aS", 0.9f }, { "drive", 0.6f } } },

        { "Distorted Gabber Lead", {
            { "sawLvl", 1.0f }, { "pulseLvl", 0.3f }, { "sineLvl", 0.3f }, { "unison", 3 }, { "detune", 0.35f },
            { "cutoff", 5000.0f }, { "reso", 0.4f }, { "fenv", 0.5f }, { "fdecay", 0.2f },
            { "pitchAmt", 5.0f }, { "pitchDec", 0.03f }, { "aS", 0.9f }, { "drive", 1.0f } } },

        { "Kawaii Soft Pad", {
            { "sawLvl", 0.4f }, { "pulseLvl", 0.2f }, { "sineLvl", 0.3f }, { "unison", 4 }, { "detune", 0.3f },
            { "width", 1.0f }, { "cutoff", 2500.0f }, { "fenv", 0.0f },
            { "aA", 0.4f }, { "aD", 1.0f }, { "aS", 0.8f }, { "aR", 1.2f }, { "drive", 0.0f } } },
    };
    return presets;
}
