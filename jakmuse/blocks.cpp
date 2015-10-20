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

#define _USE_MATH_DEFINES
#include "blocks.h"
#include "log.h"
#include <cmath>
#include <assert.h>

// ===========================================================
// Filter
// ===========================================================

void Filter::ResetTick(ResetKind kind)
{
    // decision: to resetadsr or not to resetadsr
    //        ...based on previous input
    //        ...which is taken from the reset kind
    switch(kind)
    {
    case ResetKind::NOTE:
        if(ResetADSR) {
            LOGF(LOG_BLOCKS, "* -> ATTACK");
            state = ATTACK;
            ADSR_counter = 0;
        } else {
            switch(state) {
            case REST:
                LOGF(LOG_BLOCKS, "REST -> ATTACK");
                state = ATTACK;
                ADSR_counter = 0;
                break;
            case ATTACK:
                break;
            case DECAY:
                {
                    LOGF(LOG_BLOCKS, "DECAY -> ATTACK");
                    state = ATTACK;
                    double r0 = 1.0 - ((double)ADSR_counter / DecayValue());
                    double r2 = SustainValue();
                    double r1 = 1.0 - r2;
                    r0 = (r0 * r1) + r2;
                    ADSR_counter = (int)(r0 * AttackValue());
                }
                break;
            case SUSTAIN:
                ADSR_counter = 0;
                break;
            case RELEASE:
                {
                    LOGF(LOG_BLOCKS, "RELEASE -> ATTACK");
                    state = ATTACK;
                    double r0 = 1.0 - ((double)ADSR_counter / ReleaseValue());
                    double r2 = ReleaseValue();
                    r0 = r0 * r2;
                    ADSR_counter = (int)(r0 * AttackValue());
                }
                break;
            }
        }
        break;
    case ResetKind::REST:
        switch(state) {
        case ATTACK:
            {
                double r0 = (double)ADSR_counter / AttackValue();
                double r1 = S->Value();
                double r2 = 1 - r1;
                if(r0 < r1) {
                    LOGF(LOG_BLOCKS, "ATTACK -> RELEASE");
                    state = RELEASE;
                    ADSR_counter = (int)(r0 * ReleaseValue());
                } else {
                    LOGF(LOG_BLOCKS, "ATTACK -> reset RELEASE");
                    // TODO lean release
                    state = RELEASE;
                    ADSR_counter = 0;
                }
            }
            break;
        case DECAY:
            {
                double r0 = 1.0 - (double)ADSR_counter / DecayValue();
                double r2 = SustainValue();
                double r1 = 1.0 - r2;
                r0 = r0 * r1 + r2;
                if(r0 > r2) {
                    // TODO lean release
                    LOGF(LOG_BLOCKS, "DECAY -> reset RELEASE");
                    state = RELEASE;
                    ADSR_counter = 0;
                } else {
                    LOGF(LOG_BLOCKS, "DECAY -> RELEASE");
                    state = RELEASE;
                    ADSR_counter = (int)((1.0 - r0) * ReleaseValue());
                }
            }
            break;
        case SUSTAIN:
            LOGF(LOG_BLOCKS, "SUSTAIN -> RELEASE");
            state = RELEASE;
            ADSR_counter = 0;
            break;
        case RELEASE:
            break;
        case REST:
            break;
        }
        break;
    }
}

int Filter::AttackValue()
{
    assert(A);
    return (int)(44100.0 * 2.5 * A->Value());
}

int Filter::DecayValue()
{
    assert(D);
    return (int)(44100.0 * 2.5 * D->Value());
}

double Filter::SustainValue()
{
    assert(S);
    LOGF(LOG_BLOCKS, "%f", S->Value());
    return S->Value();
}

int Filter::ReleaseValue()
{
    assert(R);
    return (int)(44100.0 * 2.5 * R->Value());
}

