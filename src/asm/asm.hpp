#pragma once

#include "dataobjectreactor.hpp"
#include "dataobject.hpp"
#include "../maker/maker_do.hpp"
#include "linkobject.hpp"
#include "../maker/maker_lo.hpp"
#include "timerobject.hpp"
#include "timerobjectreactor.hpp"

namespace Asm {
extern std::unique_ptr<Asm::DataObjectReactor> pDOR;

extern std::unique_ptr<Asm::TimerObjectReactor> pTOR;
}
