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

#ifndef BLOCKS_INTERPRETER_H
#define BLOCKS_INTERPRETER_H

#include "blocks.h"
#include "parser_types.h"
#include <functional>
#include <stdexcept>

struct IInstanceInterpreter;

struct LookupMap_t
{
    typedef std::shared_ptr<IInstanceInterpreter> value_type;
    typedef std::string key_type;
    typedef std::shared_ptr<ABlock> mapped_type;
    std::vector<std::shared_ptr<IInstanceInterpreter>> data_;
    typedef decltype(data_)::const_iterator iterator;
    typedef decltype(data_)::const_iterator const_iterator;
    mapped_type at(key_type name) const;
    iterator find(key_type name) const;
    iterator end() const { return data_.end(); }
    iterator begin() const { return data_.begin(); }
};
typedef std::function<void(LookupMap_t)> DelayedLookup_fn;

struct IInstanceInterpreter
{
    virtual ~IInstanceInterpreter() {}
    virtual std::shared_ptr<ABlock> Block() =0;
    virtual std::string Name() =0;
    virtual DelayedLookup_fn AcceptParameter(std::string paramName, PpValue value) =0;
    virtual CannonicalStream& InputBuffer() =0;
};

std::shared_ptr<IInstanceInterpreter> GetInterpreter(std::string instanceType, std::string name);

template<typename T>
struct InstanceInterpreter
: public IInstanceInterpreter
{
    typedef T thing_t;
    typedef std::shared_ptr<thing_t> thing_sp;
    thing_sp thing_;
    std::string name_;

    DelayedLookup_fn AcceptParameter(std::string paramName, PpValue value) override;
    InstanceInterpreter(std::string name)
        : name_(name)
          , thing_(new thing_t)
    {}

    std::string Name() override { return name_; }
    std::shared_ptr<ABlock> Block() override { return thing_; }
    CannonicalStream& InputBuffer() override;
};

#define DECLARE_INSTANCE(TYPE) \
    template<> DelayedLookup_fn InstanceInterpreter<TYPE>::AcceptParameter(std::string, PpValue); \
    extern template InstanceInterpreter<TYPE>;

template<> CannonicalStream& InstanceInterpreter<Input>::InputBuffer();

DECLARE_INSTANCE(Constant)
DECLARE_INSTANCE(Generator)
DECLARE_INSTANCE(Filter)
DECLARE_INSTANCE(Input)
DECLARE_INSTANCE(Delay)
DECLARE_INSTANCE(Noise)
DECLARE_INSTANCE(Output)

#undef DECLARE_INSTANCE

#endif
