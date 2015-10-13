#include "notes_interpreter.h"
#include <assert.h>
#include <sstream>

void NotesInterpreter::Fill(LookupMap_t const& map)
{
    auto&& found = map.at(name_);
    assert(found); // should never be false
    assert(divisor > 0); // should never be false

    decltype(found->InputBuffer()) buffer;
    try {
        buffer = found->InputBuffer();
    } catch(...) {
        std::throw_with_nested(std::invalid_argument(std::string("NotesInterpreter: ") + name + " is not supported.");
    }

    double mult = 44100.0 / divisor;
    for(auto&& it = notes.begin(); it != notes.end(); ++it) {
        auto&& n = *it;
        int newDuration = (int)(mult * n.duration);
        if(newDuration <= 0) {
            std::stringstream ss;
            ss << "NotesInterpreter: " << name << "; ";
            ss << (it - notes.begin()) << "th note results in invalid duration";
            throw std::invalid_argument(ss.str());
        }
        ResetKind kind = ResetKind::NOTE;
        if(n.value == 0.0) kind = ResetKind::REST;
        auto chunk = std::make_tuple(newDuration, n.value, kind);
        buffer << chunk;
    }
}
