#include "blocks_interpreter.h"

template struct InstanceInterpreter<Constant>;
template struct InstanceInterpreter<Generator>;
template struct InstanceInterpreter<Filter>;
template struct InstanceInterpreter<Input>;
template struct InstanceInterpreter<Delay>;

std::shared_ptr<IInstanceInterpreter> GetInterpreter(std::string instanceType, std::string name)
{
    if(instanceType.compare("Constant") == 0) {
        return std::dynamic_pointer_cast<IInstanceInterpreter>(std::make_shared<InstanceInterpreter<Constant>>(name));
    } else if(instanceType.compare("Generator") == 0) {
        return std::dynamic_pointer_cast<IInstanceInterpreter>(std::make_shared<InstanceInterpreter<Generator>>(name));
    } else if(instanceType.compare("Filter") == 0) {
        return std::dynamic_pointer_cast<IInstanceInterpreter>(std::make_shared<InstanceInterpreter<Filter>>(name));
    } else if(instanceType.compare("Input") == 0) {
        return std::dynamic_pointer_cast<IInstanceInterpreter>(std::make_shared<InstanceInterpreter<Input>>(name));
    } else if(instanceType.compare("Delay") == 0) {
        return std::dynamic_pointer_cast<IInstanceInterpreter>(std::make_shared<InstanceInterpreter<Delay>>(name));
    } else {
        throw std::invalid_argument(std::string("Unknown instance type ") + instanceType);
    }
}

template<>
DelayedLookup_fn
InstanceInterpreter<Constant>::AcceptParameter(
        std::string paramName,
        PpValue value)
{
    if(paramName.compare("Value") != 0) throw std::invalid_argument("paramName: Expecing Value");
    switch(value.type) {
    case PpValue::PpNUMBER:
        thing_->value_ = value.num;
        return nullptr;
    default:
        throw std::invalid_argument("value: Expecting a NUMBER");
    }
}

template<>
DelayedLookup_fn
InstanceInterpreter<Generator>::AcceptParameter(
        std::string paramName,
        PpValue value)
{
    if(paramName.compare("WT") == 0) {
        return nullptr;
    } else if(paramName.compare("Glide") == 0) {
        return nullptr;
    } else if(paramName.compare("IN") == 0) {
        switch(value.type) {
        case PpValue::PpLIST:
            {
                thing_->Inputs().clear();
                std::deque<DelayedLookup_fn> fns;
                for(PpValueList* p = value.list; p; p = p->next) {
                    PpValue v = p->value;
                    switch(v.type) {
                    case PpValue::PpSTRING:
                        {
                            std::string name;
                            name.assign(value.str);
                            thing_sp thing = thing_;
                            fns.push_back([thing, name](LookupMap_t const& map) {
                                        thing->Inputs().push_back(map.at(name));
                                    });
                        }
                        break;
                    default:
                        throw std::invalid_argument("value: expected STRING or LIST of STRINGs or NUMBER");
                    }
                }
                return [fns](LookupMap_t const& map) {
                    for(auto&& fn : fns) {
                        fn(map);
                    }
                };
            }
            break;
        case PpValue::PpSTRING:
            {
                thing_->Inputs().clear();
                std::string name;
                name.assign(value.str);
                std::shared_ptr<thing_t> thing = thing_;
                return [thing, name](LookupMap_t const& map) {
                    thing->Inputs().push_back(map.at(name));
                };
            }
        case PpValue::PpNUMBER:
            {
                thing_->Inputs().clear();
                std::shared_ptr<Constant> k(new Constant);
                k->value_ = (double)value.num / 22050; // FIXME
                thing_->Inputs().push_back(std::dynamic_pointer_cast<ABlock>(k));
            };
        default:
            throw std::invalid_argument("IN: value: expecting LIST of STRINGs or STRING");
        }
    } else if(paramName.compare("RST") == 0) {
        return nullptr;
    } else {
        throw std::invalid_argument("paramName: Expecting WT, Glide, IN, RST");
    }
}

template<>
DelayedLookup_fn
InstanceInterpreter<Filter>::AcceptParameter(std::string paramName, PpValue value)
{
    return nullptr;
}

template<>
DelayedLookup_fn
InstanceInterpreter<Input>::AcceptParameter(std::string paramName, PpValue value)
{
    return nullptr;
}

template<>
DelayedLookup_fn
InstanceInterpreter<Delay>::AcceptParameter(std::string paramName, PpValue value)
{
    return nullptr;
}
