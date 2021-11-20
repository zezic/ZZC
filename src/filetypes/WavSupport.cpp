/*
 * Adopted from https://github.com/surge-synthesizer/surge
 */

/*
** Portable (using standard fread and so on) support for .wav files generating
** wavetables.
**
** Two things which matter in addition to the fmt and data block
**
**  1: The `smpl` block
**  2: the `clm ` block - indicates a serum file
**  3: the 'cue' block which apparently NI uses
**
** I read them in this order
**
**  1. If there is a clm block that wins, we ignore the smpl and cue block, and you get your 2048 wt
**  2. If there is a cue block and no clm, use the offsets if they are power of 2 and regular
**  3. Else if there is no smpl block and you have a smpl block if you have a power of 2 sample
*length
**     we interpret you as a wt
**  4. otherwise as a one shot or - right now-  an error
*/

#define WAV_STDOUT_INFO 1

#include "WavSupport.hpp"
#include "../dsp/Wavetable.hpp"

#include <mutex>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <vector>

// Sigh - lets write a portable ntol by hand
unsigned int pl_int(char *d)
{
    return (unsigned char)d[0] + (((unsigned char)d[1]) << 8) + (((unsigned char)d[2]) << 16) +
           (((unsigned char)d[3]) << 24);
}

unsigned short pl_short(char *d) { return (unsigned char)d[0] + (((unsigned char)d[1]) << 8); }

bool four_chars(char *v, char a, char b, char c, char d)
{
    return v[0] == a && v[1] == b && v[2] == c && v[3] == d;
}

bool SurgeStorage::load_wt(std::string filename, Wavetable *wt)
{
    wt->queue_filename[0] = 0;
    std::string extension = filename.substr(filename.find_last_of('.'), filename.npos);
    for (unsigned int i = 0; i < extension.length(); i++)
        extension[i] = tolower(extension[i]);
    bool loaded = false;
    if (extension.compare(".wt") == 0)
        loaded = load_wt_wt(filename, wt);
    else if (extension.compare(".wav") == 0)
        loaded = load_wt_wav_portable(filename, wt);
    else
    {
        std::cout << "Unable to load file with extension " << extension
            << "! Surge only supports .wav and .wt wavetable files!" << std::endl;
    }
    return loaded;
}

bool SurgeStorage::load_wt_wt(std::string filename, Wavetable *wt)
{
    FILE *f = fopen(filename.c_str(), "rb");
    if (!f)
        return false;
    wt_header wh;
    memset(&wh, 0, sizeof(wt_header));

    size_t read __attribute__((unused)) = fread(&wh, sizeof(wt_header), 1, f);
    // I'm not sure why this ever worked but it is checking the 4 bytes against vawt so...
    // if (wh.tag != vt_read_int32BE('vawt'))
    if (!(wh.tag[0] == 'v' && wh.tag[1] == 'a' && wh.tag[2] == 'w' && wh.tag[3] == 't'))
    {
        // SOME sort of error reporting is appropriate
        fclose(f);
        return false;
    }

    void *data;
    size_t ds;
    if (vt_read_int16LE(wh.flags) & wtf_int16)
        ds = sizeof(short) * vt_read_int16LE(wh.n_tables) * vt_read_int32LE(wh.n_samples);
    else
        ds = sizeof(float) * vt_read_int16LE(wh.n_tables) * vt_read_int32LE(wh.n_samples);

    data = malloc(ds);
    read = fread(data, 1, ds, f);
    // FIXME - error if read != ds

    waveTableDataMutex.lock();
    bool wasBuilt = wt->BuildWT(data, wh, false);
    waveTableDataMutex.unlock();
    free(data);

    if (!wasBuilt)
    {
        std::cout << "Wavetable could not be built, which means it has too many samples or frames."
            << " You provided " << wh.n_tables << " frames of " << wh.n_samples
            << "samples, while limit is " << max_subtables << " frames and " << max_wtable_size
            << " samples."
            << " In some cases, Surge detects this situation inconsistently. Surge is now in a "
               "potentially "
            << " inconsistent state. It is recommended to restart Surge and not load the "
               "problematic wavetable again."
            << " If you would like, please attach the wavetable which caused this message to a new "
               "GitHub issue at "
            << " https://github.com/surge-synthesizer/surge/" << std::endl;
        fclose(f);
        return false;
    }
    fclose(f);
    return true;
}

