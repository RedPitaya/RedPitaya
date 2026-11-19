/*
Copyright (c) 2003-2010, Mark Borgerding

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the author nor the names of any contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// KISS FFT reduced to what the spectrum of the preview needs (from rp-api/api-dsp/src/kiss_fft):
// the forward FFT of a real signal whose length is a power of two. No inverse, no other radix
// than 2 and 4, the tables owned by the object.

#ifndef RPSA_DSP_KISS_FFT_H
#define RPSA_DSP_KISS_FFT_H

#include <cstddef>
#include <vector>

namespace dsp {

struct Cpx {
    float r = 0;
    float i = 0;
};

// The forward FFT of n real samples, n a power of two of 4 at least: n / 2 + 1 complex values
class RealFft {
   public:
    // false when n is not a power of two of 4 at least
    auto init(size_t n) -> bool;
    auto size() const -> size_t { return m_n; }
    // in: n samples, out: n / 2 + 1 values
    auto run(const float* in, Cpx* out) -> void;

   private:
    auto complexFft(const Cpx* in, Cpx* out) -> void;
    auto work(Cpx* out, const Cpx* in, size_t fstride, const int* factors) -> void;

    size_t m_n = 0;                  // real samples
    size_t m_half = 0;               // the complex FFT inside: n / 2
    std::vector<int> m_factors;      // radix, stage length, ...
    std::vector<Cpx> m_twiddles;     // of the complex FFT
    std::vector<Cpx> m_superTwiddles;  // of the split into the real spectrum
    std::vector<Cpx> m_tmp;
};

}  // namespace dsp

#endif
