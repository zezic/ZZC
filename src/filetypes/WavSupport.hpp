/*
 * Adopted from https://github.com/surge-synthesizer/surge
 */

#pragma once

#include "../dsp/Wavetable.hpp"

#include <mutex>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <vector>

unsigned int pl_int(char *d);

unsigned short pl_short(char *d);

bool four_chars(char *v, char a, char b, char c, char d);

struct FcloseGuard
{
    FILE *fp = nullptr;
    FcloseGuard(FILE *f) { fp = f; }
    ~FcloseGuard()
    {
        if (fp)
        {
            fclose(fp);
        }
    }
};

struct SurgeStorage {
    std::mutex waveTableDataMutex;

    bool load_wt(std::string filename, Wavetable *wt);
    bool load_wt_wt(std::string filename, Wavetable *wt);
    bool load_wt_wav_portable(std::string fn, Wavetable *wt);
};
