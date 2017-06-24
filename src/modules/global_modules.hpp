/*
** Declarations of all module instances
** The developer is responsible to do this
** All modules must be introduced in global_modules.hpp and global_modules.cpp, because this is the starting place for the map-maker tool
*/

#pragma once

#include "InputModule.hpp"
#include "OutputModule.hpp"
#include "ProcessModule.hpp"
#include "SerializeModule.hpp"
#include "TBBModule.hpp"

extern InputModule inModule;
extern ProcessModule processModule;
extern OutputModule outModule;
extern SerializeModule serModule;
extern TBBModule tbbModule;
