// lua.hpp
// Lua header files for C++
// <<extern "C">> not supplied automatically because Lua also compiles as C++

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <memory>

typedef std::shared_ptr<lua_State> LuaState;
typedef std::weak_ptr<lua_State> LuaStateWeak;