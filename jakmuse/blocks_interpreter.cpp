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
        switch(value.type) {
        case PpValue::PpLIST:
            {
                for(PpValueList* p = value.list; p; p = p->next) {
                    PpValue v = p->value;
                    switch(v.type) {
                    case PpValue::PpNUMBER:
                        thing_->WT.table_.push_back(v.num);
                        return nullptr;
                    default:
                        throw std::invalid_argument("WT: value: Expecing a LIST of NUMBERs");
                    }
                }
            }
        default:
            throw std::invalid_argument("WT: value: Expecing a LIST of NUMBERs");
        }
    } else if(paramName.compare("Interpolation") == 0) {
        switch(value.type) {
        case PpValue::PpSTRING:
            {
                std::string s;
                s.assign(value.str);
                if(s.compare("Cosine") == 0) {
                    thing_->WT.interpolationMethod_ = WaveTable::COSINE;
                    return nullptr;
                } else if(s.compare("Linear") == 0) {
                    thing_->WT.interpolationMethod_ = WaveTable::LINEAR;
                    return nullptr;
                } else if(s.compare("Trunc") == 0) {
                    thing_->WT.interpolationMethod_ = WaveTable::TRUNCATE;
                    return nullptr;
                } else {
                    /*FALLTHROUGH*/
                }
            }
        default:
            throw std::invalid_argument("WT: interpolation: Expecting Cosine, Trunc, Linear");
        }
    } else if(paramName.compare("Glide") == 0) {
        switch(value.type) {
        case PpValue::PpSTRING:
            {
                std::string name;
                name.assign(value.str);
                thing_sp thing = thing_;
                return [thing, name](LookupMap_t const& map) {
                        thing->TGlide = map.at(name);
                        };
            }
            break;
        case PpValue::PpNUMBER:
            {
                std::shared_ptr<Constant> k(new Constant);
                k->value_ = (double)value.num / 2550; // FIXME
                thing_->TGlide = std::dynamic_pointer_cast<ABlock>(k);
                return nullptr;
            }
        default:
            throw std::invalid_argument("Generator: Glide: expecinting a NUMBER or a STRING");
        }
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
                    for(auto&& fn : fns) fn(map);
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
                return nullptr;
            }
        default:
            throw std::invalid_argument("IN: value: expecting LIST of STRINGs or STRING");
        }
    } else {
        throw std::invalid_argument("paramName: Expecting WT, Interpolation, Glide, IN");
    }
}

template<>
DelayedLookup_fn
InstanceInterpreter<Filter>::AcceptParameter(std::string paramName, PpValue value)
{
    if(paramName.compare("A") == 0) {
    } else if(paramName.compare("D") == 0) {
    } else if(paramName.compare("S") == 0) {
    } else if(paramName.compare("R") == 0) {
    } else if(paramName.compare("ResetADSR") == 0) {
    } else if(paramName.compare("InvertADSR") == 0) {
    } else if(paramName.compare("Mixing") == 0) {
    } else if(paramName.compare("Low") == 0) {
    } else if(paramName.compare("High") == 0) {
    } else if(paramName.compare("K") == 0) {
    } else if(paramName.compare("IN") == 0) {
        // TODO this bit of code is common for everyone
        //      refactor into template<enum> ?
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
                    for(auto&& fn : fns) fn(map);
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
                k->value_ = (double)value.num / 999.0; // FIXME
                thing_->Inputs().push_back(std::dynamic_pointer_cast<ABlock>(k));
                return nullptr;
            }
        default:
            throw std::invalid_argument("IN: value: expecting LIST of STRINGs or STRING");
        }

    }
    throw std::invalid_argument("Filter: paramName: expecting A,D,S,R,ResetADSR,Low,High,K,InvertADSR,Mixing,IN");
}

template<>
DelayedLookup_fn
InstanceInterpreter<Input>::AcceptParameter(std::string paramName, PpValue value)
{
    if(paramName.compare("RST") == 0) {
        switch(value.type)
        {
        case PpValue::PpSTRING:
            {
                thing_->resetBus_.clear();
                std::string name;
                name.assign(value.str);
                std::shared_ptr<thing_t> thing = thing_;
                return [thing, name](LookupMap_t const& map) {
                    thing->resetBus_.push_back(map.at(name));
                };
            }
        case PpValue::PpLIST:
            {
                std::vector<std::string> names;
                for(PpValueList* p = value.list; p; p = p->next)
                {
                    PpValue v = p->value;
                    switch(v.type) {
                    case PpValue::PpSTRING:
                        {
                            std::string s;
                            s.assign(v.str);
                            names.push_back(s);
                        }
                        break;
                    default:
                        throw std::invalid_argument("Input: RST: expecting LIST of STRINGs or STRING");
                    }
                }
                decltype(thing_) thing = thing_;
                return [thing, names](LookupMap_t const& map) {
                    for(auto&& s : names) {
                        thing->resetBus_.push_back(map.at(s));
                    }
                };
            }
        default:
            throw std::invalid_argument("Input: RST: expecting LIST of STRINGs or STRING");
        }
    }
    throw std::invalid_argument("Input: paramName: expecing RST");
}

template<>
DelayedLookup_fn
InstanceInterpreter<Delay>::AcceptParameter(std::string paramName, PpValue value)
{
    throw std::invalid_argument("Delay: not expecting any parameters");
}

template<>
DelayedLookup_fn
InstanceInterpreter<Noise>::AcceptParameter(std::string paramName, PpValue value)
{
    if(paramName.compare("Type") == 0) {
        switch(value.type) {
        case PpValue::PpNUMBER:
            switch(value.num) {
            case 0:
                thing_->type_ = Noise::EIGHT;
                return nullptr;
            case 1:
                thing_->type_ = Noise::SIXTEEN;
                return nullptr;
            default:
                throw std::invalid_argument("Noise: Type: expecting 0 or 1");
            }
            break;
        default:
            throw std::invalid_argument("Noise: Type: expecting a NUMBER");
        }
    } else if(paramName.compare("IN") == 0) {
        // TODO this bit of code is common for everyone
        //      refactor into template<enum> ?
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
                    for(auto&& fn : fns) fn(map);
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
                k->value_ = (double)value.num / 999; // FIXME
                thing_->Inputs().push_back(std::dynamic_pointer_cast<ABlock>(k));
                return nullptr;
            }
        default:
            throw std::invalid_argument("IN: value: expecting LIST of STRINGs or STRING");
        }
    }
    throw std::invalid_argument("Noise: paramName: expecting Type or IN");
}