double Filter::ApplyLowPassFilter(double in)
{
    assert(Lo);
    if(Lo->Value() > 1.0e-7) {
        double rc = 1.0 / (2.0 * M_PI * Lo->Value());
        double a = 1.0 / (rc + 1.0);
        double y = a * in + (1 - a) * loY;
        LOGF(LOG_BLOCKS, "in %f a %f y %f filtered %f", in, a, loY, y);
        loY = y;
        return y;
    } else {
        double y = loY;
        LOGF(LOG_BLOCKS, "low pass filter went overboard");
        return y;
    }
}

double Filter::ApplyHighPassFilter(double in)
{
    assert(Hi);
    if(Hi->Value() > 1.0e-7) {
        double rc = 1.0 / (2.0 * M_PI * Hi->Value());
        double a = rc / (rc + 1.0);
        double y = a * hiY + a * (in - hiX);
        LOGF(LOG_BLOCKS, "in %f a %f y %f x %f filtered %f", in, a, hiY, hiX, y);
        hiY = y;
        hiX = in;
        return y;
    } else {
#if 1
        hiY = in;
        hiX = in;
        return in;
#else
        double rc = 1.0 / (2.0 * M_PI * 1.0e-7);
        double a = rc / (rc + 1.0);
        LOGF(LOG_BLOCKS, "high filter went overboard");
        double y = a * hiY + a * (in - hiX);
        hiY = y;
        hiX = in;
        return y;
#endif
    }
}

double Filter::ApplyEnvelope(double x)
{
    switch(state)
    {
    case ATTACK:
        if(ADSR_counter < AttackValue()) {
            double value = (double)ADSR_counter / AttackValue();
            if(InvertADSR) value = 1.0 - value;
            double ret = (value * x);
            ADSR_counter++;
            LOGF(LOG_BLOCKS, "ATTACK a %f x %f ret %f", value, x, ret);
            return ret;
        } else {
            LOGF(LOG_BLOCKS, "ATTACK -> DECAY");
            ADSR_counter = 0;
            state = DECAY;
            /*FALLTHROUGH*/
        }
    case DECAY:
        if(ADSR_counter < DecayValue()) {
            if(!InvertADSR) {
                double r0 = (1.0 - (double)ADSR_counter / DecayValue());
                double r2 = SustainValue();
                double r1 = 1 - r2;
                r0 = r0 * r1 + r2;
                LOGF(LOG_BLOCKS, "DECAY a %f x %f", r0, x);
                ADSR_counter++;
                return (r0 * x);
            } else {
                double r0 = ((double)ADSR_counter / DecayValue());
                double r2 = SustainValue();
                r0 = r0 * r2;
                LOGF(LOG_BLOCKS, "DECAY a %f x %f", r0, x);
                ADSR_counter++;
                return (r0 * x);
            }
        } else {
            LOGF(LOG_BLOCKS, "DECAY -> SUSTAIN");
            ADSR_counter = 0;
            state = SUSTAIN;
            /*FALLTHROUGH*/
        }
    case SUSTAIN:
        LOGF(LOG_BLOCKS, "SUSTAIN a %f x %f", SustainValue(), x);
        return SustainValue() * x;
    case RELEASE:
        if(ADSR_counter < ReleaseValue()) {
            if(!InvertADSR) {
                double r0 = (1.0 - (double)ADSR_counter / ReleaseValue());
                double r2 = SustainValue();
                r0 = r0 * r2;
                LOGF(LOG_BLOCKS, "RELEASE a %f x %f", r0, x);
                ADSR_counter++;
                return r0 * x;
            } else {
                double r0 = ((double)ADSR_counter / ReleaseValue());
                double r2 = SustainValue();
                double r1 = 1.0 - r2;
                r0 = r0 * r1 + r2;
                LOGF(LOG_BLOCKS, "RELEASE a %f x %f", r0, x);
                ADSR_counter++;
                return r0 * x;
            }
        } else {
            LOGF(LOG_BLOCKS, "RELEASE -> REST");
            ADSR_counter = 0;
            state = REST;
            /*FALLTHROUGH*/
        }
    case REST:
        return 0.0;
    default:
#define INVALID_STATE false
        assert(INVALID_STATE);
        return 0.0;
#undef INVALID_STATE
    }
}

