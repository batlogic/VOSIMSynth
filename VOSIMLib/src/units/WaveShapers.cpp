#include "units/WaveShapers.h"

syn::TanhUnit::TanhUnit(const string& a_name) : Unit(a_name)
{
    addParameter_(pSat, UnitParameter("sat", 1.0, 10.0, 1.0));
    addInput_("in");
    addOutput_("out");
}

syn::TanhUnit::TanhUnit(const TanhUnit& a_rhs) : TanhUnit(a_rhs.name()) {}

void syn::TanhUnit::process_()
{
    BEGIN_PROC_FUNC
        double input = READ_INPUT(0);
        double sat = param(pSat).getDouble();
        WRITE_OUTPUT(0, fast_tanh_rat(input * sat) / fast_tanh_rat(sat));
    END_PROC_FUNC
}
