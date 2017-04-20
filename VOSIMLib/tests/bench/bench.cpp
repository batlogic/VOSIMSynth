#define NONIUS_RUNNER
#include <nonius.h++>

#include "tables.h"
#include "DSPMath.h"
#include "NamedContainer.h"
#include "Unit.h"
#include "units/StateVariableFilter.h"
#include "Circuit.h"
#include "units/Oscillator.h"
#include "units/MidiUnits.h"
#include "units/ADSREnvelope.h"

#include <array>
#include <algorithm>
#include <random>
#include <string>

#define trig_benches 1
#define ladder_benches 0
#define circuit_benches 1
#define modulus_benches 0
#define lut_saw_benches 1
#define lut_pitch_benches 1
#define container_benches 1

std::random_device RandomDevice;

#if trig_benches
NONIUS_BENCHMARK("syn::lut_sin.getraw", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<int> phases(runs);
    const auto& lut_sin_table = syn::lut_sin_table();
    auto _phaseGenerator = [&runs, &lut_sin_table](int& n)->int { return n*1.0 / runs*lut_sin_table.size; };
    std::transform(phases.begin(), phases.end(), phases.begin(), _phaseGenerator);
    for (int i = 0; i < runs; i++) phases[i] = i * (1.0 / runs) * lut_sin_table.size;
    double x;
    meter.measure([&x, &phases, &lut_sin_table](int i) { x = lut_sin_table[phases[i]]; });
})

NONIUS_BENCHMARK("syn::lut_sin.lerp", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<double> phases(runs);
    const auto& lut_sin_table = syn::lut_sin_table();
    for (int i = 0; i < runs; i++) phases[i] = i*1.0 / runs;
    double x;
    meter.measure([&x, &phases, &lut_sin_table](int i) { x = lut_sin_table.lerp(phases[i]); });
})

NONIUS_BENCHMARK("syn::lut_sin.plerp", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<double> phases(runs);
    const auto& lut_sin_table = syn::lut_sin_table();
    for (int i = 0; i < runs; i++) phases[i] = i*1.0 / runs;
    double x;
    meter.measure([&x, &phases, &lut_sin_table](int i) { x = lut_sin_table.plerp(phases[i]); });
})

NONIUS_BENCHMARK("std::sin", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<double> phases(runs);
    const auto& lut_sin_table = syn::lut_sin_table();
    for (int i = 0; i < runs; i++) phases[i] = DSP_PI * i * 1.0 / runs;
    double x;
    meter.measure([&x, &phases](int i) { x = std::sin(phases[i]); });
})

NONIUS_BENCHMARK("std::tanh", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<double> phases(runs);
    for (int i = 0; i < runs; i++) phases[i] = i * 20.0 / runs - 10.;
    double x;
    meter.measure([&x, &phases](int i) { x = std::tanh(phases[i]); });
})

NONIUS_BENCHMARK("syn::fast_tanh_rat", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<double> phases(runs);
    for (int i = 0; i < runs; i++) phases[i] = i * 6. / runs - 3.;
    double x;
    meter.measure([&x, &phases](int i) { x = syn::fast_tanh_rat<double>(phases[i]); });
})

NONIUS_BENCHMARK("syn::fast_tanh_rat_2", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<double> phases(runs);
    for (int i = 0; i < runs; i++) phases[i] = i * 6. / runs - 3.;
    double x;
    meter.measure([&x, &phases](int i) { x = syn::fast_tanh_rat2<double>(phases[i]); });
})
#endif

#if ladder_benches
NONIUS_BENCHMARK("ladder", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    syn::LadderFilter ladder("ladder");
    double input = 1.0;
    ladder.setFs(48000.0);
    ladder.setParam(syn::LadderFilter::pFc, 10000.0);
    ladder.setParam(syn::LadderFilter::pFb, 1.0);
    ladder.setParam(syn::LadderFilter::pDrv, 0.0);
    ladder.connectInput(0, &input);

    double x;
    meter.measure([&x, &input, &ladder](int i)
    {
        ladder.tick();
        x = ladder.readOutput(0,0);
        return x;
    });
})

