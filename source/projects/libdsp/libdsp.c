
#include <math.h>
#include <stdint.h>

// Scaling functions from: https://www.desmos.com/calculator/ewnq4hyrbz

double scale_linear(double x, double i_min, double i_max, double o_min, double o_max)
{
    double slope = 1.0 * (o_max - o_min) / (i_max - i_min);
    return o_min + round(slope * (x - i_min));
}


double scale_sine1(double x, double i_min, double i_max, double o_min, double o_max)
{
    return - (o_max - o_min) / 2.0 * cos(M_PI * (i_min - x) / (i_min - i_max)) + (o_max + o_min) / 2;
}

double scale_sine2(double x, double i_min, double i_max, double o_min, double o_max)
{
    return ((o_max - o_min) / M_PI) * asin((2 / (i_max - i_min)) * ( x - ((i_min + i_max) / 2) )) + ((o_max + o_min) / 2);
}


double scale_exp1(double x, double s, double i_min, double i_max, double o_min, double o_max)
{
    return -s * powf(fabs(o_min - o_max - s), (x - i_max) / (i_min  - i_max)) + o_max + s;
}

double scale_exp2(double x, double s, double i_min, double i_max, double o_min, double o_max)
{
    return s * powf(fabs(o_max - o_min + s), (x - i_min) / (i_max  - i_min)) + o_min - s;
}


double scale_log1(double x, double p, double i_min, double i_max, double o_min, double o_max)
{
    return ((o_max - o_min) * log(fabs(x - i_min + p))) / log(fabs(i_max - i_min + p)) + o_min;
}

double scale_log2(double x, double p, double i_min, double i_max, double o_min, double o_max)
{
    return ((o_min - o_max) * log(fabs(x - i_max - p))) / log(fabs(i_min - i_max - p)) + o_max;
}

//------------------------------------------------------------------------------
// Audio DSP Functions
//------------------------------------------------------------------------------

// Soft clipping / saturation using tanh approximation
// drive: 1.0 = no distortion, higher = more saturation
double soft_clip(double x, double drive)
{
    double shaped = x * drive;
    // Fast tanh approximation: x / (1 + |x|)
    return shaped / (1.0 + fabs(shaped));
}

// Hard clipping
// threshold: clipping threshold (0.0 - 1.0)
double hard_clip(double x, double threshold)
{
    if (x > threshold) return threshold;
    if (x < -threshold) return -threshold;
    return x;
}

// Bit crusher / sample rate reducer
// bits: bit depth (1-16)
// Returns quantized sample
double bit_crush(double x, double bits)
{
    if (bits <= 0) return x;
    if (bits >= 16) return x;

    double levels = pow(2.0, bits);
    double step = 2.0 / levels;
    return floor(x / step) * step;
}

// Simple one-pole low-pass filter (stateless, requires external state)
// x: input sample
// prev: previous output (feedback)
// cutoff: filter coefficient (0.0 = full filter, 1.0 = no filter)
double lpf_1pole(double x, double prev, double cutoff)
{
    return prev + cutoff * (x - prev);
}

// Simple one-pole high-pass filter (stateless, requires external state)
// x: input sample
// prev_in: previous input
// prev_out: previous output
// cutoff: filter coefficient
double hpf_1pole(double x, double prev_in, double prev_out, double cutoff)
{
    return cutoff * (prev_out + x - prev_in);
}

// Linear interpolation
double lerp(double a, double b, double t)
{
    return a + t * (b - a);
}

// Exponential envelope follower
// x: input sample (typically absolute value)
// prev: previous envelope value
// attack: attack coefficient (0.0 - 1.0, higher = faster)
// release: release coefficient (0.0 - 1.0, higher = faster)
double envelope_follow(double x, double prev, double attack, double release)
{
    double coeff = (x > prev) ? attack : release;
    return prev + coeff * (x - prev);
}

