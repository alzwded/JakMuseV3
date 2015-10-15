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

struct WaveTable
{
    std::vector<double> table_;
    enum
    {
        TRUNCATE,
        LINEAR,
        COSINE
    } interpolationMethod_;

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

private:
    std::deque<value_type> values_;
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

    std::list<std::shared_ptr<ABlock>>& Inputs() { return inputs_; }
    double Value() { return ovalue_; }

    void Tick1() // output[-1] buffer -> input buffer
    {
        ivalue_ = Input();
    }

    virtual void Tick2() {} // for inputs generating ResetTick

    virtual void ResetTick(ResetKind) =0; // reset bus -> internal

    void Tick3() // internal -> output buffer
    {
        ovalue_ = NextValue_(ivalue_);
    }

protected:
    virtual double NextValue_(double) =0; // input buffer -> internal

protected:
    double Input()
    {
        double v = 0.0;
        v = std::accumulate(inputs_.begin(), inputs_.end(), v,
                [](double v, std::shared_ptr<ABlock> const& b) -> double {
                    return v + b->Value();
                });
        return v;
    }

protected:
    std::list<std::shared_ptr<ABlock>> inputs_;
    double ivalue_ = 0.0, ovalue_ = 0.0;
};

struct Constant
: public ABlock
{
    double value_ = 0.0;

    void ResetTick(ResetKind) override {}

protected:
    double NextValue_(double) override { return value_; }
};

struct Filter
: public ABlock
{
    std::shared_ptr<ABlock> K;
    std::shared_ptr<ABlock> A;
    std::shared_ptr<ABlock> D;
    std::shared_ptr<ABlock> S;
    std::shared_ptr<ABlock> R;
    bool ResetADSR;
    bool InvertADSR;
    std::shared_ptr<ABlock> Hi;
    std::shared_ptr<ABlock> Lo;
    enum { Cut, Flatten } mixing_;

    void ResetTick(ResetKind) override;

protected:
    double NextValue_(double) override;

private:
    double ApplyLowPassFilter(double in);
    double ApplyHighPassFilter(double in);
    double ApplyEnvelope(double in);
    int AttackValue();
    int DecayValue();
    int ReleaseValue();
    double SustainValue();

private:
    enum {
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE,
        REST
    } state;
    int ADSR_counter = 0;
    double loY = 0.0;
    double hiX = 0.0;
    double hiY = 0.0;
};

struct Generator
: public ABlock
{
    WaveTable WT;
    std::shared_ptr<ABlock> TGlide;

    void ResetTick(ResetKind) override;

protected:
    double NextValue_(double) override;

private:
    double F = 0.0;
    int NGlide = 0;
    PhaseAccumulator PA;
};

struct Input
: public ABlock
{
    typedef std::tuple<int, double, ResetKind> value_type;

    // TODO interpolation? That is in the staff right now...
    //      maybe it's actually better if interpolated PCM data
    //      gets interpolated before being put in the stream
    //      (gets rid of lookahead or potential delays)
    //      TO BE CONSIDERED
    TypeStream<value_type> stream_;
    std::vector<std::shared_ptr<ABlock>> resetBus_;

    void Tick2() override;

    void ResetTick(ResetKind) override;

protected:
    double NextValue_(double) override;

private:
    int step = 0;
    int objective = 0;
    double value = 0.0;
};

struct Delay
: public ABlock
{
    void ResetTick(ResetKind) override {}

protected:
    double NextValue_(double x) override { return x; }
};

struct Noise
: public ABlock
{
    void ResetTick(ResetKind) override;
    enum { EIGHT, SIXTEEN } type_;

protected:
    double NextValue_(double x) override;

private:
    int counter, goal;
    std::vector<unsigned> regs = std::vector<unsigned>{ 0xA001, 0xA001 }; // vs2013 RTM generates C2797 for some reason, so explicitly call list initializer; updates don't; yeah, it's one of those things
    static const unsigned polys[2];
};

struct Output
: public ABlock
{
    void ResetTick(ResetKind) override {}
    enum { Cut, Flatten } mixing_;

protected:
    double NextValue_(double) override;
};

typedef TypeStream<Input::value_type> CannonicalStream;


#endif
