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

#include "blocks_interpreter.h"
#include "log.h"

template struct InstanceInterpreter<Constant>;
template struct InstanceInterpreter<Generator>;
template struct InstanceInterpreter<Filter>;
template struct InstanceInterpreter<Input>;
template struct InstanceInterpreter<Delay>;
template struct InstanceInterpreter<Noise>;
template struct InstanceInterpreter<Output>;

std::shared_ptr<IInstanceInterpreter> GetInterpreter(std::string instanceType, std::string name)
{
#define INSTANCE_DECLARATION_START() if(0)

#define DECLARE_INSTANCE(TYPE) } else if(instanceType.compare(#TYPE) == 0) {\
    do{ return std::dynamic_pointer_cast<IInstanceInterpreter>(std::make_shared<InstanceInterpreter<TYPE>>(name)); }while(0)

#define INSTANCE_DECLARATION_END() else {\
    throw std::invalid_argument(std::string("Unknown instance type ") + instanceType);\
}do{}while(0)

    INSTANCE_DECLARATION_START() {
        DECLARE_INSTANCE(Constant);
        DECLARE_INSTANCE(Generator);
        DECLARE_INSTANCE(Input);
        DECLARE_INSTANCE(Filter);
        DECLARE_INSTANCE(Delay);
        DECLARE_INSTANCE(Noise);
        DECLARE_INSTANCE(Output);
    } INSTANCE_DECLARATION_END();

