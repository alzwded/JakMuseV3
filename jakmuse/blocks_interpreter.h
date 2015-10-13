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
    mapped_type at(key_type name) const;
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

#undef DECLARE_INSTANCE

#endif
