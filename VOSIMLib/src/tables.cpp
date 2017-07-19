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
#include "vosimlib/tables.h"
#include "vosimlib/DSPMath.h"
#include <math.h>

namespace syn
{
    ResampledTable::ResampledTable(const double* a_data, int a_size, const BlimpTable& a_blimp_table_online, const BlimpTable& a_blimp_table_offline) :
        NormalTable(a_data, a_size),
        m_num_resampled_tables(0),
        m_resampled_sizes(0),
        m_resampled_tables(0),
        m_blimp_table_online(a_blimp_table_online),
        m_blimp_table_offline(a_blimp_table_offline)
    {
        resample_tables();
    }

    void ResampledTable::resample_tables() {
        /* Construct resampled tables at ratios of powers of K */
        m_num_resampled_tables = MAX<double>(1, log2(1.0*size) - 2);
        m_resampled_sizes.resize(m_num_resampled_tables);
        m_resampled_tables.resize(m_num_resampled_tables);
        double currsize = size;
        for (int i = 0; i < m_num_resampled_tables; i++) {
            m_resampled_sizes[i] = currsize;
            m_resampled_tables[i].resize(currsize);
            resample_table(m_data, size, &m_resampled_tables[i][0], m_resampled_sizes[i], m_blimp_table_offline);
            currsize *= 0.5;
        }
    }

    double ResampledTable::getresampled(double phase, double period) const {
        int min_size_diff = -1;
        int table_index = 0;
        for (int i = 0; i < m_num_resampled_tables; i++) {
            int curr_size_diff = m_resampled_sizes[i] - static_cast<int>(period);
            if (curr_size_diff < 0) {
                break;
            }
            if (curr_size_diff < min_size_diff || min_size_diff == -1) {
                min_size_diff = curr_size_diff;
                table_index = i;
            }
        }
        //        int table_index = CLAMP<int>(log2(period) - log2(m_size), 0, m_num_resampled_tables-1);
        return getresampled_single(&m_resampled_tables[table_index][0], m_resampled_sizes[table_index], phase, period, m_blimp_table_online);
    }

    void resample_table(const double* table, int size, double* resampled_table, double period, const BlimpTable& blimp_table, bool normalize) {
        double phase = 0;
        double phase_step = 1. / period;
        double input_energy = 0.0;
        double output_energy = 0.0;

        for (int i = 0; i < period; i++, phase += phase_step) {
            resampled_table[i] = getresampled_single(table, size, phase, period, blimp_table);
            if (normalize)
                output_energy += resampled_table[i] * resampled_table[i];
        }
        /* normalize */
        if (normalize) {
            for (int i = 0; i < size; i++)
            {
                input_energy += table[i] * table[i];
            }
            double input_power = input_energy / size;
            double output_power = output_energy / period;
            double norm_gain = sqrt(input_power/output_power);
            for (int i = 0; i < period; i++) {
                resampled_table[i] = resampled_table[i] * norm_gain;
            }
        }
    }

    void fft_resample_table(const double* table, int size, double* resampled_table, double period)
    {
    }

    double getresampled_single(const double* table, int size, double phase, double newSize, const BlimpTable& blimp_table) {
        double ratio = newSize * (1.0 / size);
        phase = WRAP(phase, 1.0)*size;

        double blimp_step;
        if (ratio < 1.0)
            blimp_step = static_cast<double>(blimp_table.res) * ratio;
        else
            blimp_step = static_cast<double>(blimp_table.res);

        int index = static_cast<int>(phase);
        double offset = (phase - index) * blimp_step;
        double output = 0.0;
        double filt_sum = 0.0; // used to make the filter unity gain

        // Backward pass
        double bkwd_filt_phase = offset;
        int bkwd_table_index = index;
        while (bkwd_filt_phase < blimp_table.size) {
#if DO_LERP_FOR_SINC
            double bkwd_filt_sample = blimp_table.plerp(bkwd_filt_phase / blimp_table.size);
#else
            double bkwd_filt_sample = blimp_table[static_cast<int>(bkwd_filt_phase)];
#endif
            if (bkwd_table_index < 0) {
                bkwd_table_index = size - 1;
            }
            output += bkwd_filt_sample * table[bkwd_table_index--];
            bkwd_filt_phase += blimp_step;
            filt_sum += bkwd_filt_sample;
        }

        // Forward pass
        double fwd_filt_phase = blimp_step - offset;
        int fwd_table_index = index + 1;
        while (fwd_filt_phase < blimp_table.size) {
#if DO_LERP_FOR_SINC
            double fwd_filt_sample = blimp_table.plerp(fwd_filt_phase / blimp_table.size);
#else
            double fwd_filt_sample = blimp_table[static_cast<int>(fwd_filt_phase)];
#endif
            if (fwd_table_index >= size) {
                fwd_table_index = 0;
            }
            output += fwd_filt_sample * table[fwd_table_index++];
            fwd_filt_phase += blimp_step;
            filt_sum += fwd_filt_sample;
        }

        return output / filt_sum;
    }
}