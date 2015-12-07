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

#ifndef BLOCKS_H
#define BLOCKS_H

#include <memory>

#include <algorithm>
#include <iterator>
#include <numeric>

#include <tuple>
#include <vector>
#include <deque>
#include <list>

#ifdef _MSC_VER
typedef __int64 int64_t;
#else
# include <cstdint>
#endif

#include "log.h"

struct WaveTable
{
    std::vector<double> table_;
    enum
    {
        TRUNCATE,
        LINEAR,
        COSINE
    } interpolationMethod_ = TRUNCATE;

    double Value(double idx);

private:
    double Interpolate(double p, int i1, int i2);
};

struct PhaseAccumulator
{
    void Tick(double);
    double Value();
private:
    double phi = 0.0;
};

template<typename T>
struct TypeStream
{
    typedef T value_type;
    typedef T& reference;
    typedef T* pointer;
    typedef T const& const_reference;
    typedef T const* const_pointer;

    TypeStream& operator>>(reference d)
    {
        if(!values_.empty()) {
            d = values_.front();
            values_.pop_front();
            return *this; 
        } else {
            failed_ = true;
            return *this;
        }
    }
    TypeStream& operator<<(value_type d)
    {
        values_.push_back(d);
        return *this;
    }

    void push_back(value_type d)
    {
        values_.push_back(d);
    }

    template<typename IT>
    void append(IT begin, IT end)
    {
        values_.insert(value_.end(), begin, end);
        return *this;
    }

    inline bool bad() { return values_.empty(); }
    inline bool good() { return !bad() && !fail(); }
    inline bool fail() { return failed_; }
    inline bool eof() { return values_.empty(); }
    inline bool operator!() { return fail(); }
    inline operator void*() { return fail() ? nullptr : this; }
    inline operator bool() { return !fail(); }
    inline void clear() { failed_ = false; }

public: // eff u ur public nao
    std::deque<value_type> values_;

private:
    bool failed_ = false;
};

enum class ResetKind {
    NOTE,
    REST
};

// on cycle:
//    for each block call Tick1() (output[-1] buffer -> input buffer)
//    for each block call Reset() (update internal state)
//    for each block call Tick2() (internal state -> output buffer)
//    put Output->Value()
struct ABlock
{
    virtual ~ABlock() {}
    typedef std::tuple<bool, ResetKind> reset_t;

    std::list<std::shared_ptr<ABlock>>& Inputs() { return inputs_; }
    double Value() { return ovalue_; }
    std::shared_ptr<ABlock> ResetBy;
    virtual reset_t Reset() { return reset_; }

    void Tick1() // output[-1] buffer -> input buffer
    {
        ivalue_ = Input();
    }

    // for inputs generating ResetTick
    virtual void Tick2() {
        if(ResetBy) reset_ = ResetBy->reset_;
    }

    void Tick3() // internal -> output buffer
    {
        if(std::get<0>(reset_)) ResetTick(std::get<1>(reset_));
        ovalue_ = NextValue_(ivalue_);
    }

protected:
    virtual double NextValue_(double) =0; // input buffer -> internal
    virtual void ResetTick(ResetKind) =0; // reset bus -> internal


protected:
    double Input()
    {
        double v = 0.0;
        v = std::accumulate(inputs_.begin(), inputs_.end(), v,
                [](double v, std::shared_ptr<ABlock> const& b) -> double {
                    LOGF(LOG_BLOCKS, "add %f", b->Value());
                    return v + b->Value();
                });
        LOGF(LOG_BLOCKS, "Grand total %f", v);
        return v;
    }

protected:
    std::list<std::shared_ptr<ABlock>> inputs_;
    double ivalue_ = 0.0;
    double ovalue_ = 0.0;
public:
    reset_t reset_ = reset_t(false, ResetKind::REST);
};

struct Constant
: public ABlock
{
    double value_;

    void ResetTick(ResetKind) override {}

    Constant() : value_(0.0) { ivalue_ = ovalue_ = value_; }
    Constant(double value) : value_(value) { ivalue_ = ovalue_ = value_; }

protected:
    double NextValue_(double) override { return value_; }
};

