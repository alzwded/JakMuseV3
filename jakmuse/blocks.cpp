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
        return 0;
    default:
#define INVALID_STATE false
        assert(INVALID_STATE);
        return 0;
#undef INVALID_STATE
    }
}

double Filter::NextValue_(double in)
{
    double r0 = ApplyLowPassFilter(in);
    double r1 = ApplyHighPassFilter(r0);
    double r2 = ApplyEnvelope(r1);
    assert(K);
    double r3 = K->Value() * r2;
    switch(mixing_) {
    case Cut:
        if(r3 > 1.0) return 1.0;
        if(r3 < 0.0) return 0.0;
        return r3;
    case Flatten:
        return (atan(r3 * 2.0 - 1.0) + 1.0) / 2.0;
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
        NGlide = TGlide->Value();
        break;
    case ResetKind::REST:
        NGlide = 0;
        break;
    }
}

double Generator::NextValue_(double in)
{
    double newF = 22050.0 * in;
    if(NGlide) {
        F = F + (newF - F) / NGlide;
        --NGlide;
    } else {
        F = newF;
    }
    PA.Tick(F);
    return WT.Value(PA.Value() / 44100.0);
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
    return atan(2.0 * i - 1.0);
}

// ===========================================================
// WaveTable
// ===========================================================

double WaveTable::Value(double idx)
{
    if(idx < 0.0) idx = 0.0;
    if(idx > 1.0) idx = 1.0;

    double normalized = idx * table_.size();

    if(normalized - trunc(normalized) < 1.0e-15) {
        int i = (int)normalized;
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
    phi = fmod(phi + F, 44100.0);
}

double PhaseAccumulator::Value()
{
    return phi;
}
