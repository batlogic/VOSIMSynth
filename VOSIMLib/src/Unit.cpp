#include "Unit.h"

using std::hash;

using namespace std;

namespace syn {

    Unit::Unit() :
            m_parent{nullptr},
            m_name{""},
            m_hasTicked(false)
    { }

    Unit::Unit(const string& a_name) :
            m_parent{nullptr},
            m_name{a_name},
            m_hasTicked(false)
    { }

    unsigned int Unit::getClassIdentifier() const
    {
        hash<string> hash_fn;
        return hash_fn(_getClassName());
    }

    const Circuit* const Unit::getParent() const
    {
        return m_parent;
    }

    int Unit::getNumParameters() const
    {
        return m_parameters.size();
    }

    int Unit::getNumInputs() const
    {
        return m_inputSignals.getNumChannels();
    }

    int Unit::getNumOutputs() const
    {
        return m_outputSignals.getNumChannels();
    }

    const string& Unit::getName() const
    {
        return m_name;
    }

    void Unit::_setName(const string& a_name)
    {
        m_name = a_name;
    }

    void Unit::tick()
    {
        if (m_hasTicked)
            return;
        m_hasTicked = true;

        /* Pull signals from inputs */
        for (int i = 0 ; i < m_inputSignals.getNumChannels() ; i++) {
            Signal& inputChannel = m_inputSignals.getChannel(i);
            m_inputConnections.pull(i, inputChannel);
        }

        m_outputSignals.clear();

        process_(m_inputSignals, m_outputSignals);
    }

    void Unit::reset()
    {
        m_inputSignals.clear();
        m_hasTicked = false;
    }

    int Unit::addInput_(const string& a_name, double a_default, Signal::ChannelAccType a_accType)
    {
        int inputId = m_inputSignals.addChannel(a_name);
		m_inputSignals.getChannel(inputId).setDefault(a_default);
		m_inputSignals.getChannel(inputId).setChannelAccType(a_accType);
		return inputId;
    }

    int Unit::addOutput_(const string& a_name)
    {
        return m_outputSignals.addChannel(a_name);
    }

    int Unit::addParameter_(const UnitParameter& a_param)
    {
        int indx = m_parameters.add(a_param.getName(), a_param);
        return m_parameters.getItemIndex(indx);
    }

    void Unit::_setParent(Circuit* a_new_parent)
    {
        m_parent = a_new_parent;
    }

    bool Unit::_connectInput(shared_ptr<Unit> a_fromUnit, int a_fromOutputPort, int a_toInputPort)
    {
        return m_inputConnections.connect(a_fromUnit, a_fromOutputPort, a_toInputPort);
    }

    bool Unit::_disconnectInput(shared_ptr<Unit> a_fromUnit, int a_fromOutputPort, int a_toInputPort)
    {
        return m_inputConnections.disconnect(a_fromUnit, a_fromOutputPort, a_toInputPort);
    }

    bool Unit::_disconnectInput(shared_ptr<Unit> a_fromUnit){
        return m_inputConnections.disconnect(a_fromUnit);
    }

    void Unit::setFs(double a_newFs)
    {
        m_audioConfig.fs = a_newFs;
        onFsChange_();
    }

    void Unit::setTempo(double a_newTempo)
    {
        m_audioConfig.tempo = a_newTempo;
        onTempoChange_();
    }

    void Unit::noteOn(int a_note, int a_velocity)
    {
        m_midiData.note = a_note;
        m_midiData.velocity = a_velocity;
        m_midiData.isNoteOn = true;
        onNoteOn_();
    }

    void Unit::noteOff(int a_note, int a_velocity)
    {
        m_midiData.isNoteOn = false;
        onNoteOff_();
    }

    double Unit::getFs() const
    {
        return m_audioConfig.fs;
    }

    double Unit::getTempo() const
    {
        return m_audioConfig.tempo;
    }

	bool Unit::isNoteOn() const {
		return m_midiData.isNoteOn;
    }

	int Unit::getNote() const
    {
        return m_midiData.note;
    }

    int Unit::getVelocity() const
    {
        return m_midiData.velocity;
    }

    bool Unit::isActive() const
    {
        return m_midiData.isNoteOn;
    }

    Unit* Unit::clone() const
    {
        Unit* unit = _clone();

        unit->m_name=m_name;
        unit->m_inputSignals=m_inputSignals;
        unit->m_outputSignals=m_outputSignals;
        unit->m_parameters=m_parameters;
        unit->m_midiData=m_midiData;
        unit->setFs(m_audioConfig.fs);
        unit->setTempo(m_audioConfig.tempo);
        return unit;
    }
}

