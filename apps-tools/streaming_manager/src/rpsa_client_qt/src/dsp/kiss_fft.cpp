/*
Copyright (c) 2003-2010, Mark Borgerding

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the author nor the names of any contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "kiss_fft.h"
#include <cmath>

namespace dsp {

namespace {

constexpr double PI = 3.14159265358979323846264338327950288;

inline auto mul(const Cpx& a, const Cpx& b) -> Cpx {
    return {a.r * b.r - a.i * b.i, a.r * b.i + a.i * b.r};
}

inline auto expi(double phase) -> Cpx {
    return {static_cast<float>(std::cos(phase)), static_cast<float>(std::sin(phase))};
}

// the butterflies of a forward FFT, radix 2 and 4
void bfly2(Cpx* out, size_t fstride, const Cpx* tw, size_t m) {
    Cpx* out2 = out + m;
    for (size_t k = 0; k < m; k++, tw += fstride) {
        const Cpx t = mul(out2[k], *tw);
        out2[k] = {out[k].r - t.r, out[k].i - t.i};
        out[k] = {out[k].r + t.r, out[k].i + t.i};
    }
}

void bfly4(Cpx* out, size_t fstride, const Cpx* twiddles, size_t m) {
    const Cpx *tw1 = twiddles, *tw2 = twiddles, *tw3 = twiddles;
    for (size_t k = 0; k < m; k++, out++, tw1 += fstride, tw2 += 2 * fstride, tw3 += 3 * fstride) {
        const Cpx s0 = mul(out[m], *tw1);
        const Cpx s1 = mul(out[2 * m], *tw2);
        const Cpx s2 = mul(out[3 * m], *tw3);
        const Cpx s5 = {out->r - s1.r, out->i - s1.i};
        const Cpx a = {out->r + s1.r, out->i + s1.i};
        const Cpx s3 = {s0.r + s2.r, s0.i + s2.i};
        const Cpx s4 = {s0.r - s2.r, s0.i - s2.i};
        out[2 * m] = {a.r - s3.r, a.i - s3.i};
        *out = {a.r + s3.r, a.i + s3.i};
        out[m] = {s5.r + s4.i, s5.i - s4.r};
        out[3 * m] = {s5.r - s4.i, s5.i + s4.r};
    }
}

}  // namespace

auto RealFft::init(size_t n) -> bool {
    if (n < 4 || (n & (n - 1)) != 0)
        return false;
    if (n == m_n)
        return true;
    m_n = n;
    m_half = n / 2;
    // the stages: radix 4 while it divides, then radix 2
    m_factors.clear();
    size_t rest = m_half;
    while (rest > 1) {
        const size_t p = rest % 4 == 0 ? 4 : 2;
        rest /= p;
        m_factors.push_back(static_cast<int>(p));
        m_factors.push_back(static_cast<int>(rest));
    }
    m_twiddles.resize(m_half);
    for (size_t i = 0; i < m_half; i++)
        m_twiddles[i] = expi(-2 * PI * static_cast<double>(i) / static_cast<double>(m_half));
    m_superTwiddles.resize(m_half / 2);
    for (size_t i = 0; i < m_half / 2; i++)
        m_superTwiddles[i] = expi(-PI * (static_cast<double>(i + 1) / static_cast<double>(m_half) + 0.5));
    m_tmp.resize(m_half);
    return true;
}

auto RealFft::work(Cpx* out, const Cpx* in, size_t fstride, const int* factors) -> void {
    const size_t p = static_cast<size_t>(*factors++);  // the radix
    const size_t m = static_cast<size_t>(*factors++);  // the length of the stage / p
    Cpx* const begin = out;
    Cpx* const end = out + p * m;
    if (m == 1) {
        for (; out != end; out++, in += fstride)
            *out = *in;
    } else {
        // p FFTs of length m, each of every p-th input
        for (; out != end; out += m, in += fstride)
            work(out, in, fstride * p, factors);
    }
    if (p == 4)
        bfly4(begin, fstride, m_twiddles.data(), m);
    else
        bfly2(begin, fstride, m_twiddles.data(), m);
}

auto RealFft::complexFft(const Cpx* in, Cpx* out) -> void {
    work(out, in, 1, m_factors.data());
}

// the real samples as m_half complex ones (even, odd), their FFT, then split into the spectrum
auto RealFft::run(const float* in, Cpx* out) -> void {
    complexFft(reinterpret_cast<const Cpx*>(in), m_tmp.data());
    const size_t h = m_half;
    const Cpx dc = m_tmp[0];
    out[0] = {dc.r + dc.i, 0};
    out[h] = {dc.r - dc.i, 0};
    for (size_t k = 1; k <= h / 2; k++) {
        const Cpx fpk = m_tmp[k];
        const Cpx fpnk = {m_tmp[h - k].r, -m_tmp[h - k].i};
        const Cpx f1k = {fpk.r + fpnk.r, fpk.i + fpnk.i};
        const Cpx f2k = {fpk.r - fpnk.r, fpk.i - fpnk.i};
        const Cpx tw = mul(f2k, m_superTwiddles[k - 1]);
        out[k] = {0.5f * (f1k.r + tw.r), 0.5f * (f1k.i + tw.i)};
        out[h - k] = {0.5f * (f1k.r - tw.r), 0.5f * (tw.i - f1k.i)};
    }
}

}  // namespace dsp