#undef INSTANCE_DECLARATION_START
#undef DECLARE_INSTANCE
#undef INSTANCE_DECLARATION_END
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
        thing_->value_ = (double)value.num / 999.0;
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
                        thing_->WT.table_.push_back((double)v.num / 999.0);
                        break;
                    default:
                        throw std::invalid_argument("WT: value: Expecing a LIST of NUMBERs");
                    }
                }
                return nullptr;
            }
        default:
            throw std::invalid_argument("WT: value: Expecing a LIST of NUMBERs");
        }
    } else if(paramName.compare("GlideOnRest") == 0) {
        switch(value.type) {
        case PpValue::PpNUMBER:
            if(value.num == 0 || value.num == 1) {
                thing_->GlideOnRest = (value.num != 0);
                return nullptr;
            } else {
                /*FALLTHROUGH*/
            }
        default:
            throw std::invalid_argument("WT: GlideOnRest: expecting 0 or 1");
        }
    } else if(paramName.compare("RST") == 0) {
        switch(value.type) {
        case PpValue::PpSTRING:
            {
                std::string s;
                s.assign(value.str);
                auto thing = thing_;
                return [thing, s](LookupMap_t const& map) {
                    thing->ResetBy = map.at(s);
                };
            }
        default:
            throw std::invalid_argument("WT: RST: expecting a block name");
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
                k->value_ = (double)value.num / 999.0; // FIXME
                thing_->TGlide = std::dynamic_pointer_cast<ABlock>(k);
                k->Tick3();
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
                            name.assign(v.str);
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
                k->value_ = (double)value.num / 22050.0; // FIXME
                k->Tick3();
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
#define NUMBER_OR_INPUT(PARAM, SCALE) do{\
        switch(value.type) {\
        case PpValue::PpNUMBER:\
            {\
                std::shared_ptr<Constant> c(new Constant);\
                c->value_ = (double)value.num / SCALE;\
                LOGF(LOG_INTERPRETER, "%s = %f", #PARAM, c->value_); \
                thing_->PARAM = std::dynamic_pointer_cast<ABlock>(c);\
                c->Tick3(); /* force buffer transfer */\
                return nullptr;\
            }\
        case PpValue::PpSTRING:\
            {\
                std::string name;\
                name.assign(value.str);\
                auto thing = thing_;\
                LOGF(LOG_INTERPRETER, "%s = {%s}", #PARAM, value.str); \
                return [thing, name](LookupMap_t const& map) {\
                    thing->PARAM = map.at(name);\
                };\
            }\
        default:\
            throw std::invalid_argument("Filter: " #PARAM ": expecting STRING or NUMBER");\
        }\
}while(0)
    if(paramName.compare("A") == 0) {
        NUMBER_OR_INPUT(A, 999.0);
    } else if(paramName.compare("D") == 0) {
        NUMBER_OR_INPUT(D, 999.0);
    } else if(paramName.compare("S") == 0) {
        NUMBER_OR_INPUT(S, 999.0);
    } else if(paramName.compare("R") == 0) {
        NUMBER_OR_INPUT(R, 999.0);
    } else if(paramName.compare("ResetADSR") == 0) {
        switch(value.type) {
        case PpValue::PpNUMBER:
            if(value.num > 1) throw std::invalid_argument("Filter: ResetADSR: expecting 1 or 0");
            thing_->ResetADSR = value.num != 0;
            return nullptr;
        default:
            throw std::invalid_argument("Filter: ResetADSR expecting NUMBER");
        }
    } else if(paramName.compare("InvertADSR") == 0) {
        switch(value.type) {
        case PpValue::PpNUMBER:
            if(value.num > 1) throw std::invalid_argument("Filter: InvertADSR: expecting 1 or 0");
            thing_->InvertADSR = value.num != 0;
            return nullptr;
        default:
            throw std::invalid_argument("Filter: InvertADSR: expecting NUMBER");
        }
    } else if(paramName.compare("Mixing") == 0) {
        switch(value.type) {
        case PpValue::PpSTRING:
            {
                std::string s;
                s.assign(value.str);
                if(s.compare("Cut") == 0) {
                    thing_->mixing_ = Filter::Cut;
                } else if(s.compare("Flatten") == 0) {
                    thing_->mixing_ = Filter::Flatten;
                } else {
                    throw std::invalid_argument("Filter: Mixing: expecting Cut or Flatten");
                }
            }
            return nullptr;
        default:
            throw std::invalid_argument("Filter: Mixing: expecting a STRING");
        }
    } else if(paramName.compare("Low") == 0) {
        NUMBER_OR_INPUT(Lo, 22050.0);
    } else if(paramName.compare("High") == 0) {
        NUMBER_OR_INPUT(Hi, 22050.0);
    } else if(paramName.compare("K") == 0) {
        NUMBER_OR_INPUT(K, 999.0);
    } else if(paramName.compare("RST") == 0) {
        switch(value.type) {
        case PpValue::PpSTRING:
            {
                std::string s;
                s.assign(value.str);
                auto thing = thing_;
                return [thing, s](LookupMap_t const& map) {
                    thing->ResetBy = map.at(s);
                };
            }
        default:
            throw std::invalid_argument("WT: RST: expecting a block name");
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
                            name.assign(v.str);
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
                k->Tick3();
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
    if(paramName.compare("OnRest") == 0) {
        switch(value.type)
        {
        case PpValue::PpSTRING:
            {
                std::string s;
                s.assign(value.str);
                if(s.compare("RetainValue") == 0) {
                    thing_->OnRest = Input::RetainValue;
                    return nullptr;
                } else if(s.compare("Zero") == 0) {
                    thing_->OnRest = Input::Zero;
                    return nullptr;
                } else{
                    /*FALLTHROUGH*/
                }
            }
        default:
            throw std::invalid_argument("Input: OnRest: expecting RetainValue or Zero");
        }
    }
    throw std::invalid_argument("Input: paramName: expecing OnRest");
}

template<>
DelayedLookup_fn
InstanceInterpreter<Delay>::AcceptParameter(std::string paramName, PpValue value)
{
    if(paramName.compare("Amount") == 0) {
        switch(value.type) {
        case PpValue::PpNUMBER: 
            {
                if(//value.num > 999 ||
                        value.num < 0) throw std::invalid_argument("Delay: Amount: should be between 0 and 999");
                std::shared_ptr<Constant> k(new Constant);
                k->value_ = (double)value.num / 999.0; // FIXME
                k->Tick3();
                thing_->delay_ = k;
                return nullptr;
            }
        case PpValue::PpSTRING:
            {
                std::string name;
                name.assign(value.str);
                thing_sp thing = thing_;
                return [thing, name](LookupMap_t const& map) {
                    thing->delay_ = map.at(name);
                }; 
            }
        default:
            throw std::invalid_argument("Delay: Amount: expecting a NUMBER or a STRING");
        }
    } else if(paramName.compare("RST") == 0) {
        switch(value.type) {
        case PpValue::PpSTRING:
            {
                std::string s;
                s.assign(value.str);
                auto thing = thing_;
                return [thing, s](LookupMap_t const& map) {
                    thing->ResetBy = map.at(s);
                };
            }
        default:
            throw std::invalid_argument("WT: RST: expecting a block name");
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
                            name.assign(v.str);
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
        default:
            throw std::invalid_argument("IN: value: expecting LIST of STRINGs or STRING");
        }
    } else {
        throw std::invalid_argument("Delay: unknown param; expecting IN, Amount");
    }
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
    } else if(paramName.compare("RST") == 0) {
        switch(value.type) {
        case PpValue::PpSTRING:
            {
                std::string s;
                s.assign(value.str);
                auto thing = thing_;
                return [thing, s](LookupMap_t const& map) {
                    thing->ResetBy = map.at(s);
                };
            }
        default:
            throw std::invalid_argument("WT: RST: expecting a block name");
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
                            name.assign(v.str);
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
                k->Tick3();
                thing_->Inputs().push_back(std::dynamic_pointer_cast<ABlock>(k));
                return nullptr;
            }
        default:
            throw std::invalid_argument("IN: value: expecting LIST of STRINGs or STRING");
        }
    }
    throw std::invalid_argument("Noise: paramName: expecting Type or IN");
}

template<>
DelayedLookup_fn
InstanceInterpreter<Output>::AcceptParameter(
        std::string paramName,
        PpValue value)
{
    if(paramName.compare("IN") == 0) {
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
                            name.assign(v.str);
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
        default:
            throw std::invalid_argument("IN: value: expecting LIST of STRINGs or STRING");
        }
    } else if(paramName.compare("Mixing") == 0) {
        switch(value.type) {
        case PpValue::PpSTRING:
            {
                std::string s;
                s.assign(value.str);
                if(s.compare("Cut") == 0) {
                    thing_->mixing_ = Output::Cut;
                } else if(s.compare("Flatten") == 0) {
                    thing_->mixing_ = Output::Flatten;
                } else {
                    throw std::invalid_argument("Output: Mixing: expecting Cut or Flatten");
                }
            }
            return nullptr;
        default:
            throw std::invalid_argument("Output: Mixing: expecting a STRING");
        }
    } else {
        throw std::invalid_argument("paramName: Expecting Mixing, IN");
    }
}

LookupMap_t::mapped_type LookupMap_t::at(LookupMap_t::key_type name) const
{
    auto&& found = std::find_if(data_.begin(), data_.end(),
            [name](value_type const& v) -> bool {
                return v->Name().compare(name) == 0;
            });
    if(found == data_.end()) throw std::out_of_range(name + " not found.");
    return (*found)->Block();
}

LookupMap_t::iterator LookupMap_t::find(LookupMap_t::key_type name) const
{
    auto&& found = std::find_if(data_.begin(), data_.end(),
            [name](value_type const& v) -> bool {
                return v->Name().compare(name) == 0;
            });
    return found;
}

template<>
CannonicalStream& InstanceInterpreter<Input>::InputBuffer()
{
    return thing_->stream_;
}

template<typename T>
CannonicalStream& InstanceInterpreter<T>::InputBuffer()
{
    throw std::invalid_argument("Block does not support NOTES input");
}
