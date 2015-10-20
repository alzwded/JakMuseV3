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
#include "notes_interpreter.h"
#include <assert.h>
#include <sstream>
#include <cmath>
#include <type_traits>
#include <exception>

std::shared_ptr<ANotesInterpreter>
GetNotesInterpreter(std::string instanceType, std::string name)
{
    if(instanceType.compare("NOTES") == 0) {
        return std::make_shared<NotesInterpreter>(name);
    } else if(instanceType.compare("PCM") == 0) {
        return std::make_shared<PCMInterpreter>(name);
    } else {
        throw std::invalid_argument(std::string() + "Unknown notation: " + instanceType);
    }
}

size_t ComputeDuration(int divisor, std::vector<Note> const& notes)
{
    if(divisor <= 0) return 0;
    size_t m = 0;
    m = std::accumulate(notes.begin(), notes.end(), m,
            [](size_t m, Note const& n) -> size_t {
                if(n.duration <= 0) return m;
                else return m + n.duration;
            });
    return 44100.0 / divisor * m;
}

size_t NotesInterpreter::Duration() const
{
    return ComputeDuration(divisor, notes);
}

size_t PCMInterpreter::Duration() const
{
    return ComputeDuration(stride, notes);
}

void NotesInterpreter::Fill(LookupMap_t const& map)
{
    auto&& foundIt = map.find(name_);
    auto&& found = *foundIt;
    assert(found); // should never be false
    assert(divisor > 0); // should never be false

    std::remove_reference<decltype(found->InputBuffer())>::type* pBuffer;
    try {
        pBuffer = &found->InputBuffer();
    } catch(std::exception e) {
        throw std::invalid_argument(std::string("NotesInterpreter: ") + name_ + " is not supported. " + e.what());
    }
    decltype(found->InputBuffer()) buffer = *pBuffer;

    double mult = 44100.0 / divisor;
    for(auto&& it = notes.begin(); it != notes.end(); ++it) {
        auto&& n = *it;
        int newDuration = (int)(mult * n.duration);
        if(newDuration <= 0) {
            std::stringstream ss;
            ss << "NotesInterpreter: " << name_ << "; ";
            ss << (it - notes.begin()) << "th note results in invalid duration";
            throw std::invalid_argument(ss.str());
        }
        ResetKind kind = ResetKind::NOTE;
        if(n.value < 1.0e-15) kind = ResetKind::REST;
        auto chunk = std::make_tuple(newDuration, n.value, kind);
        buffer << chunk;
    }
}

void PCMInterpreter::Fill(LookupMap_t const& map)
{
    auto&& pFound = map.find(name_);
    assert(pFound != map.end()); // should never be false
    assert(stride > 0); // should never be false
    auto&& found = *pFound;

    std::remove_reference<decltype(found->InputBuffer())>::type* pBuffer;
    try {
        pBuffer = &found->InputBuffer();
    } catch(std::exception e) {
        throw std::invalid_argument(std::string("PCMInterpreter: ") + name_ + " is not supported. " + e.what());
    }
    auto&& buffer = *pBuffer;

    double mult = 44100.0 / stride; // FIXME not really as intended, but it really helps sync the two; eff semantics!
    for(auto&& it = notes.begin(); it != notes.end(); ++it) {
        auto&& n = *it;
        int total = (int)(mult * n.duration);
        if(total <= 0) {
            std::stringstream ss;
            ss << "PCMInterpreter: " << name_ << "; ";
            ss << (it - notes.begin()) << "th note results in invalid duration";
            throw std::invalid_argument(ss.str());
        }
        ResetKind kind = ResetKind::NOTE;
        switch(interpolation) {
        case TRUNC:
            {
                auto chunk = std::make_tuple(total, ((double)n.value / 999.0), kind);
                buffer << chunk;
            }
            break;
        case LINEAR:
            {
                auto it2 = it;
                ++it2;
                if(it2 == notes.end()) continue;
                auto&& n2 = *it2;
                double v1 = n.value / 999.0 * 2.0 - 1.0;
                double v2 = n2.value / 999.0 * 2.0 - 1.0;
                for(int i = 0; i < total; ++i) {
                    double pc = (double)i/total;
                    double samp = v1 * (1 - pc) + v2 * pc;
                    buffer << std::make_tuple(1, samp, kind);
                }
            }
            break;
        case COSINE:
            {
                auto it2 = it;
                ++it2;
                if(it2 == notes.end()) continue;
                auto&& n2 = *it2;
                double v1 = n.value / 999.0 * 2.0 - 1.0;
                double v2 = n2.value / 999.0 * 2.0 - 1.0;
                for(int i = 0; i < total; ++i) {
                    double pc = (double)i/total;
                    double qht = (1.0 - cos(M_PI * pc)) / 2.0;
                    double samp = v1 * (1.0 - qht) + v2 * qht;
                    buffer << std::make_tuple(1, samp, kind);
                }
            }
            break;
        }
    }
}

