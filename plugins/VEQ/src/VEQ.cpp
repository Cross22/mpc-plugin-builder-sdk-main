// VEQ: eight-band parameter model and pass-through audio path.
// The on-device skin currently displays a static response-grid placeholder;
// this plugin does not apply the band parameters to incoming audio.

#include "mpc_plugin.h"
#include "dsp/smoother.h"

#include <cstdio>
#include <vector>

namespace {

constexpr int kNumBands = 8;
constexpr int kMaxChannels = 2;
constexpr int kParamsPerBand = 4;

constexpr float kDefaultFrequencies[kNumBands] = {
    20.0f, 54.0f, 145.0f, 386.0f, 1036.0f, 2782.0f, 7473.0f, 20000.0f
};

float dbToGain(float dB)
{
    return std::pow(10.0f, dB / 20.0f);
}

} // namespace

class VEQ : public mpc::Plugin
{
public:
    VEQ() : mpc::Plugin(makeInfo())
    {
        // Parameter zero remains harmless for the MPC host's load-time nudge.
        // Retain the original "gain" ID so existing VEQ state stays compatible.
        addParam(mpc::floatParam("gain", "Master", -24.0f, 24.0f, 0.0f, "dB", 1));

        std::vector<float> defaultProgram;
        defaultProgram.reserve(1 + kNumBands * kParamsPerBand);
        defaultProgram.push_back(0.0f);

        for (int band = 0; band < kNumBands; ++band)
        {
            char id[24];
            char name[16];
            const int bandNumber = band + 1;

            std::snprintf(id, sizeof(id), "band%d_on", bandNumber);
            std::snprintf(name, sizeof(name), "B%d On", bandNumber);
            addParam(mpc::boolParam(id, name, true));

            std::snprintf(id, sizeof(id), "band%d_freq", bandNumber);
            std::snprintf(name, sizeof(name), "FREQ");
            addParam(mpc::logParam(id, name, 20.0f, 20000.0f, kDefaultFrequencies[band], "Hz", 0));

            std::snprintf(id, sizeof(id), "band%d_gain", bandNumber);
            std::snprintf(name, sizeof(name), "GAIN");
            addParam(mpc::floatParam(id, name, -24.0f, 24.0f, 0.0f, "dB", 1));

            std::snprintf(id, sizeof(id), "band%d_q", bandNumber);
            std::snprintf(name, sizeof(name), "Q");
            addParam(mpc::logParam(id, name, 0.1f, 10.0f, 0.71f, "", 2));

            defaultProgram.push_back(1.0f);
            defaultProgram.push_back(kDefaultFrequencies[band]);
            defaultProgram.push_back(0.0f);
            defaultProgram.push_back(0.71f);
        }

        addParam(mpc::boolParam("analyze", "Analyze", false));
        addParam(mpc::boolParam("debug", "Debug", false));
        defaultProgram.push_back(0.0f);
        defaultProgram.push_back(0.0f);

        addProgram("Default", defaultProgram);
    }

    void prepare(double sampleRate, int maxBlockSize) override
    {
        (void)maxBlockSize;
        sampleRate_ = sampleRate;
        masterGain_.setTime(10.0f, sampleRate_);
        reset();
    }

    void reset() override
    {
        masterGain_.snap(dbToGain(get(0)));
    }

    void process(mpc::ProcessContext& ctx) override
    {
        const int numFrames = ctx.numFrames;
        const int numChannels = mpc::clamp(ctx.numOutputs, 0, kMaxChannels);
        masterGain_.setTarget(dbToGain(get(0)));

        if (ctx.outputs == nullptr)
            return;

        for (int frame = 0; frame < numFrames; ++frame)
        {
            const float gain = masterGain_.process();

            for (int ch = 0; ch < numChannels; ++ch)
            {
                if (ctx.outputs[ch] == nullptr)
                    continue;

                const float* input = (ctx.inputs != nullptr && ch < ctx.numInputs)
                    ? ctx.inputs[ch] : nullptr;
                ctx.outputs[ch][frame] = input != nullptr ? input[frame] * gain : 0.0f;
            }
        }
    }

    void parameterChanged(int index, float plainValue) override
    {
        (void)index;
        (void)plainValue;
    }

private:
    static mpc::PluginInfo makeInfo()
    {
        mpc::PluginInfo info;
        info.name = "VEQ";
        info.vendor = "BC";
        info.uniqueId = MPC_FOURCC(kUid[0], kUid[1], kUid[2], kUid[3]);
        info.version = 1200;
        info.isInstrument = false;
        info.numInputs = 2;
        info.numOutputs = 2;
        info.tailSamples = 1;
        return info;
    }

    static constexpr char kUid[5] = "BhOs";
    mpc::dsp::Smoother masterGain_;
    double sampleRate_ = 44100.0;
};

MPC_PLUGIN_ENTRY(VEQ)
