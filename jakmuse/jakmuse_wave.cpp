/*
Copyright (c) 2015, Vlad Mesco
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
Copyright (c) 2014-2015, Vlad Mesco
All rights reserved.
Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <cstdint>
#include <cstdio>
#include <vector>
#include <string>
#include <stdexcept>
#include <exception>

static void wav_write_header(FILE* f, unsigned samplPerSec, unsigned numSamples)
{
    // for PCM: M = 2
#define M 4
#define NC 1
#define NS numSamples
#define F samplPerSec
#define WAVE_FORMAT_PCM 0x0001
#define WAVE_FORMAT_IEEE_FLOAT 0x0003

    struct wav_header_s {
        uint32_t ckId;
        uint32_t cksize;
        uint32_t WAVEID;
        uint32_t fmt_ckId;
        uint32_t fmt_cksize;
        uint16_t wFormatTag;
        uint16_t nChannels;
        uint32_t nSamplesPerSec;
        uint32_t nAvgBytesPerSec;
        uint16_t nBlockAlign;
        uint16_t wBitsPerSample;
        uint32_t data_factCkId;
        uint32_t data_factCksize;
        uint32_t data_factDwSampleLength;
        uint32_t data_ckId;
        uint32_t data_cksize;
    } header = {
        'FFIR', // little endian RIFF
        4 + 26 + 12 + (8 + M /*B per sample*/ * NC /*N channels*/ * NS + 0),
        'EVAW', // little endian WAVE
        ' tmf', // little endian fmt 
        16,
        WAVE_FORMAT_IEEE_FLOAT, // 1 = PCM, 3 = float
        NC,
        F,
        F * M * NC,
        M * NC,
        8 * M,
        'tcaf', // little endian fact
        4,
        NC * NS,
        'atad', // little endian data
        M * NC * NS,
    };

    size_t hr = fwrite(&header, sizeof(wav_header_s), 1, f);
    if(hr != 1 || ferror(f)) {
        throw std::runtime_error(std::string(strerror(errno)));
    }
#undef M
#undef NC
#undef NS
#undef F
}

static void wav_write_samples(FILE* f, std::vector<float> const& samples)
{
    size_t hr = fwrite(samples.data(), sizeof(float), samples.size(), f);
    if(hr != samples.size() || ferror(f)) {
        throw std::runtime_error(std::string(strerror(errno)));
    }
}

void wav_write_file(std::string const& filename, std::vector<float> const& samples, unsigned samples_per_second)
{
    FILE* f = fopen(filename.c_str(), "wb");
    if(!f) {
        throw std::invalid_argument(std::string() + "Failed to open " + filename + " for writing");
    }

    try {
        clearerr(f);
        wav_write_header(f, samples_per_second, samples.size());
        wav_write_samples(f, samples);
    } catch(std::exception e) {
        fclose(f);
        throw std::runtime_error(std::string() + "Failed to write wave file: " + e.what());
    }

    fclose(f);
}

#ifdef TEST_WAVE
#include <cmath>
int main()
{
    // generate a sine
    std::vector<float> thing;
    for(size_t i = 0; i < 44100 * 2; ++i) {
        thing.push_back(sin(3.14159f * 2.0 * 440.0 / 44100.0 * i));
    }

    (void) wav_write_file("test.wav", thing, 44100);
}
#endif