void NotesInterpreter::AcceptParameter(std::string paramName, PpValue value)
{
    if(paramName.compare("Divisor") == 0) {
        switch(value.type) {
        case PpValue::PpNUMBER:
            if(value.num <= 0) throw std::invalid_argument(std::string() + "NotesInterpreter: " + name_ + " param " + paramName + " expected a number > 0");
            divisor = value.num;
            break;
        default:
            throw std::invalid_argument(std::string() + "NotesInterpreter: " + name_ + " param " + paramName + " expected a NUMBER");
        }
    } else if(paramName.compare("Notes") == 0) {
        switch(value.type) {
        case PpValue::PpLIST:
            {
                for(PpValueList* p = value.list; p; p = p->next) {
                    PpValue v = p->value;
                    switch(v.type) {
                    case PpValue::PpNOTE:
                        notes.push_back(v.note);
                        break;
                    default:
                        throw std::invalid_argument(std::string() + "NotesInterpreter: " + name_ + " param " + paramName + " expected a LIST of NOTEs");
                    }
                }
            }
            break;
        default:
            throw std::invalid_argument(std::string() + "NotesInterpreter: " + name_ + " param " + paramName + " expected a LIST of NOTEs");
        }
    } else {
        throw std::invalid_argument(std::string() + "NotesInterpreter: " + name_ + " unexpected parameter " + paramName);
    }
}

void PCMInterpreter::AcceptParameter(std::string paramName, PpValue value)
{
    if(paramName.compare("Stride") == 0) {
        switch(value.type) {
        case PpValue::PpNUMBER:
            stride = value.num;
            break;
        default:
            throw std::invalid_argument(std::string() + "PCMInterpreter: " + name_ + " param " + paramName + " expected a NUMBER");
        }
    } else if(paramName.compare("Samples") == 0) {
        switch(value.type) {
        case PpValue::PpLIST:
            {
                for(PpValueList* p = value.list; p; p = p->next) {
                    PpValue v = p->value;
                    decltype(v.num) n1, n2;
                    switch(v.type) {
                    case PpValue::PpNUMBER:
                        n1 = v.num;
                        break;
                    default:
                        throw std::invalid_argument(std::string() + "PCMInterpreter: " + name_ + " param " + paramName + " expected a LIST of NUMBERs");
                    }
                    if(!(p = p->next)) {
                        throw std::invalid_argument(std::string() + "PCMInterpreter: " + name_ + " param " + paramName + " expected an even number of samples");
                    }
                    v = p->value;
                    switch(v.type) {
                    case PpValue::PpNUMBER:
                        n2 = v.num;
                        break;
                    default:
                        throw std::invalid_argument(std::string() + "PCMInterpreter: " + name_ + " param " + paramName + " expected a LIST of NUMBERs");
                    }
                    Note n = { n1, n2 };
                    notes.push_back(n);
                }
            }
            break;
        default:
            throw std::invalid_argument(std::string() + "PCMInterpreter: " + name_ + " param " + paramName + " expected a LIST of NUMBERs");
        }
    } else if(paramName.compare("Interpolation") == 0) {
        switch(value.type) {
        case PpValue::PpSTRING:
            {
                std::string s;
                s.assign(value.str);
                if(s.compare("Trunc") == 0) {
                    interpolation = TRUNC;
                } else if(s.compare("Cosine") == 0) {
                    interpolation = COSINE;
                } else if(s.compare("Linear") == 0) {
                    interpolation = LINEAR;
                } else {
                    throw std::invalid_argument(std::string() + "PCMInterpreter: " + name_ + " param " + paramName + " expected Trunc, Linear, Cosine");
                }
            }
            break;
        default:
            throw std::invalid_argument(std::string() + "PCMInterpreter: " + name_ + " param " + paramName + " expected Trunc, Linear, Cosine");
        }
    } else {
        throw std::invalid_argument(std::string() + "PCMInterpreter: " + name_ + " unexpected parameter " + paramName);
    }
}
