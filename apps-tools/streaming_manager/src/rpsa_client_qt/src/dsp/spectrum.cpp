#include "spectrum.h"
#include <algorithm>
#include <cmath>

namespace dsp {

namespace {
constexpr double PI = 3.14159265358979323846;
}

auto Spectrum::setup(uint32_t n, Window window) -> bool {
    if (n < 256 || n > 65536 || (n & (n - 1)) != 0)
        return false;
    if (n == m_n && window == m_window)
        return true;
    if (!m_fft.init(n))
        return false;
    m_n = n;
    m_window = window;
    m_coefs.resize(n);
    m_in.resize(n);
    m_out.resize(n / 2 + 1);
    // the coefficients of the API (rp_dsp.cpp window_init); the scale of a window cancels in m_gain
    const double last = static_cast<double>(n - 1);
    double sum = 0;
    for (uint32_t i = 0; i < n; i++) {
        const double x = 2 * PI * i / last;
        double w = 1;
        if (window == HANN)
            w = 0.8165 * (1 - std::cos(x));  // the power scaling of the API: the same float rounding
        else if (window == FLAT_TOP)
            w = 0.21557895 - 0.41663158 * std::cos(x) + 0.277263158 * std::cos(2 * x) - 0.083578947 * std::cos(3 * x) + 0.006947368 * std::cos(4 * x);
        m_coefs[i] = static_cast<float>(w);
        sum += w;
    }
    m_gain = static_cast<float>(2.0 / sum);
    // the bins the API overwrites with the next one (remoteDCCount)
    m_dcBins = window == FLAT_TOP ? 5 : 2;
    return true;
}

auto Spectrum::power(const float* in, float* out) -> void {
    for (uint32_t i = 0; i < m_n; i++)
        m_in[i] = in[i] * m_coefs[i];
    m_fft.run(m_in.data(), m_out.data());
    const uint32_t b = bins();
    // |X| * gain: the amplitude; / sqrt(2): RMS; squared: the power
    const float k = m_gain * m_gain * 0.5f;
    for (uint32_t i = 0; i < b; i++)
        out[i] = (m_out[i].r * m_out[i].r + m_out[i].i * m_out[i].i) * k;
    const uint32_t dc = std::min(m_dcBins, b - 1);
    for (uint32_t i = 0; i < dc; i++)
        out[i] = out[dc];
}

}  // namespace dsp
