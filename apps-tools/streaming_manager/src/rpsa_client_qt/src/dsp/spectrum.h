#ifndef RPSA_DSP_SPECTRUM_H
#define RPSA_DSP_SPECTRUM_H

#include <cstdint>
#include <vector>
#include "kiss_fft.h"

namespace dsp {

// The amplitude spectrum of a block, as the spectrum analyzer of the Red Pitaya API makes it
// (rp-api/api-dsp CDSP: window, FFT, amplitude 2 / sum of the window, RMS, the DC bins), kept to
// the windows of the preview (streaming_improvements 4.5)
class Spectrum {
   public:
    // the values of rp_dsp_api::window_mode_t
    enum Window { RECTANGULAR = 0, HANN = 1, FLAT_TOP = 4 };

    // n: a power of two, 256 to 65536; false otherwise
    auto setup(uint32_t n, Window window) -> bool;
    auto size() const -> uint32_t { return m_n; }
    auto bins() const -> uint32_t { return m_n / 2; }

    // n samples (of the full range, -1..1) -> n / 2 values of the power, V^2 RMS of a full range
    // of 1 V: 10 log10 of it is dBV. The first bins (the DC and the skirt of the window) take the
    // value of the bin after them.
    auto power(const float* in, float* out) -> void;

   private:
    uint32_t m_n = 0;
    Window m_window = HANN;
    std::vector<float> m_coefs;  // the window
    float m_gain = 1;            // 2 / the sum of the window: the amplitude of a sine
    uint32_t m_dcBins = 2;
    std::vector<float> m_in;
    std::vector<Cpx> m_out;
    RealFft m_fft;
};

}  // namespace dsp

#endif