struct Filter
: public ABlock
{
    std::shared_ptr<ABlock> K = std::shared_ptr<ABlock>(new Constant(1.0));
    std::shared_ptr<ABlock> A = std::shared_ptr<ABlock>(new Constant);
    std::shared_ptr<ABlock> D = std::shared_ptr<ABlock>(new Constant);
    std::shared_ptr<ABlock> S = std::shared_ptr<ABlock>(new Constant(1.0));
    std::shared_ptr<ABlock> R = std::shared_ptr<ABlock>(new Constant);
    bool ResetADSR = true;
    bool InvertADSR = false;
    std::shared_ptr<ABlock> Hi = std::shared_ptr<ABlock>(new Constant);
    std::shared_ptr<ABlock> Lo = std::shared_ptr<ABlock>(new Constant(1.0));
    enum { Cut, Flatten } mixing_ = Cut;

    void ResetTick(ResetKind) override;

protected:
    double NextValue_(double) override;

private:
    double ApplyLowPassFilter(double in);
    double ApplyHighPassFilter(double in);
    double ApplyEnvelope(double in);
    int64_t AttackValue();
    int64_t DecayValue();
    int64_t ReleaseValue();
    double SustainValue();

private:
    enum {
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE,
        REST
    } state = ATTACK;
    int64_t ADSR_counter = 0;
    double loY = 0.0;
    double hiX = 0.0;
    double hiY = 0.0;
};

struct Generator
: public ABlock
{
    WaveTable WT;
    std::shared_ptr<ABlock> TGlide = std::shared_ptr<ABlock>(new Constant);
    bool GlideOnRest = false;

    void ResetTick(ResetKind) override;

protected:
    double NextValue_(double) override;

private:
    double F = 0.0;
    int64_t NGlide = 0;
    PhaseAccumulator PA;
    bool shutUp = true;
};

struct Input
: public ABlock
{
    typedef std::tuple<int64_t, double, ResetKind> value_type;

    // TODO interpolation? That is in the staff right now...
    //      maybe it's actually better if interpolated PCM data
    //      gets interpolated before being put in the stream
    //      (gets rid of lookahead or potential delays)
    //      TO BE CONSIDERED
    TypeStream<value_type> stream_;
    enum { RetainValue, Zero } OnRest = Zero;

    void Tick2() override;

    void ResetTick(ResetKind) override;

protected:
    double NextValue_(double) override;

private:
    int64_t step = 0;
    int64_t objective = 0;
    double value = 0.0;
};

struct Delay
: public ABlock
{
#define Delay_SIZE (1024ul)
    typedef std::tuple<double, bool, ResetKind> head_t;

    Delay()
        : head_(&buffer_[0])
    {
        for(size_t i = 0; i < Delay_SIZE; ++i) {
            buffer_[i] = std::make_tuple(0.0, false, ResetKind::REST);
        }
    }

    void ResetTick(ResetKind) override;
    std::shared_ptr<ABlock> delay_ = std::shared_ptr<ABlock>(new Constant);

    void Tick2() override;

protected:
    double NextValue_(double x) override;

private:
    head_t buffer_[Delay_SIZE];
    head_t* head_;

private:
    head_t& Read()
    {
        auto v = fabs(delay_->Value());
        size_t delay = (size_t)(v * 999.0);
        auto lastIdx = (((head_ - &buffer_[0]) + Delay_SIZE) - delay) % Delay_SIZE;
        auto&& last = buffer_[lastIdx];
        return last;
    }
};

struct Noise
: public ABlock
{
    void ResetTick(ResetKind) override;
    enum { EIGHT, SIXTEEN } type_ = SIXTEEN;

protected:
    double NextValue_(double x) override;

private:
    int64_t counter = 0, goal = 0;
    std::vector<unsigned> regs = std::vector<unsigned>{ 0xA001, 0xA001 }; // vs2013 RTM generates C2797 for some reason, so explicitly call list initializer; updates don't; yeah, it's one of those things
    static const unsigned polys[2];
};

struct Output
: public ABlock
{
    void ResetTick(ResetKind) override {}
    enum { Cut, Flatten } mixing_ = Cut;

protected:
    double NextValue_(double) override;
};

typedef TypeStream<Input::value_type> CannonicalStream;


#endif
