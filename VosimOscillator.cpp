#include "VosimOscillator.h"

/******************************
* VOSIM methods
*
******************************/
namespace syn
{

  VosimOscillator::VosimOscillator(const VosimOscillator& vosc) :
    VosimOscillator(vosc.m_name)
  {
    m_CurrPulseGain = vosc.m_Step;
    m_PhaseScale = vosc.m_PhaseScale;
    m_LastPulsePhase = vosc.m_LastPulsePhase;
  }

  double VosimOscillator::process()
  {
    Oscillator::tick_phase();
    double number = m_number * 4;
    if (m_number.isDirty() || m_pitch.isDirty() || m_pitchshift.isDirty() || m_ppitch.isDirty() || m_relativeamt.isDirty())
    {
      double pulseshift = (m_pitch + m_pitchshift + 12 * (number - 1))*m_relativeamt;
      m_PhaseScale = pitchToFreq(m_ppitch * (128 - pulseshift) + pulseshift) / (m_Step*m_Fs);
    }

    // add compensation for phase scales < 0.5 (i.e. won't be able to reach pulse peak)
    m_CompensationGain = 1.0;
    if (m_PhaseScale < 0.5)
    {
      double lastvalue = lut_vosim_pulse_cos.getlinear(m_PhaseScale);
      if (lastvalue)
        m_CompensationGain = (1. / lastvalue);
    }
    else if (number > 0 && number <= 0.5)
    {
      double lastvalue = lut_vosim_pulse_cos.getlinear(number);
      if (lastvalue)
        m_CompensationGain = (1. / lastvalue);
    }

    double vout;
    double pulsePhase = m_phase * m_PhaseScale;
    int N = (int)pulsePhase;
    int lastN = (int)m_LastPulsePhase;
    if (!N)
    {
      m_CurrPulseGain = 1.0;
    }
    else if (N != lastN)
    {
      m_CurrPulseGain *= m_decay;
    }
    double wrPulsePhase = (pulsePhase - N);
    if (pulsePhase >= number)
    {
      vout = 0;
    }
    else
    {
      double tableval = lut_vosim_pulse_cos.getlinear(wrPulsePhase);
      _ASSERT(1. / m_CompensationGain > tableval);
      vout = m_CurrPulseGain*m_CompensationGain*tableval;
    }
    m_LastPulsePhase = pulsePhase;
    return m_velocity*m_gain*vout;
  }
}