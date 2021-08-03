#ifndef OBJECTTRANSLATOR_H
#define OBJECTTRANSLATOR_H

#include "luafunction.h"

#include <QVariant>

class LUAINTERFACESHARED_EXPORT ObjectTranslator
{
public:

    struct MethodTranslatorHelper
    {
        QString name;
        lua_CFunction pFunc;
    };

    static QVariantList popValues(lua_State* L, int oldTop);
    static QVariant getObject(lua_State* L, int index);
    static QVariantMap getTable(lua_State* L, int index);

    template<typename T, typename ...Args>
    static void checkValue(lua_State* L, int index, T& first, Args&... dests)
    {
        first = qvariant_cast<T>(getObject(L, index));
        checkValue(L, index + 1, dests...);
    }

    static inline void checkValue(lua_State* L, int index)
    {

    }

    static inline void checkValue(lua_State* L, int index, QVariant& dest)
    {
        dest = getObject(L, index);
    }

    template<typename T, typename ...Args>
    static void pushValue(lua_State* L, const T& first, const Args&... values)
    {
        pushValue(L, first);
        pushValue(L, values...);
    }

    template<typename T>
    static inline void pushValue(lua_State* L, const T& value)
    {
        pushValue(L, QVariant::fromValue(value));
    }

    static inline void pushValue(lua_State* L, const char* str)
    {
        lua_pushstring(L, str);
    }

    static inline void pushValue(lua_State* L, lua_CFunction func)
    {
        lua_pushcfunction(L, func);
    }

    static inline void pushValue(lua_State* L, const MethodTranslatorHelper& func)
    {
        lua_pushstring(L, func.name.toStdString().c_str());
        lua_pushcclosure(L, func.pFunc, 1);
    }

    static inline void pushValue(lua_State* L)
    {

    }

    static void pushValue(lua_State* L, const QVariant& value);

};

#endif // OBJECTTRANSLATOR_H
