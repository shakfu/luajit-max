-- time_helpers.lua
-- Utility functions for time-based DSP calculations
-- SAMPLE_RATE is automatically provided by the external

-- Convert milliseconds to samples
function ms_to_samples(milliseconds)
    return math.floor(milliseconds * SAMPLE_RATE / 1000.0 + 0.5)
end

-- Convert seconds to samples
function sec_to_samples(seconds)
    return math.floor(seconds * SAMPLE_RATE + 0.5)
end

-- Convert samples to milliseconds
function samples_to_ms(samples)
    return samples * 1000.0 / SAMPLE_RATE
end

-- Convert samples to seconds
function samples_to_sec(samples)
    return samples / SAMPLE_RATE
end

-- Convert frequency (Hz) to period in samples
function freq_to_period(frequency_hz)
    return math.floor(SAMPLE_RATE / frequency_hz + 0.5)
end

-- Convert period (samples) to frequency (Hz)
function period_to_freq(period_samples)
    return SAMPLE_RATE / period_samples
end

-- Calculate filter coefficient from cutoff frequency (Hz)
-- For simple one-pole lowpass: alpha = 1 - e^(-2*pi*fc/fs)
function cutoff_to_alpha(cutoff_hz)
    return 1.0 - math.exp(-2.0 * math.pi * cutoff_hz / SAMPLE_RATE)
end

-- Calculate cutoff frequency from time constant (seconds)
-- fc = 1 / (2*pi*tau)
function tau_to_cutoff(tau_seconds)
    return 1.0 / (2.0 * math.pi * tau_seconds)
end

-- Print timing info (useful for debugging)
function print_timing_info()
    api.post(string.format("Sample Rate: %.1f Hz", SAMPLE_RATE))
    api.post(string.format("Sample Period: %.6f seconds", 1.0 / SAMPLE_RATE))
    api.post(string.format("Nyquist Frequency: %.1f Hz", SAMPLE_RATE / 2.0))
    api.post(string.format("1 ms = %d samples", ms_to_samples(1)))
    api.post(string.format("10 ms = %d samples", ms_to_samples(10)))
    api.post(string.format("100 ms = %d samples", ms_to_samples(100)))
    api.post(string.format("1 second = %d samples", sec_to_samples(1)))
end

-- Example usage:
-- require 'time_helpers'
--
-- local delay_time_ms = 50  -- 50 milliseconds
-- local delay_samples = ms_to_samples(delay_time_ms)
-- _delay:setDelay(delay_samples)
