#ifndef LUAFUNCTION_H
#define LUAFUNCTION_H

#include "luainterface_global.h"
#include "lua51/lua.hpp"

class LUAINTERFACESHARED_EXPORT LuaFunction
{
    friend class Lua;
public:

    LuaFunction();
    LuaFunction(int reference, const LuaState& interpreter);
    LuaFunction(lua_CFunction function, const LuaState& interpreter);
    ~LuaFunction();

    LuaFunction(const LuaFunction& other);
    LuaFunction(LuaFunction&& other);
    LuaFunction& operator=(const LuaFunction& other);
    LuaFunction& operator=(LuaFunction&& other);

private:

    void push()const;

    lua_State* m_pLuaState;
    LuaStateWeak m_onwer;

    int m_reference;
    lua_CFunction m_function;

};

#endif // LUAFUNCTION_H