double Filter::NextValue_(double in)
{
    double normalized = in;
    double r0 = ApplyLowPassFilter(normalized);
    double r1 = ApplyHighPassFilter(r0);
    double r2 = ApplyEnvelope(r1);
    assert(K);
    double r3 = K->Value() * r2;
    LOGF(LOG_BLOCKS, "in %f norm %f lo %f hi %f enve %f gain %f", in, normalized, r0, r1, r2, r3);
    switch(mixing_) {
    case Cut:
        if(r3 > 1.0) return 1.0;
        if(r3 < -1.0) return -1.0;
        return r3;
    case Flatten:
        return tanh(r3);
    default:
#define UNKNOWN_MIXING_TYPE false
        assert(UNKNOWN_MIXING_TYPE);
        return 0.0;
#undef UNKNOWN_MIXING_TYPE
    }
}

// ===========================================================
// Generator
// ===========================================================

void Generator::ResetTick(ResetKind kind)
{
    switch(kind)
    {
    case ResetKind::NOTE:
        assert(TGlide);
        if(!GlideOnRest && !shutUp) {
            NGlide = (int)(TGlide->Value() * 2550.0);
            shutUp = false;
        } else if(!GlideOnRest && shutUp) {
            NGlide = 0;
            shutUp = false;
        } else if(GlideOnRest) {
            NGlide = (int)(TGlide->Value() * 2550.0);
            shutUp = false;
        }
        break;
    case ResetKind::REST:
        if(GlideOnRest) {
            NGlide = (int)(TGlide->Value() * 2550.0);
            shutUp = false;
        } else {
            NGlide = 0;
            shutUp = true;
        }
        break;
    }
}

double Generator::NextValue_(double in)
{
    LOGF(LOG_BLOCKS, "in = %f", in);
    double newF = 22050.0 * in;
    if(!shutUp && NGlide) {
        F = F + (newF - F) / NGlide;
        --NGlide;
    } else if(shutUp || !NGlide) {
        F = newF;
    }
    PA.Tick(F);
    LOGF(LOG_BLOCKS, "PA = %f, WT = %f, sample = %f", PA.Value(), WT.Value(PA.Value()), (WT.Value(PA.Value() / 44100.0)));
    return (WT.Value(PA.Value() / 44100.0));
}

// ===========================================================
// Input
// ===========================================================

void Input::ResetTick(ResetKind) {}

void Input::Tick2()
{
    // return next value
    if(step >= objective) {
        step = 0;
        value_type newValue;
        stream_ >> newValue;
        ResetKind kind = ResetKind::NOTE;
        if(!stream_) {
            objective = 0;
            value = 0.0;
            kind = ResetKind::REST;
        } else {
            objective = std::get<0>(newValue);
            if(OnRest == Input::RetainValue
                    && fabs(std::get<1>(newValue)) < 1.0e-7)
            {
                value = value;
            } else {
                value = std::get<1>(newValue);
            }
            kind = std::get<2>(newValue);
        }
        reset_ = std::make_tuple(true, kind);
    } else {
        ++step;
    }
}

double Input::NextValue_(double)
{
    return value;
}

// ===========================================================
// Output
// ===========================================================

double Output::NextValue_(double i)
{
    LOGF(LOG_BLOCKS, "in = %f", i);
    switch(mixing_)
    {
    case Output::Cut: return i;
    case Output::Flatten: return tanh(i);
    default:
#define INVALID_MIXING_TYPE false
        assert(INVALID_MIXING_TYPE);
        return 0.0;
#undef INVALID_MIXING_TYPE
    }
}

// ===========================================================
// WaveTable
// ===========================================================

