#include "eq.h"

#include <mathutils.h>

#include <cmath>

namespace nap
{

	namespace audio
	{

		void Eq::prepare(Mode mode, ControllerValue frequency, float sampleRate, ControllerValue gain, ControllerValue q)
		{
			mMode = mode;
			mFrequency = frequency;
			mGain = gain;
			mQ = q;
			mSampleRate = sampleRate;
			float b0, b1, b2, a1, a2, c0, d0;
			calcCoeffs(b0, b1, b2, a1, a2, c0, d0);
			mBiquad.resetCoefficients(b0, b1, b2, a1, a2, c0, d0);
		}


		void Eq::set(Mode mode, ControllerValue frequency, float sampleRate, ControllerValue gain, ControllerValue q)
		{
			mMode = mode;
			mFrequency = frequency;
			mGain = gain;
			mQ = q;
			mSampleRate = sampleRate;
			float b0, b1, b2, a1, a2, c0, d0;
			calcCoeffs(b0, b1, b2, a1, a2, c0, d0);
			mBiquad.setCoefficients(b0, b1, b2, a1, a2, c0, d0);
		}


		void Eq::setSampleRate(float sampleRate)
		{
			mSampleRate = sampleRate;
			float b0, b1, b2, a1, a2, c0, d0;
			calcCoeffs(b0, b1, b2, a1, a2, c0, d0);
			mBiquad.setCoefficients(b0, b1, b2, a1, a2, c0, d0);
		}


		/**
		 * Calculates the biquad coefficients for the current mode using the RBJ audio EQ cookbook.
		 *
		 * The coefficients follow the textbook convention, matching BiquadFilter:
		 *
		 *          b0 + b1 * z^-1 + b2 * z^-2
		 *   H(z) = --------------------------
		 *          1  + a1 * z^-1 + a2 * z^-2
		 *
		 * so b0 / b1 / b2 are the numerator taps and a1 / a2 the denominator taps. The denominator is
		 * normalized so that a0 == 1, which is why a0 is not part of the signature. c0 is the wet gain
		 * and d0 the dry gain, used to blend the filtered signal with the input.
		 */
		void Eq::calcCoeffs(float &b0, float &b1, float &b2, float &a1, float &a2, float &c0, float &d0)
		{
			// Guard against a zero or negative Q, which would divide by zero below.
			float q = mQ > 0.f ? mQ : 0.70710678f;

			// Keep the frequency strictly inside the audible band. At or above Nyquist omega reaches
			// pi and the filter's poles leave the unit circle, so the output runs away to infinity.
			float nyquist = 0.5f * mSampleRate;
			float frequency = mFrequency;
			if (frequency < 1.f)
				frequency = 1.f;
			if (frequency > nyquist * 0.999f)
				frequency = nyquist * 0.999f;

			// Angular frequency of the center / shelf frequency, and the intermediate terms shared
			// by all modes. The factor is 2*pi (not pi): with pi the peak lands at half the requested
			// frequency, which is the classic symptom of a half-angle omega here.
			float omega = math::PIX2 * frequency / mSampleRate;
			float sinOmega = std::sin(omega);
			float cosOmega = std::cos(omega);
			float alpha = sinOmega / (2.f * q);

			// Linear amplitude of the gain. The square root (10^(G/40) rather than 10^(G/20)) is what
			// makes the peak of a peaking filter land exactly at G dB instead of 2G dB.
			float A = math::power<float>(10.f, mGain / 40.f);

			// The raw (unnormalized) coefficients of the transfer function, filled in per mode.
			float b0Raw, b1Raw, b2Raw, a0Raw, a1Raw, a2Raw;

			switch (mMode)
			{
				case Mode::Peak:
				{
					b0Raw = 1.f + alpha * A;
					b1Raw = -2.f * cosOmega;
					b2Raw = 1.f - alpha * A;
					a0Raw = 1.f + alpha / A;
					a1Raw = -2.f * cosOmega;
					a2Raw = 1.f - alpha / A;
					break;
				}
				case Mode::LowShelf:
				{
					// sqrt(A) terms appear here because the shelf is defined by its gain at DC and Nyquist.
					float sqrtA = std::sqrt(A);
					float twoSqrtAAlpha = 2.f * sqrtA * alpha;

					b0Raw = A * ((A + 1.f) - (A - 1.f) * cosOmega + twoSqrtAAlpha);
					b1Raw = 2.f * A * ((A - 1.f) - (A + 1.f) * cosOmega);
					b2Raw = A * ((A + 1.f) - (A - 1.f) * cosOmega - twoSqrtAAlpha);
					a0Raw = (A + 1.f) + (A - 1.f) * cosOmega + twoSqrtAAlpha;
					a1Raw = -2.f * ((A - 1.f) + (A + 1.f) * cosOmega);
					a2Raw = (A + 1.f) + (A - 1.f) * cosOmega - twoSqrtAAlpha;
					break;
				}
				case Mode::HighShelf:
				{
					float sqrtA = std::sqrt(A);
					float twoSqrtAAlpha = 2.f * sqrtA * alpha;

					b0Raw = A * ((A + 1.f) + (A - 1.f) * cosOmega + twoSqrtAAlpha);
					b1Raw = -2.f * A * ((A - 1.f) + (A + 1.f) * cosOmega);
					b2Raw = A * ((A + 1.f) + (A - 1.f) * cosOmega - twoSqrtAAlpha);
					a0Raw = (A + 1.f) - (A - 1.f) * cosOmega + twoSqrtAAlpha;
					a1Raw = 2.f * ((A - 1.f) - (A + 1.f) * cosOmega);
					a2Raw = (A + 1.f) - (A - 1.f) * cosOmega - twoSqrtAAlpha;
					break;
				}
			}

			// Normalize everything by a0 so the denominator starts with 1. The coefficients keep their
			// meaning, so this is a straight division of the numerator and denominator taps.
			float invA0 = 1.f / a0Raw;
			b0 = b0Raw * invA0;
			b1 = b1Raw * invA0;
			b2 = b2Raw * invA0;
			a1 = a1Raw * invA0;
			a2 = a2Raw * invA0;

			// Fully wet, no dry blend.
			c0 = 1.f;
			d0 = 0.f;
		}

	}

}
