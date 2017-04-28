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

#ifndef __VOICEMANAGER__
#define __VOICEMANAGER__

#include "Circuit.h"
#include "Unit.h"
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/lockfree/policies.hpp>
#include <boost/circular_buffer.hpp>
#include <map>

#define MAX_VOICEMANAGER_MSG_QUEUE_SIZE 1024
#define MAX_VOICES 16

using std::map;
using std::string;
using boost::lockfree::spsc_queue;
using boost::lockfree::capacity;

namespace syn {
    
    class Command;

    class VOSIMLIB_API VoiceManager {
    public:
        enum VoiceStealingPolicy {
            Oldest = 0,
            Newest,
            Lowest,
            Highest
        };

    public:
        VoiceManager()
            :
            m_queuedActions{MAX_VOICEMANAGER_MSG_QUEUE_SIZE},
            m_bufferSize(1),
            m_internalBufferSize(1),
            m_tickCount(0),
            m_activeVoices(MAX_VOICES),
            m_idleVoices(MAX_VOICES),
            m_garbageList(MAX_VOICES),
            m_instrument{"main"},
            m_voiceStealingPolicy(Oldest),
            m_legato(false) {
            setBufferSize(m_bufferSize);
            setInternalBufferSize(m_internalBufferSize);
        }

        void MSFASTCALL tick(const double* a_left_input, const double* a_right_input, double* a_left_output, double* a_right_output) GCCFASTCALL;

        /**
         * Safely queue a function to be called on the real-time thread in between samples.
         * 
         * Note that this function should ONLY be called from the gui thread! This is a single-producer
         * single-consumer queue!
         * 
         * \returns True if the message was pushed onto the queue, false if the queue was full.
         */
        bool queueAction(Command* a_action);

        unsigned getTickCount() const;

        void setFs(double a_newFs);

        /**
         * \brief The number of samples read and produced by the tick() method of the VoiceManager.
         */
        void setBufferSize(int a_bufferSize);

        /**
         * \brief Set the number of samples read and produced by the tick() method of the internal circuits.
         */
        void setInternalBufferSize(int a_internalBufferSize);
        int getInternalBufferSize() const { return m_internalBufferSize; }

        void setTempo(double a_newTempo);

        void noteOn(int a_noteNumber, int a_velocity);

        void noteOff(int a_noteNumber, int a_velocity);

        void sendControlChange(int a_cc, double a_newvalue);

        void setMaxVoices(int a_newMax);

        vector<int> getActiveVoiceIndices() const;
        vector<int> getReleasedVoiceIndices() const;
        vector<int> getIdleVoiceIndices() const;

        int getMaxVoices() const;

        int getLowestVoiceIndex(bool a_preferReleased = false) const;
        int getNewestVoiceIndex(bool a_preferReleased = false) const;
        int getOldestVoiceIndex(bool a_preferReleased = false) const;
        int getHighestVoiceIndex(bool a_preferReleased = false) const;

        void onIdle();

        /**
         * Retrieves a unit from a specific voice circuit.
         * If \p a_voiceId is negative, the unit is retrieved from the prototype circuit.
         */
        Unit& getUnit(int a_id, int a_voiceId = -1);
        const Unit& getUnit(int a_id, int a_voiceId = -1) const;

        Circuit* getPrototypeCircuit();
        const Circuit* getPrototypeCircuit() const;

        /**
         * Retrieve the specified voice circuit. Returns the prototype circuit if \p a_voiceId is negative.
         */
        Circuit* getVoiceCircuit(int a_voiceId);
        const Circuit* getVoiceCircuit(int a_voiceId) const;

        void setPrototypeCircuit(const Circuit& a_circ);

        VoiceStealingPolicy getVoiceStealingPolicy() const { return m_voiceStealingPolicy; }
        void setVoiceStealingPolicy(VoiceStealingPolicy a_newPolicy) { m_voiceStealingPolicy = a_newPolicy; }

        bool getLegato() const { return m_legato; }
        void setLegato(bool a_newLegato) { m_legato = a_newLegato; }

    private:
        /**
         * Processes all actions from the action queue
         */
        void _flushActionQueue();

        int _createVoice(int a_note, int a_velocity);

        void _makeIdle(int a_voiceIndex);

        int _stealIdleVoice();

    private:
        typedef std::vector<int> VoiceIndexList;
        typedef map<int, std::vector<int>> VoiceMap;

        spsc_queue<Command*> m_queuedActions;

        vector<Circuit> m_circuits;
        int m_bufferSize; ///< size of the buffers that will be written to by VoiceManager::tick
        int m_internalBufferSize; ///< size of the voice buffers that will be read from by VoiceManager::tick
        int m_tickCount;

        VoiceMap m_voiceMap; ///< maps midi notes to voice indices
        VoiceIndexList m_activeVoices; ///< list of active voice indices
        VoiceIndexList m_idleVoices; ///< list of idle voice indices
        VoiceIndexList m_garbageList; ///< pre-allocated storage for collecting idle voices during audio processing
        VoiceIndexList m_releasedVoices; ///< list of voices that are active but have been sent a note off signal
        Circuit m_instrument;

        VoiceStealingPolicy m_voiceStealingPolicy; ///< Determines which voices are replaced when all of them are active

        bool m_legato; ///< When true, voices get reset upon activation only if they are in the "note off" state.
    };
}
#endif
