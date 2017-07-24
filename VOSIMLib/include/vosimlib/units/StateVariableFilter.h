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

/**
* \file units/StateVariableFilter.h
* \brief
* \details
* \author Austen Satterlee
* \date March 11, 2016
*/
#ifndef __STATEVARIABLEFILTER__
#define __STATEVARIABLEFILTER__
#include "vosimlib/Unit.h"

namespace syn
{
    class VOSIMLIB_API StateVariableFilter : public Unit
    {
        DERIVE_UNIT(StateVariableFilter)
    public:

        const int c_oversamplingFactor = 8;

        explicit StateVariableFilter(const string& a_name);

        StateVariableFilter(const StateVariableFilter& a_rhs) :
            StateVariableFilter(a_rhs.name()) {}

        virtual void reset() override;

    protected:

        void process_() override;
        void onNoteOn_() override;

        int m_pFc, m_pRes;
        int m_iResAdd, m_iResMul;
        int m_iFcAdd, m_iFcMul;
        int m_oLP, m_oBP, m_oN, m_oHP;
        double m_prevBPOut, m_prevLPOut;
        double m_F, m_damp;

        const double c_minRes = 1.0;
        const double c_maxRes = 10.0;
    };

    class VOSIMLIB_API TrapStateVariableFilter : public StateVariableFilter
    {
        DERIVE_UNIT(TrapStateVariableFilter)
    public:
        explicit TrapStateVariableFilter(const string& a_name);

        TrapStateVariableFilter(const TrapStateVariableFilter& a_rhs) :
            TrapStateVariableFilter(a_rhs.name()) {}

        virtual void reset() override;
    protected:
        void process_() override;

    protected:
        double m_prevInput;
    };

    /**
     * 1 Pole "TPT" implementation
     */
    struct OnePoleLP
    {
        /**
         * Set forward gain by specifying cutoff frequency and sampling rate.
         */
        void setFc(double a_fc, double a_fs);

        double process(double a_input);

        void reset();

        double m_state = 0.0;
        double m_G = 0.0; /// feed forward gain
    };


    /**
     * 1 Pole "TPT" unit wrapper
     */
    class VOSIMLIB_API OnePoleLPUnit : public Unit
    {
        DERIVE_UNIT(OnePoleLPUnit)
    public:
        enum Param
        {
            pFc = 0
        };

        enum Input
        {
            iAudioIn = 0,
            iFcAdd,
            iFcMul,
            iSync
        };

        enum Output
        {
            oLP = 0,
            oHP = 1
        };

        explicit OnePoleLPUnit(const string& a_name);

        OnePoleLPUnit(const OnePoleLPUnit& a_rhs) : OnePoleLPUnit(a_rhs.name()) {};

        double getState() const;

        virtual void reset() override;;

    protected:
        void process_() override;
        void onNoteOn_() override;
    private:
        OnePoleLP implem;
        double m_lastSync;
    };

    class LadderFilterBase : public Unit
    {
    public:
        explicit LadderFilterBase(const string& a_name);

    protected:
        void onNoteOn_() override;

    public:
        const int c_oversamplingFactor = 8;

        enum Param
        {
            pFc = 0,
            pFb,
            pDrv
        };

        enum Input
        {
            iAudioIn = 0,
            iFcAdd,
            iFcMul,
            iFbAdd,
            iDrvAdd
        };
    };

    class VOSIMLIB_API LadderFilterA : public LadderFilterBase
    {
        DERIVE_UNIT(LadderFilterA)
    public:
        explicit LadderFilterA(const string& a_name);
        LadderFilterA(const LadderFilterA& a_rhs) : LadderFilterA(a_rhs.name()) {};
        void reset() override;
    protected:
        void process_() override;
    protected:
        const double VT = 0.312;
        std::array<double, 5> m_V;
        std::array<double, 5> m_dV;
        std::array<double, 5> m_tV;
    };

    class VOSIMLIB_API LadderFilterB : public LadderFilterBase
    {
        DERIVE_UNIT(LadderFilterB)
    public:
        explicit LadderFilterB(const string& a_name);
        LadderFilterB(const LadderFilterB& a_rhs) : LadderFilterB(a_rhs.name()) {};
        void reset() override;
    protected:
        void process_() override;
    protected:
        typedef Eigen::Matrix<double, 1, 5> Vector5d;
        OnePoleLP m_LP[4];
        Vector5d m_ffGains; /// state feed forward gains
    };
}
#endif