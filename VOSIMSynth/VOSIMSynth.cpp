/*
Copyright 2016, Austen Satterlee

This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/

#include "VOSIMSynth.h"
#include "IPlug_include_in_plug_src.h"

#include <units/Oscillator.h>
#include <units/VosimOscillator.h>
#include <units/ADSREnvelope.h>
#include <units/Follower.h>
#include <units/MathUnits.h>
#include <units/MemoryUnit.h>
#include <units/MidiUnits.h>
#include <units/StateVariableFilter.h>
#include <units/WaveShapers.h>
#include <MIDIReceiver.h>
#include <VoiceManager.h>
#include <UnitFactory.h>
#include <tables.h>

#include "MainWindow.h"
#include "MainGUI.h"
#include "CircuitWidget.h"
#include "Widgets/SummingUnitWidget.h"
#include "Widgets/MultiplyingUnitWidget.h"

VOSIMSynth::VOSIMSynth(IPlugInstanceInfo instanceInfo) : IPLUG_CTOR(0, 1, instanceInfo), m_tempo(0), m_tickCount(0) {
    TRACE;
    makeInstrument();
    makeGraphics();
    syn::lut_bl_tri_table();
    syn::lut_bl_saw_table();
    syn::lut_bl_square_table();
}

void VOSIMSynth::makeGraphics() {
    syn::VoiceManager* vm = m_voiceManager;
    syn::UnitFactory* uf = m_unitFactory;
    synui::MainWindow* mainWindow = new synui::MainWindow(GUI_WIDTH, GUI_HEIGHT, [vm, uf](synui::MainWindow* a_win) { return new synui::MainGUI(a_win, vm, uf); });
    mainWindow->setHInstance(gHInstance);
    mainWindow->onResize.Connect(this, &VOSIMSynth::ResizeGraphics);
    AttachAppWindow(mainWindow);
    registerUnitWidgets(*mainWindow->getGUI()->circuitWidget());
}

void VOSIMSynth::makeInstrument() {
    m_unitFactory = &syn::UnitFactory::instance();
    registerUnits(*m_unitFactory);

    m_voiceManager = new syn::VoiceManager();
    m_voiceManager->setMaxVoices(8);

    m_MIDIReceiver = new syn::MIDIReceiver(m_voiceManager);
}

VOSIMSynth::~VOSIMSynth() {
    delete m_voiceManager;
    delete m_MIDIReceiver;
}

void VOSIMSynth::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames) {
    // Mutex is already locked for us.

    for (int s = 0; s < nFrames; s++) {
        m_MIDIReceiver->advance();
    }

    // If tempo has changed, notify instrument
    if (m_tempo != GetTempo()) {
        m_tempo = GetTempo();
        m_voiceManager->setTempo(GetTempo());
    }

    // Process samples
    m_voiceManager->tick(inputs[0], inputs[1], outputs[0], outputs[1]);

    m_MIDIReceiver->Flush(nFrames);
    m_tickCount++;
}

void VOSIMSynth::ProcessMidiMsg(IMidiMsg* pMsg) {
    m_MIDIReceiver->onMessageReceived(pMsg);
}

bool VOSIMSynth::SerializeState(ByteChunk* pChunk) {
    syn::Circuit* circuit = m_voiceManager->getPrototypeCircuit();
    std::stringstream ss;
    json j;
    // Store synthesizer data
    json& synth = j["synth"] = json();
    synth["circuit"] = circuit->operator json();

    // Store gui data
    json& gui = j["gui"] = GetAppWindow()->operator json();

    ss << j;
    pChunk->PutStr(ss.str().c_str());
    return true;
}

int VOSIMSynth::UnserializeState(ByteChunk* pChunk, int startPos) {
    m_unitFactory->resetBuildCounts();

    string input;
    startPos = pChunk->Get(&input, startPos);
    std::stringstream ss{ input };
    json j; ss >> j;

    // Main JSON components
    const json& synth = j["synth"];
    const json& gui = j["gui"];

    syn::Unit* circuit = syn::Unit::fromJSON(synth["circuit"]);

    // Reset gui
    GetAppWindow()->reset();
    // Load new circuit into voice manager
    m_voiceManager->setPrototypeCircuit(*static_cast<const syn::Circuit*>(circuit));
    // Inform new circuit of buffer size, sampling rate, etc...
    Reset();
    // Load gui
    GetAppWindow()->load(gui);
    m_voiceManager->onIdle();

    startPos += ss.gcount();
    return startPos;
}

void VOSIMSynth::PresetsChangedByHost() {}

void VOSIMSynth::OnIdle() {
    m_voiceManager->onIdle();
}

void VOSIMSynth::OnActivate(bool active) {}

void VOSIMSynth::OnGUIOpen() {}

void VOSIMSynth::OnGUIClose() {}

void VOSIMSynth::registerUnits(syn::UnitFactory& a_uf)
{
    a_uf.addUnitPrototype<syn::StateVariableFilter>("Audio Filters", "svf");
    a_uf.addUnitPrototype<syn::TrapStateVariableFilter>("Audio Filters", "tsvf");
    a_uf.addUnitPrototype<syn::LadderFilter>("Audio Filters", "ldrA");
    a_uf.addUnitPrototype<syn::LadderFilterTwo>("Audio Filters", "ldrB");
    a_uf.addUnitPrototype<syn::OnePoleLP>("Mod Filters", "lag");
    a_uf.addUnitPrototype<syn::FollowerUnit>("Mod Filters", "follow");
    a_uf.addUnitPrototype<syn::DCRemoverUnit>("Mod Filters", "dc");

    a_uf.addUnitPrototype<syn::BasicOscillator>("Oscillators", "basic");
    a_uf.addUnitPrototype<syn::VosimOscillator>("Oscillators", "vosim");
    a_uf.addUnitPrototype<syn::FormantOscillator>("Oscillators", "formant");

    a_uf.addUnitPrototype<syn::ADSREnvelope>("Modulators", "ADSR");
    a_uf.addUnitPrototype<syn::LFOOscillator>("Modulators", "LFO");

    a_uf.addUnitPrototype<syn::MemoryUnit>("Delays", "z^-1");
    a_uf.addUnitPrototype<syn::VariableMemoryUnit>("Delays", "z^-t");
    
    a_uf.addUnitPrototype<syn::PanningUnit>("Manipulators", "pan");
    a_uf.addUnitPrototype<syn::SwitchUnit>("Manipulators", "switch");
    a_uf.addUnitPrototype<syn::LerpUnit>("Manipulators", "affine");
    a_uf.addUnitPrototype<syn::QuantizerUnit>("Manipulators", "quantize");
    a_uf.addUnitPrototype<syn::RectifierUnit>("Manipulators", "rect");
    a_uf.addUnitPrototype<syn::TanhUnit>("Manipulators", "tanh");

    a_uf.addUnitPrototype<syn::ConstantUnit>("Arithmetics", "const");
    a_uf.addUnitPrototype<syn::SummerUnit>("Arithmetics", "sum");
    a_uf.addUnitPrototype<syn::GainUnit>("Arithmetics", "mul");
    
    a_uf.addUnitPrototype<syn::PitchToFreqUnit>("Converters", "p2f");
    a_uf.addUnitPrototype<syn::FreqToPitchUnit>("Converters", "f2p");

    a_uf.addUnitPrototype<syn::GateUnit>("MIDI", "gate");
    a_uf.addUnitPrototype<syn::MidiNoteUnit>("MIDI", "pitch");
    a_uf.addUnitPrototype<syn::VelocityUnit>("MIDI", "vel");
    a_uf.addUnitPrototype<syn::MidiCCUnit>("MIDI", "CC");
    a_uf.addUnitPrototype<syn::VoiceIndexUnit>("MIDI", "voice");

    a_uf.addUnitPrototype<syn::Circuit>("", "circuit");
    a_uf.addUnitPrototype<syn::InputUnit>("", "in");
    a_uf.addUnitPrototype<syn::OutputUnit>("", "out");
}

void VOSIMSynth::registerUnitWidgets(synui::CircuitWidget& a_cw)
{
    a_cw.registerUnitWidget<syn::SummerUnit>([](synui::CircuitWidget* parent, syn::VoiceManager* a_vm, int unitId) { return new synui::SummingUnitWidget(parent, a_vm, unitId); });
    a_cw.registerUnitWidget<syn::GainUnit>([](synui::CircuitWidget* parent, syn::VoiceManager* a_vm, int unitId) { return new synui::MultiplyingUnitWidget(parent, a_vm, unitId); });
}


void VOSIMSynth::Reset() {
    m_MIDIReceiver->Resize(GetBlockSize());
    m_voiceManager->setBufferSize(GetBlockSize());
    m_voiceManager->setFs(GetSampleRate());
}

void VOSIMSynth::OnParamChange(int paramIdx) {}