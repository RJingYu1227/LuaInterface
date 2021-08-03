#include "luafunction.h"
#include "objecttranslator.h"

LuaFunction::LuaFunction() :
    m_pLuaState(nullptr),
    m_reference(0),
    m_function(nullptr)
{
    
}

LuaFunction::LuaFunction(int reference, const LuaState& interpreter) :
    m_pLuaState(interpreter.get()),
    m_onwer(interpreter),
    m_reference(reference),
    m_function(nullptr)
{

}

LuaFunction::LuaFunction(lua_CFunction function, const LuaState& interpreter) :
    m_pLuaState(interpreter.get()),
    m_onwer(interpreter),
    m_reference(0),
    m_function(function)
{

}

LuaFunction::~LuaFunction()
{
    if (m_onwer.expired() == false && m_reference != 0) {
        lua_unref(m_pLuaState, m_reference);
    }
}

LuaFunction::LuaFunction(const LuaFunction& other) :
    m_pLuaState(other.m_pLuaState),
    m_onwer(other.m_onwer),
    m_reference(0),
    m_function(other.m_function)
{
    if (m_onwer.expired() == false && other.m_reference != 0) {
        lua_getref(m_pLuaState, other.m_reference);
        m_reference = lua_ref(m_pLuaState, 1);
    }
}

LuaFunction::LuaFunction(LuaFunction&& other) :
    m_pLuaState(other.m_pLuaState),
    m_onwer(other.m_onwer),
    m_reference(other.m_reference),
    m_function(other.m_function)
{
    other.m_pLuaState = nullptr;
    other.m_onwer.reset();
    other.m_reference = 0;
    other.m_function = nullptr;
}

LuaFunction& LuaFunction::operator=(const LuaFunction& other)
{
    if (m_onwer.expired() == false && m_reference != 0) {
        lua_unref(m_pLuaState, m_reference);
    }

    m_pLuaState = other.m_pLuaState;
    m_onwer = other.m_onwer;

    if (m_onwer.expired() == false && other.m_reference != 0) {
        lua_getref(m_pLuaState, other.m_reference);
        m_reference = lua_ref(m_pLuaState, 1);
    }
    else {
        m_reference = 0;
    }

    m_function = other.m_function;

    return *this;
}

LuaFunction& LuaFunction::operator=(LuaFunction&& other)
{
    if (m_onwer.expired() == false && m_reference != 0) {
        lua_unref(m_pLuaState, m_reference);
    }

    m_pLuaState = other.m_pLuaState;
    m_onwer = other.m_onwer;
    m_reference = other.m_reference;
    m_function = other.m_function;

    other.m_pLuaState = nullptr;
    other.m_onwer.reset();
    other.m_reference = 0;
    other.m_function = nullptr;

    return *this;
}

void LuaFunction::push()const
{
    if (m_reference != 0) {
        lua_getref(m_pLuaState, m_reference);
    }
    else {
        ObjectTranslator::pushValue(m_pLuaState, m_function);
    }
}