NONIUS_BENCHMARK("ladder_B", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    syn::LadderFilterTwo ladder("ladder");
    double input = 1.0;
    ladder.setFs(48000.0);
    ladder.setParam(syn::LadderFilter::pFc, 10000.0);
    ladder.setParam(syn::LadderFilter::pFb, 1.0);
    ladder.setParam(syn::LadderFilter::pDrv, 0.0);
    ladder.connectInput(0, &input);

    double x;
    meter.measure([&x, &input, &ladder](int i)
    {
        ladder.tick();
        x = ladder.readOutput(0,0);
        return x;
    });
})

NONIUS_BENCHMARK("svf", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    syn::StateVariableFilter svf("ladder");
    double input = 1.0;
    svf.setFs(48000.0);
    svf.setParam(0, 10000.0);
    svf.setParam(1, 1.0);
    svf.connectInput(0, &input);

    double x;
    meter.measure([&x, &input, &svf](int i)
    {
        svf.tick();
        x = svf.readOutput(0,0);
        return x;
    });
})

NONIUS_BENCHMARK("trapezoidal svf", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    syn::TrapStateVariableFilter tsvf("ladder");
    double input = 1.0;
    tsvf.setFs(48000.0);
    tsvf.setParam(0, 10000.0);
    tsvf.setParam(1, 1.0);
    tsvf.connectInput(0, &input);

    double x;
    meter.measure([&x, &input, &tsvf](int i)
    {
        tsvf.tick();
        x = tsvf.readOutput(0,0);
        return x;
    });
})
#endif

#if circuit_benches
NONIUS_BENCHMARK("ladder circuit (buf size=1)", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    const int nUnits = 12;
    syn::Circuit mycircuit("main");
    mycircuit.setFs(48000.0);
    mycircuit.setBufferSize(1);
    // Add a bunch of ladder filter units in serial
    syn::LadderFilter* lf[nUnits];
    lf[0] = new syn::LadderFilter("lf0");
    mycircuit.addUnit(lf[0]);
    for (int i = 1; i < nUnits; i++) {
        lf[i] = new syn::LadderFilter("lf" + std::to_string(i));
        mycircuit.addUnit(lf[i]);
        mycircuit.connectInternal(mycircuit.getUnitId(*lf[i - 1]), 0, mycircuit.getUnitId(*lf[i]), 0);
    }
    // Add pitch and oscillator units
    syn::MidiNoteUnit* mnu = new syn::MidiNoteUnit("mnu0");
    mycircuit.addUnit(mnu);
    syn::BasicOscillator* bosc = new syn::BasicOscillator("bosc0");
    mycircuit.addUnit(bosc);
    mycircuit.connectInternal(mycircuit.getUnitId(*mnu), 0, mycircuit.getUnitId(*bosc), 0);
    mycircuit.connectInternal(mycircuit.getUnitId(*bosc), 0, mycircuit.getUnitId(*lf[0]), 0);
    // Connect final ladder unit to circuit output
    mycircuit.connectInternal(mycircuit.getUnitId(*lf[nUnits - 1]), 0, mycircuit.getOutputUnitId(), 0);

    double x;
    meter.measure([&x, &mycircuit](int i)
    {
        for (int j = 0; j < 4800; j++)
            mycircuit.tick();
        x = mycircuit.readOutput(0, 0);
        return x;
    });
})

