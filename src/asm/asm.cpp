/*
** Definitions of DataObjectReactor and TimerObjectReactor
** Should be treaded as singletons
*/

#include <memory>

#include "../asm/asm.hpp"

namespace Asm {
std::unique_ptr<Asm::DataObjectReactor> pDOR = std::make_unique<Asm::DataObjectReactor>();
std::unique_ptr<Asm::TimerObjectReactor> pTOR = std::make_unique<Asm::TimerObjectReactor>(*pDOR);
}
