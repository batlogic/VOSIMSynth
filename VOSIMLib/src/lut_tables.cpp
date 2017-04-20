#include "tables.h"
#include "lut_tables.h"

namespace syn {
    /*::lut_defs::*/
    BlimpTable& lut_blimp_table_offline() { static BlimpTable table(BLIMP_TABLE_OFFLINE, 263297, 257, 2048); return table; }
    BlimpTable& lut_blimp_table_online() { static BlimpTable table(BLIMP_TABLE_ONLINE, 11270, 11, 2048); return table; }
    AffineTable& lut_pitch_table() { static AffineTable table(PITCH_TABLE, 1024, -128, 256); return table; }
    ResampledTable& lut_bl_saw_table() { static ResampledTable table(BL_SAW_TABLE, 8193, lut_blimp_table_online(), lut_blimp_table_offline()); return table; }
    ResampledTable& lut_bl_square_table() { static ResampledTable table(BL_SQUARE_TABLE, 8193, lut_blimp_table_online(), lut_blimp_table_offline()); return table; }
    ResampledTable& lut_bl_tri_table() { static ResampledTable table(BL_TRI_TABLE, 8193, lut_blimp_table_online(), lut_blimp_table_offline()); return table; }
    NormalTable& lut_sin_table() { static NormalTable table(SIN_TABLE, 1024); return table; }
    /*::/lut_defs::*/
}