NONIUS_BENCHMARK("ladder circuit (buf size=4800)", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    const int nUnits = 12;
    syn::Circuit mycircuit("main");
    mycircuit.setFs(48000.0);
    mycircuit.setBufferSize(4800);
    // Add a bunch of ladder filter units in serial
    syn::LadderFilter* lf[nUnits];
    lf[0] = new syn::LadderFilter("lf0");
    mycircuit.addUnit(lf[0]);
    for (int i = 1; i < nUnits; i++) {
        lf[i] = new syn::LadderFilter("lf" + std::to_string(i));
        mycircuit.addUnit(lf[i]);
        mycircuit.connectInternal(mycircuit.getUnitId(*lf[i - 1]), 0, mycircuit.getUnitId(*lf[i]), 0);
    }
    // Add pitch and oscillator units
    syn::MidiNoteUnit* mnu = new syn::MidiNoteUnit("mnu0");
    mycircuit.addUnit(mnu);
    syn::BasicOscillator* bosc = new syn::BasicOscillator("bosc0");
    mycircuit.addUnit(bosc);
    mycircuit.connectInternal(mycircuit.getUnitId(*mnu), 0, mycircuit.getUnitId(*bosc), 0);
    mycircuit.connectInternal(mycircuit.getUnitId(*bosc), 0, mycircuit.getUnitId(*lf[0]), 0);
    // Connect final ladder unit to circuit output
    mycircuit.connectInternal(mycircuit.getUnitId(*lf[nUnits - 1]), 0, mycircuit.getOutputUnitId(), 0);

    double x;
    meter.measure([&x, &mycircuit](int i)
    {
        mycircuit.tick();
        x = mycircuit.readOutput(0, 0);
        return x;
    });
})
#endif

#if modulus_benches
NONIUS_BENCHMARK("std::mod", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<double> phases(runs);
    for (int i = 0; i < runs; i++) phases[i] = i * 20.0 / runs - 10.;
    double x;
    meter.measure([&x, &phases](int i) { x = std::fmod(phases[i], 1); });
})

NONIUS_BENCHMARK("syn::WRAP", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<double> phases(runs);
    for (int i = 0; i < runs; i++) phases[i] = i * 20.0 / runs - 10.;
    double x;
    meter.measure([&x, &phases](int i) { x = syn::WRAP(phases[i], 1.0); });
})
#endif

#if lut_saw_benches
NONIUS_BENCHMARK("lut saw", [](nonius::chronometer& meter) {
    std::vector<double> periods(meter.runs());
    std::vector<double> phases(meter.runs());
    std::uniform_int_distribution<> _periodGenerator(2, syn::lut_bl_saw_table().size - 1);
    std::uniform_real_distribution<> _phaseGenerator(0.0, 1.0);
    std::transform(periods.begin(), periods.end(), periods.begin(), [&_periodGenerator](double& x) {return _periodGenerator(RandomDevice); });
    std::transform(phases.begin(), phases.end(), phases.begin(), [&_phaseGenerator](double& x) {return _phaseGenerator(RandomDevice); });
    double x;
    meter.measure([&x, &periods, &phases](int i)
    {
        x = syn::lut_bl_saw_table().getresampled(phases[i], periods[i]);
    });
})

NONIUS_BENCHMARK("naive saw", [](nonius::chronometer& meter) {
    std::vector<double> phases(meter.runs());
    std::uniform_real_distribution<> _phaseGenerator(0.0, 1.0);
    std::transform(phases.begin(), phases.end(), phases.begin(), [&_phaseGenerator](double& x) {return _phaseGenerator(RandomDevice); });
    double x;
    meter.measure([&x, &phases](int i)
    {
        x = syn::naive_saw(phases[i]);
    });
})
#endif

#if lut_pitch_benches
NONIUS_BENCHMARK("lut pitch2freq", [](nonius::chronometer& meter) {
    std::vector<double> pitches(meter.runs());
    std::uniform_int_distribution<> _pitchGenerator(-128, 128);
    std::transform(pitches.begin(), pitches.end(), pitches.begin(), [&_pitchGenerator](double& x) {return _pitchGenerator(RandomDevice); });
    double x;
    meter.measure([&x, &pitches](int i)
    {
        x = syn::pitchToFreq(pitches[i]);
    });
})

