#include "bench.h"
#include "vosimlib/tables.h"
#include "vosimlib/DSPMath.h"
#include "vosimlib/IntMap.h"
#include "vosimlib/Unit.h"
#include "vosimlib/Circuit.h"

#include <iostream>

int main(int argc, char** argv) {
    const int bufSize = 256;
    syn::VoiceManager vm;
    syn::Circuit mycircuit = makeTestCircuit();
    vm.setPrototypeCircuit(mycircuit);
    vm.setFs(48e3);
    vm.setMaxVoices(16);
    vm.setBufferSize(bufSize);
    vm.setInternalBufferSize(256);
    // Trigger 16 voices
    vm.noteOn(60, 127);
    vm.noteOn(61, 127);
    vm.noteOn(62, 127);
    vm.noteOn(63, 127);
    vm.noteOn(64, 127);
    vm.noteOn(65, 127);
    vm.noteOn(66, 127);
    vm.noteOn(67, 127);
    vm.noteOn(68, 127);
    vm.noteOn(69, 127);
    vm.noteOn(70, 127);
    vm.noteOn(71, 127);
    vm.noteOn(72, 127);
    vm.noteOn(73, 127);
    vm.noteOn(74, 127);
    vm.noteOn(75, 127);

    std::vector<double> leftIn(bufSize, 0), rightIn(bufSize, 0);
    std::vector<double> leftOut(bufSize, 0), rightOut(bufSize, 0);
    for (int i = 0; i < 1; i++) {
        vm.tick(&leftIn.front(), &rightIn.front(), &leftOut.front(), &rightOut.front());
    }
    for(int i = 0; i < bufSize; i++) {
        std::cout << leftOut[i] << "\t" << rightOut[i] << std::endl;
    }
    return 0;
}