// Simple wavefolder
// x: input sample
// threshold: folding threshold
double wavefold(double x, double threshold)
{
    if (threshold <= 0) return x;

    double folded = x;
    while (fabs(folded) > threshold) {
        if (folded > threshold) {
            folded = 2.0 * threshold - folded;
        } else if (folded < -threshold) {
            folded = -2.0 * threshold - folded;
        }
    }
    return folded;
}

// Ring modulation
double ring_mod(double x, double modulator)
{
    return x * modulator;
}

// Clamp value to range
double clamp(double x, double min, double max)
{
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

//------------------------------------------------------------------------------
// Oscillator Functions (stateless - require external phase management)
//------------------------------------------------------------------------------

// Sine wave oscillator
// phase: 0.0 - 1.0 (normalized phase)
// Returns: -1.0 to 1.0
double osc_sine(double phase)
{
    return sin(phase * 2.0 * M_PI);
}

// Saw wave oscillator (naive)
// phase: 0.0 - 1.0 (normalized phase)
// Returns: -1.0 to 1.0
double osc_saw(double phase)
{
    return 2.0 * phase - 1.0;
}

// Saw wave oscillator (band-limited using polyBLEP)
// phase: 0.0 - 1.0 (normalized phase)
// phase_inc: phase increment per sample (freq/samplerate)
// Returns: -1.0 to 1.0
double osc_saw_bl(double phase, double phase_inc)
{
    double value = 2.0 * phase - 1.0;

    // PolyBLEP residual
    double t = phase;
    if (t < phase_inc) {
        t /= phase_inc;
        value += 2.0 * (t + t * (1.0 - t));
    } else if (t > 1.0 - phase_inc) {
        t = (t - 1.0) / phase_inc;
        value += 2.0 * (t + t * (1.0 + t));
    }

    return value;
}

// Square wave oscillator (naive)
// phase: 0.0 - 1.0 (normalized phase)
// pulse_width: 0.0 - 1.0 (duty cycle, 0.5 = square)
// Returns: -1.0 to 1.0
double osc_square(double phase, double pulse_width)
{
    return (phase < pulse_width) ? 1.0 : -1.0;
}

// Square wave oscillator (band-limited using polyBLEP)
// phase: 0.0 - 1.0 (normalized phase)
// pulse_width: 0.0 - 1.0 (duty cycle, 0.5 = square)
// phase_inc: phase increment per sample (freq/samplerate)
// Returns: -1.0 to 1.0
double osc_square_bl(double phase, double pulse_width, double phase_inc)
{
    double value = (phase < pulse_width) ? 1.0 : -1.0;

    // PolyBLEP at rising edge (phase = 0)
    double t = phase;
    if (t < phase_inc) {
        t /= phase_inc;
        value += 2.0 * (t - t * t - 1.0);
    } else if (t > 1.0 - phase_inc) {
        t = (t - 1.0) / phase_inc;
        value += 2.0 * (t * t + t + 1.0);
    }

    // PolyBLEP at falling edge (phase = pulse_width)
    t = phase - pulse_width;
    if (t < 0.0) t += 1.0;
    if (t < phase_inc) {
        t /= phase_inc;
        value -= 2.0 * (t - t * t - 1.0);
    } else if (t > 1.0 - phase_inc) {
        t = (t - 1.0) / phase_inc;
        value -= 2.0 * (t * t + t + 1.0);
    }

    return value;
}

// Triangle wave oscillator
// phase: 0.0 - 1.0 (normalized phase)
// Returns: -1.0 to 1.0
double osc_triangle(double phase)
{
    if (phase < 0.5) {
        return 4.0 * phase - 1.0;
    } else {
        return -4.0 * phase + 3.0;
    }
}

// Phase increment calculation helper
// freq: frequency in Hz
// sample_rate: sample rate in Hz
// Returns: phase increment per sample (0.0 - 1.0)
double osc_phase_inc(double freq, double sample_rate)
{
    return freq / sample_rate;
}

// Phase wraparound helper
// phase: current phase value
// Returns: wrapped phase (0.0 - 1.0)
double osc_phase_wrap(double phase)
{
    while (phase >= 1.0) phase -= 1.0;
    while (phase < 0.0) phase += 1.0;
    return phase;
}