NONIUS_BENCHMARK("naive pitch2freq", [](nonius::chronometer& meter) {
    std::vector<double> pitches(meter.runs());
    std::uniform_int_distribution<> _pitchGenerator(-128, 128);
    std::transform(pitches.begin(), pitches.end(), pitches.begin(), [&_pitchGenerator](double& x) {return _pitchGenerator(RandomDevice); });
    double x;
    meter.measure([&x, &pitches](int i)
    {
        x = syn::naive_pitchToFreq(pitches[i]);
    });
})
#endif

#if container_benches
#define container_size 64
NONIUS_BENCHMARK("syn::NamedContainer []", [](nonius::chronometer& meter)
{
    std::uniform_int_distribution<> _accessGenerator(0, container_size - 1);
    std::vector<int> loads(meter.runs());
    std::vector<int> stores(meter.runs());
    std::transform(loads.begin(), loads.end(), loads.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });
    std::transform(stores.begin(), stores.end(), stores.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });

    syn::NamedContainer<syn::InputPort, container_size> myContainer;
    for (int i = 0; i < container_size; i++) myContainer.add(std::to_string(i), syn::InputPort(i));

    meter.measure([&myContainer, &stores, &loads](int i)
    {
        myContainer[stores[i]] = myContainer[loads[i]];
    });
})

NONIUS_BENCHMARK("std::unordered_map []", [](nonius::chronometer& meter)
{
    std::uniform_int_distribution<> _accessGenerator(0, container_size - 1);
    std::vector<int> loads(meter.runs());
    std::vector<int> stores(meter.runs());
    std::transform(loads.begin(), loads.end(), loads.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });
    std::transform(stores.begin(), stores.end(), stores.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });

    std::unordered_map<int, syn::InputPort> myContainer;
    for (int i = 0; i < container_size; i++) myContainer[i] = syn::InputPort(i);

    meter.measure([&myContainer, &stores, &loads](int i)
    {
        myContainer[stores[i]] = myContainer[loads[i]];
    });
})

NONIUS_BENCHMARK("std::array []", [](nonius::chronometer& meter)
{
    std::uniform_int_distribution<> _accessGenerator(0, container_size - 1);
    std::vector<int> loads(meter.runs());
    std::vector<int> stores(meter.runs());
    std::transform(loads.begin(), loads.end(), loads.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });
    std::transform(stores.begin(), stores.end(), stores.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });

    std::array<syn::InputPort, container_size> myContainer;
    for (int i = 0; i < container_size; i++) myContainer[i] = syn::InputPort(i);

    meter.measure([&myContainer, &stores, &loads](int i)
    {
        myContainer[stores[i]] = myContainer[loads[i]];
    });
})

NONIUS_BENCHMARK("std::vector []", [](nonius::chronometer& meter)
{
    std::uniform_int_distribution<> _accessGenerator(0, container_size - 1);
    std::vector<int> loads(meter.runs());
    std::vector<int> stores(meter.runs());
    std::transform(loads.begin(), loads.end(), loads.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });
    std::transform(stores.begin(), stores.end(), stores.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });

    std::vector<syn::InputPort> myContainer(container_size);
    for (int i = 0; i < container_size; i++) myContainer[i] = syn::InputPort(i);

    meter.measure([&myContainer, &stores, &loads](int i)
    {
        myContainer[stores[i]] = myContainer[loads[i]];
    });
})

NONIUS_BENCHMARK("std::map [int]", [](nonius::chronometer& meter)
{
    std::uniform_int_distribution<> _accessGenerator(0, container_size - 1);
    std::vector<int> loads(meter.runs());
    std::vector<int> stores(meter.runs());
    std::transform(loads.begin(), loads.end(), loads.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });
    std::transform(stores.begin(), stores.end(), stores.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });

    std::map<int, syn::InputPort> myContainer;
    for (int i = 0; i < container_size; i++) myContainer[i] = syn::InputPort(i);

    meter.measure([&myContainer, &stores, &loads](int i)
    {
        myContainer[stores[i]] = myContainer[loads[i]];
    });
})
#endif