double WaveTable::Value(double idx)
{
    if(idx < 0.0) idx = 0.0;
    if(idx > 1.0 - 1.0e-15) idx = 1.0;

    double normalized = idx * (table_.size() - 1); // don't loop around; enables sawtooth

    if(fabs(normalized - trunc(normalized)) < 1.0e-15) {
        int i = (int)normalized;
        return table_[i];
    } else if(fabs(ceil(normalized) - normalized) < 1.0e-15) {
        int i = (int)ceil(normalized);
        return table_[i];
    } else {
        int i1 = (int)floor(normalized);
        int i2 = (int)ceil(normalized);
        return Interpolate(normalized - i1, i1, i2);
    }
}

double WaveTable::Interpolate(double p, int i1, int i2)
{
    double v1 = table_[i1];
    double v2 = (i2 == table_.size()) ? table_[0] : table_[i2];

    switch(interpolationMethod_) {
    case TRUNCATE:
        return v1;
    case LINEAR:
        return v1 * (1.0 - p) + v2 * p;
    case COSINE:
        {
            double t = (1.0 - cos(M_PI * p)) / 2.0;
            return v1 * (1.0 - t) + v2 * t;
        }
    default:
#define INVALID_INTERPOLATION_METHOD false
        assert(INVALID_INTERPOLATION_METHOD);
        return v1;
#undef INVALID_INTERPOLATION_METHOD
    };
}

// ===========================================================
// PhaseAccumulator
// ===========================================================

void PhaseAccumulator::Tick(double in)
{
    double F = in;
    if(fabs(F) < 1.0e-15) return;
    LOGF(LOG_BLOCKS, "in = %f, phi = %f", F, phi);
    phi = fmod(phi + F, 44100.0);
    LOGF(LOG_BLOCKS, "phi = %f", phi);
}

double PhaseAccumulator::Value()
{
    return phi;
}

// ===========================================================
// Noise
// ===========================================================

const unsigned Noise::polys[2] = { 0x8255, 0xA801 };

void Noise::ResetTick(ResetKind kind)
{
    if(kind == ResetKind::REST) goal = -1;
    else goal = (int)(999.0 * ivalue_);

    ++counter;
    if(counter >= goal) {
        switch(type_) {
        case EIGHT:
            if(regs[0] & 0x1) regs[0] = (regs[0] >> 1) ^ polys[0];
            else regs[0] = (regs[0] >> 1);
            break;
        case SIXTEEN:
            if(regs[1] & 0x1) regs[1] = (regs[1] >> 1) ^ polys[1];
            else regs[1] = (regs[1] >> 1);
            break;
        }
        counter = 0;
    }
}

double Noise::NextValue_(double)
{
    if(goal < 0) return 0.0;

    switch(type_) {
    case EIGHT: return ((double)regs[0] / 0xFFFF - 0.5) * 2.0;
    case SIXTEEN: return ((double)regs[1] / 0xFFFF - 0.5) * 2.0;
    default:
#define INVALID_NOISE_TYPE false
        assert(INVALID_NOISE_TYPE);
        return 0.0;
#undef INVALID_NOISE_TYPE
    }
}

// ===========================================================
// Delay
// ===========================================================

void Delay::Tick2()
{
    if(ResetBy) {
        auto l = std::make_tuple(0.0, std::get<0>(ResetBy->reset_), std::get<1>(ResetBy->reset_));
        buffer_.push_front(l);
    } else {
        auto l = std::make_tuple(0.0, false, ResetKind::REST);
        buffer_.push_front(l);
    }
    auto last = buffer_.back();
    reset_ = std::make_tuple(std::get<1>(last), std::get<2>(last));
}

void Delay::ResetTick(ResetKind kind)
{
}

double Delay::NextValue_(double x)
{
    auto f = buffer_.front();
    buffer_.front() = std::make_tuple(x, std::get<1>(f), std::get<2>(f));
    if(buffer_.size() >= delay_) {
        auto ret = buffer_.back();
        buffer_.pop_back();
        return std::get<0>(ret);
    } else {
        return 0.0;
    }
}