bool SurgeStorage::load_wt_wav_portable(std::string fn, Wavetable *wt)
{
    std::string uitag = "Wavetable Import Error";
#if WAV_STDOUT_INFO
    std::cout << "Loading wt_wav_portable" << std::endl;
    std::cout << "  fn='" << fn << "'" << std::endl;
#endif

    FILE *fp = fopen(fn.c_str(), "rb");
    if (!fp)
    {
        std::cout << "Unable to open file '" << fn << "'!" << std::endl;
        return false;
    }
    FcloseGuard closeOnReturn(fp);

    char riff[4], szd[4], wav[4];
    auto hds = fread(riff, 1, 4, fp);
    hds += fread(szd, 1, 4, fp);
    hds += fread(wav, 1, 4, fp);
    if (hds != 12)
    {
        std::cout << "'" << fn << "' does not contain a valid RIFF header chunk!" << std::endl;
        return false;
    }

    if (!four_chars(riff, 'R', 'I', 'F', 'F') && !four_chars(wav, 'W', 'A', 'V', 'E'))
    {
        std::cout << "'" << fn << "' is not a standard RIFF/WAVE file. Header is: " << riff[0] << riff[1]
            << riff[2] << riff[3] << " " << wav[0] << wav[1] << wav[2] << wav[3] << "." << std::endl;
        return false;
    }

    // WAV HEADER
    unsigned short audioFormat, numChannels;
    unsigned int sampleRate __attribute__((unused)), byteRate __attribute__((unused));
    unsigned short blockAlign __attribute__((unused)), bitsPerSample;

    // Result of data read
    bool hasSMPL = false;
    bool hasCLM = false;
    ;
    bool hasCUE = false;
    bool hasSRGE = false;
    bool hasSRGO = false;
    int clmLEN = 0;
    int smplLEN = 0;
    int cueLEN = 0;
    int srgeLEN = 0;

    // Now start reading chunks
    int tbr = 4;
    char *wavdata = nullptr;
    int datasz __attribute__((unused)) = 0, datasamples __attribute__((unused)) = 0;
    while (true)
    {
        char chunkType[4], chunkSzD[4];
        int br;
        if (!((br = fread(chunkType, 1, 4, fp)) == 4))
        {
            break;
        }
        br = fread(chunkSzD, 1, 4, fp);
        // FIXME - deal with br
        int cs = pl_int(chunkSzD);

#if WAV_STDOUT_INFO
        std::cout << "  CHUNK  `";
        for (int i = 0; i < 4; ++i)
            std::cout << chunkType[i];
        std::cout << "`  sz=" << cs << std::endl;
#endif
        tbr += 8 + cs;

        char *data = (char *)malloc(cs);
        br = fread(data, 1, cs, fp);
        if (br != cs)
        {
            free(data);

            break;
        }

        if (four_chars(chunkType, 'f', 'm', 't', ' '))
        {
            char *dp = data;
            audioFormat = pl_short(dp);
            dp += 2; // 1 is PCM; 3 is IEEE Float
            numChannels = pl_short(dp);
            dp += 2;
            sampleRate = pl_int(dp);
            dp += 4;
            byteRate = pl_int(dp);
            dp += 4;
            blockAlign = pl_short(dp);
            dp += 2;
            bitsPerSample = pl_short(dp);
            dp += 2;

#if WAV_STDOUT_INFO
            std::cout << "     FMT=" << audioFormat << " x " << numChannels << " at "
                      << bitsPerSample << " bits" << std::endl;
#endif

            free(data);

            // Do a format check here to bail out
            if (!((numChannels == 1) &&
                  (((audioFormat == 1 /* WAVE_FORMAT_PCM */) && (bitsPerSample == 16)) ||
                   ((audioFormat == 3 /* IEEE_FLOAT */) && (bitsPerSample == 32)))))
            {
                std::string formname = "Unknown (" + std::to_string(audioFormat) + ")";
                if (audioFormat == 1)
                    formname = "PCM";
                if (audioFormat == 3)
                    formname = "float";

                std::cout << "Currently, Surge only supports 16-bit PCM or 32-bit float mono WAV files. "
                    << " You provided a " << bitsPerSample << "-bit " << formname << " "
                    << numChannels << "-channel file." << std::endl;

                return false;
            }
        }
        else if (four_chars(chunkType, 'c', 'l', 'm', ' '))
        {
            // These all begin '<!>dddd' where d is 2048 it seems
            char *dp = data + 3;
            if (four_chars(dp, '2', '0', '4', '8'))
            {
                // 2048 CLM detected
                hasCLM = true;
                clmLEN = 2048;
            }
            free(data);
        }
        else if (four_chars(chunkType, 'u', 'h', 'W', 'T'))
        {
            // This is HIVE metadata so treat it just like CLM / Serum
            hasCLM = true;
            clmLEN = 2048;

            free(data);
        }
        else if (four_chars(chunkType, 's', 'r', 'g', 'e'))
        {
            hasSRGE = true;
            char *dp = data;
            int version __attribute__((unused)) = pl_int(dp);
            dp += 4;
            srgeLEN = pl_int(dp);
            free(data);
        }
        else if (four_chars(chunkType, 's', 'r', 'g', 'o'))
        {
            hasSRGO = true;
            char *dp = data;
            int version __attribute__((unused)) = pl_int(dp);
            dp += 4;
            srgeLEN = pl_int(dp);
            free(data);
        }
        else if (four_chars(chunkType, 'c', 'u', 'e', ' '))
        {
            char *dp = data;
            int numCues = pl_int(dp);
            dp += 4;
            std::vector<int> chunkStarts;
            for (int i = 0; i < numCues; ++i)
            {
                for (int j = 0; j < 6; ++j)
                {
                    auto d = pl_int(dp);
                    if (j == 5)
                        chunkStarts.push_back(d);

                    dp += 4;
                }
            }

            // Now are my chunkstarts regular
            int d = -1;
            bool regular = true;
            for (size_t i = 1; i < chunkStarts.size(); ++i)
            {
                if (d == -1)
                    d = chunkStarts[i] - chunkStarts[i - 1];
                else
                {
                    if (d != chunkStarts[i] - chunkStarts[i - 1])
                        regular = false;
                }
            }

            if (regular)
            {
                hasCUE = true;
                cueLEN = d;
            }

            free(data);
        }
        else if (four_chars(chunkType, 'd', 'a', 't', 'a'))
        {
            datasz = cs;
            datasamples = cs * 8 / bitsPerSample / numChannels;
            wavdata = data;
        }
        else if (four_chars(chunkType, 's', 'm', 'p', 'l'))
        {
            char *dp = data;
            unsigned int samplechunk[9];
            for (int i = 0; i < 9; ++i)
            {
                samplechunk[i] = pl_int(dp);
                dp += 4;
            }
            unsigned int nloops = samplechunk[7];
            unsigned int sdsz = samplechunk[8];
#if WAV_STDOUT_INFO
            std::cout << "   SMPL: nloops=" << nloops << " " << sdsz << std::endl;
#endif

            if (nloops == 0)
            {
                // It seems RAPID uses a smpl block with no samples to indicate a 2048.
                hasSMPL = true;
                smplLEN = 2048;
            }

            if (nloops > 1)
            {
                // We don't support this. Indicate somehow.
                // FIXME
            }

            for (size_t i = 0; i < nloops && i < 1; ++i)
            {
                unsigned int loopdata[6];
                for (int j = 0; j < 6; ++j)
                {
                    loopdata[j] = pl_int(dp);
                    dp += 4;
#if WAV_STDOUT_INFO
                    std::cout << "      loopdata[" << j << "] = " << loopdata[j] << std::endl;
#endif
                }
                hasSMPL = true;
                smplLEN = loopdata[3] - loopdata[2] + 1;

                if (smplLEN == 0)
                    smplLEN = 2048;
            }
        }
        else
        {
            /*std::cout << "Default Dump\n";
            for( int i=0; i<cs; ++i ) std::cout << data[i];
            std::cout << std::endl; */
            free(data);
        }
    }

#if WAV_STDOUT_INFO
    std::cout << "  hasCLM =" << hasCLM << " / " << clmLEN << std::endl;
    std::cout << "  hasCUE =" << hasCUE << " / " << cueLEN << std::endl;
    std::cout << "  hasSMPL=" << hasSMPL << "/" << smplLEN << std::endl;
    std::cout << "  hasSRGE=" << hasSRGE << "/" << srgeLEN << std::endl;
#endif

    bool loopData = hasSMPL || hasCLM || hasSRGE;
    int loopLen =
        hasCLM ? clmLEN : (hasCUE ? cueLEN : (hasSRGE ? srgeLEN : (hasSMPL ? smplLEN : -1)));
    if (loopLen == 0)
    {
        std::cout << "Surge cannot understand this particular .wav file. Please consult the Surge Wiki "
               "for"
            << " information on .wav file metadata." << std::endl;

        if (wavdata)
            free(wavdata);
        return false;
    }

    int loopCount = datasamples / loopLen;

#if WAV_STDOUT_INFO
    std::cout << "  samples=" << datasamples << " loopLen=" << loopLen << " loopCount=" << loopCount
              << std::endl;
#endif

    // wt format header (surge internal)
    wt_header wh;
    memset(&wh, 0, sizeof(wt_header));
    wh.flags = wtf_is_sample;

    int sh = 0;
    if (loopData)
    {
        wh.flags = 0;
        switch (loopLen)
        {
        case 4096:
            sh = 12;
            break;
        case 2048:
            sh = 11;
            break;
        case 1024:
            sh = 10;
            break;
        case 512:
            sh = 9;
            break;
        case 256:
            sh = 8;
            break;
        case 128:
            sh = 7;
            break;
        case 64:
            sh = 6;
            break;
        case 32:
            sh = 5;
            break;
        case 16:
            sh = 4;
            break;
        case 8:
            sh = 3;
            break;
        case 4:
            sh = 2;
            break;
        case 2:
            sh = 1;
            break;
        default:
#if WAV_STDOUT_INFO
            std::cout << "Setting style to sample" << std::endl;
#endif
            wh.flags = wtf_is_sample;
            break;
        }
    }

    if (loopLen != -1 && (sh == 0 || loopCount < 2))
    {
        std::cout << "Currently, Surge only supports wavetables with at least 2 frames of up to 4096 "
               "samples each in power-of-two increments."
            << " You provided a wavetable with " << loopCount
            << (loopCount == 1 ? " frame" : " frames") << " of " << loopLen << " samples. '" << fn
            << "'" << std::endl;

        if (wavdata)
            free(wavdata);
        return false;
    }

    wh.n_samples = 1 << sh;
    int mask __attribute__((unused)) = wt->size - 1;
    int sample_length = std::min(datasamples, max_wtable_size * max_subtables);
    wh.n_tables = std::min(max_subtables, (sample_length >> sh));

    if (wh.flags & wtf_is_sample)
    {
        auto windowSize = 1024;
        if (hasSRGO)
            windowSize = srgeLEN;

        while (windowSize * 4 > sample_length && windowSize > 8)
            windowSize = windowSize / 2;
        wh.n_samples = windowSize;
        wh.n_tables = (int)(sample_length / windowSize);
    }

    int channels __attribute__((unused)) = 1;

    if ((audioFormat == 1 /* WAVE_FORMAT_PCM */) && (bitsPerSample == 16) && numChannels == 1)
    {
        // assert(wh.n_samples * wh.n_tables * 2 <= datasz);
        wh.flags |= wtf_int16;
    }
    else if ((audioFormat == 3 /* WAVE_FORMAT_IEEE_FLOAT */) && (bitsPerSample == 32) &&
             numChannels == 1)
    {
        // assert(wh.n_samples * wh.n_tables * 4 <= datasz);
    }
    else
    {
        std::ostringstream oss;
        std::cout << "Currently, Surge only supports 16-bit PCM or 32-bit float mono .wav files. "
            << " You provided a " << bitsPerSample << "-bit" << audioFormat << " " << numChannels
            << "-channel file." << std::endl;

        if (wavdata)
            free(wavdata);
        return false;
    }

    if (wavdata && wt)
    {
        waveTableDataMutex.lock();
        wt->BuildWT(wavdata, wh, wh.flags & wtf_is_sample);
        waveTableDataMutex.unlock();
        free(wavdata);
    }
    return true;
}
