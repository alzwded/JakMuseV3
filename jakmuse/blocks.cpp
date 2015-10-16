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
            state = ATTACK;
            ADSR_counter = 0;
        } else {
            switch(state) {
            case REST:
                state = ATTACK;
                ADSR_counter = 0;
                break;
            case ATTACK:
                break;
            case DECAY:
                {
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
                    state = ATTACK;
                    double r0 = 1.0 - ((double)ADSR_counter / DecayValue());
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
                    state = RELEASE;
                    ADSR_counter = (int)(r0 * ReleaseValue());
                } else {
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
                    state = RELEASE;
                    ADSR_counter = 0;
                } else {
                    state = RELEASE;
                    ADSR_counter = (int)((1.0 - r0) * ReleaseValue());
                }
            }
            break;
        case SUSTAIN:
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
    double a = Lo->Value();
    double y = loY + a  * (in - loY);
    loY = y;
    return y;
}

double Filter::ApplyHighPassFilter(double in)
{
    assert(Hi);
    double a = Hi->Value();
    double y = a * (hiY + in - hiX);
    hiY = y;
    hiX = in;
    return y;
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
            return ret;
        } else {
            ADSR_counter = 0;
            state = DECAY;
            /*FALLTHROUGH*/
        }
    case DECAY:
        if(ADSR_counter < DecayValue()) {
            if(!ResetADSR) {
                double r0 = (1.0 - (double)ADSR_counter / DecayValue());
                double r2 = SustainValue();
                double r1 = 1 - r2;
                r0 = r0 * r1 + r2;
                return (r0 * x);
            } else {
                double r0 = ((double)ADSR_counter / DecayValue());
                double r2 = SustainValue();
                r0 = r0 * r2;
                return (r0 * x);
            }
            ADSR_counter++;
        } else {
            ADSR_counter = 0;
            state = SUSTAIN;
            /*FALLTHROUGH*/
        }
    case SUSTAIN:
        return SustainValue() * x;
    case RELEASE:
        if(ADSR_counter < ReleaseValue()) {
            if(!ResetADSR) {
                double r0 = (1.0 - (double)ADSR_counter / ReleaseValue());
                double r2 = SustainValue();
                r0 = r0 * r2;
                return r0 * x;
            } else {
                double r0 = ((double)ADSR_counter / ReleaseValue());
                double r2 = SustainValue();
                double r1 = 1.0 - r2;
                r0 = r0 * r1 + r2;
                return r0 * x;
            }
        } else {
            ADSR_counter = 0;
            state = REST;
            /*FALLTHROUGH*/
        }
    case REST:
        return 0.5;
    default:
#define INVALID_STATE false
        assert(INVALID_STATE);
        return 0.5;
#undef INVALID_STATE
    }
}

double Filter::NextValue_(double in)
{
    double normalized = 2.0 * in - 1.0;
    double r0 = ApplyLowPassFilter(normalized);
    double r1 = ApplyHighPassFilter(r0);
    double r2 = ApplyEnvelope(r1);
    assert(K);
    double r3 = K->Value() * r2;
    switch(mixing_) {
    case Cut:
        if(r3 > 1.0) return 1.0;
        if(r3 < -1.0) return 0.0;
        return (r3 + 1.0) / 2.0;
    case Flatten:
        return (tanh(r3) + 1.0) / 2.0;
    default:
#define UNKNOWN_MIXING_TYPE false
        assert(UNKNOWN_MIXING_TYPE);
        return 0.5;
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
        NGlide = TGlide->Value();
        break;
    case ResetKind::REST:
        NGlide = 0;
        break;
    }
}

double Generator::NextValue_(double in)
{
    printf("!!! %f\n", in);
    double newF = 22050.0 * in;
    if(NGlide) {
        F = F + (newF - F) / NGlide;
        --NGlide;
    } else {
        F = newF;
    }
    PA.Tick(F);
    printf(">>> %f %f\n", PA.Value(), (WT.Value(PA.Value() / 44100.0) + 1.0) / 2.0);
    return (WT.Value(PA.Value() / 44100.0) + 1.0) / 2.0;
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
            value = std::get<1>(newValue);
            kind = std::get<2>(newValue);
        }
        for(auto&& b : resetBus_) {
            b->ResetTick(kind);
        }
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
    printf("Ouptut: %f\n", 2.0 * i - 1.0);
    switch(mixing_)
    {
    case Output::Cut: return 2.0 * i - 1.0;
    case Output::Flatten: return tanh(2.0 * i - 1.0);
    default:
#define INVALID_MIXING_TYPE false
        assert(INVALID_MIXING_TYPE);
        return 0.5;
#undef INVALID_MIXING_TYPE
    }
}

// ===========================================================
// WaveTable
// ===========================================================

double WaveTable::Value(double idx)
{
    printf("WT: %f", idx);
    if(idx < 0.0) idx = 0.0;
    if(idx > 1.0 - 1.0e-15) idx = 1.0;
    printf(" %f", idx);

    double normalized = idx * (table_.size() - 1); // don't loop around; enables sawtooth
    printf(" %d %f\n", table_.size(), normalized);

    if(normalized - trunc(normalized) < 1.0e-15) {
        int i = (int)normalized;
        return table_[i];
    } else if(ceil(normalized) - normalized < 1.0e-15) {
        int i = (int)ceil(normalized);
        return table_[i];
    } else {
        int i1 = (int)floor(normalized);
        int i2 = (int)ceil(normalized);
        return Interpolate(idx - i1, i1, i2);
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
    printf("))) %f %f", phi, F);
    phi = fmod(phi + F, 44100.0);
    printf(" %f\n", phi);
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
    if(goal < 0) return 0.5;

    switch(type_) {
    case EIGHT: return (double)regs[0] / 0xFFFF;
    case SIXTEEN: return (double)regs[1] / 0xFFFF;
    default:
#define INVALID_NOISE_TYPE false
        assert(INVALID_NOISE_TYPE);
        return 0.5;
#undef INVALID_NOISE_TYPE
    }
}

// ===========================================================
// Delay
// ===========================================================

void Delay::ResetTick(ResetKind kind)
{
    buffer_.clear();
}

double Delay::NextValue_(double x)
{
    buffer_.push_front(x);
    if(buffer_.size() >= delay_) {
        auto ret = buffer_.back();
        buffer_.pop_back();
        return ret;
    } else {
        return 0.5;
    }
}
