#include "blocks_interpreter.h"

int main(int argc, char* argv[])
{
    // BEGIN TEST CODE
    std::shared_ptr<IInstanceInterpreter> blocks[] = {
        GetInterpreter("Input", "I1"),
        GetInterpreter("Filter", "F1"),
        GetInterpreter("Generator", "G1"),
        GetInterpreter("Noise", "N1"),
        GetInterpreter("Delay", "D1"),
        GetInterpreter("Constant", "C1")
    };
    PpValue v;
    v.type = PpValue::PpSTRING;
    v.str = "X1";
    auto l1 = blocks[0]->AcceptParameter("RST", v);
    auto l2 = blocks[1]->AcceptParameter("A", v);
    auto l3 = blocks[2]->AcceptParameter("Glide", v);
    auto l4 = blocks[3]->AcceptParameter("IN", v);
    v.type = PpValue::PpNUMBER;
    auto l5 = blocks[5]->AcceptParameter("Value", v);
    try {
        (void) blocks[4]->AcceptParameter("Exception", v);
    } catch(std::invalid_argument e) {
        fprintf(stderr, "Exception caught for X3 param Exception value whatever: %s\n", e.what());
    }
    // END TEST CODE
}
