/*
 * Adopted from https://github.com/surge-synthesizer/surge
 */

#pragma once
#include <cstring>
#include <string>
const int max_wtable_size = 4096;
const int max_subtables = 512;
const int max_mipmap_levels = 16;
// I don't know why your max wtable samples would be less than your max tables * your max sample
// size. So lets fix that! This size is consistent with the check in WaveTable.cpp //
// CheckRequiredWTSize with ts and tc at 1024 and 512
const int max_wtable_samples = 2097152;
// const int max_wtable_samples =  268000; // delay pops 4 uses the most
//

#define vt_read_int32LE vt_write_int32LE
#define vt_read_int32BE vt_write_int32BE
#define vt_read_int16LE vt_write_int16LE
#define vt_read_float32LE vt_write_float32LE

inline int vt_write_int32LE(int t) { return t; }

inline float vt_write_float32LE(float f) { return f; }

inline int vt_write_int32BE(int t)
{
#if (ARCH_LIN || ARCH_MAC)
    // this was `swap_endian`:
    return ((t << 24) & 0xff000000) | ((t << 8) & 0x00ff0000) | ((t >> 8) & 0x0000ff00) |
           ((t >> 24) & 0x000000ff);
#else
    // return _byteswap_ulong(t);
    return __builtin_bswap32(t);
#endif
}

inline short vt_write_int16LE(short t) { return t; }

inline void vt_copyblock_W_LE(short *dst, const short *src, size_t count)
{
    memcpy(dst, src, count * sizeof(short));
}

inline void vt_copyblock_DW_LE(int *dst, const int *src, size_t count)
{
    memcpy(dst, src, count * sizeof(int));
}


#pragma pack(push, 1)
struct wt_header
{
    char tag[4];
    unsigned int n_samples;
    unsigned short n_tables;
    unsigned short flags;
};
#pragma pack(pop)

class Wavetable
{
  public:
    Wavetable();
    ~Wavetable();
    void Copy(Wavetable *wt);
    bool BuildWT(void *wdata, wt_header &wh, bool AppendSilence);
    void MipMapWT();

    void allocPointers(size_t newSize);

  public:
    int size;
    int n_tables;
    int size_po2;
    int flags;
    float dt;
    float *TableF32WeakPointers[max_mipmap_levels][max_subtables];
    short *TableI16WeakPointers[max_mipmap_levels][max_subtables];

    size_t dataSizes;
    float *TableF32Data;
    short *TableI16Data;

    int current_id, queue_id;
    bool refresh_display;
    char queue_filename[256];
};

enum wtflags
{
    wtf_is_sample = 1,
    wtf_loop_sample = 2,
    wtf_int16 = 4,       // If this is set we have int16 in range 0-2^15
    wtf_int16_is_16 = 8, // and in this case, range 0-2^16 if with above
};
