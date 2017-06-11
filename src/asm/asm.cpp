#include <memory>

#include "../asm/asm.hpp"

namespace Asm {
std::unique_ptr<Asm::DataObjectReactor> rptr = std::make_unique<Asm::DataObjectReactor>();

std::unique_ptr<Asm::TimerObjectReactor> trptr = std::make_unique<Asm::TimerObjectReactor>(*rptr);
}
