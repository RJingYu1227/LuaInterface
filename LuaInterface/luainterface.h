#ifndef LUAINTERFACE_H
#define LUAINTERFACE_H

#include "luamethodwrapper.h"

#include <stdexcept>

class LUAINTERFACESHARED_EXPORT Lua
{
public:
    
    Lua();
    ~Lua();

    /**
     * @brief 将值注册到lua环境中
     * 
     * @tparam 参见ObjectTranslator::pushValue可以支持的类型 
     * @param fullPath "a"表示变量a；"t.a"表示表元素t[a]；以此类推
     * @param value 
     */
    template<typename T>
    void setObject(const QString& fullPath, const T& value)
    {
        int oldTop = lua_gettop(m_pLuaState);
        
        QStringList path = fullPath.split('.');
        if (path.length() == 1) {
            ObjectTranslator::pushValue(m_pLuaState, value);
            lua_setglobal(m_pLuaState, fullPath.toStdString().c_str());
        }
        else {
            lua_getglobal(m_pLuaState, path.at(0).toStdString().c_str());
            for (int i = 1; i < path.size() - 1; ++i) {
                lua_pushstring(m_pLuaState, path.at(i).toStdString().c_str());
                lua_gettable(m_pLuaState, -2);
            }
            lua_pushstring(m_pLuaState, path.last().toStdString().c_str());
            ObjectTranslator::pushValue(m_pLuaState, value);
            lua_settable(m_pLuaState, -3);
        }
        
        lua_settop(m_pLuaState, oldTop);
    }

    template<typename T>
    T getObject(const QString& fullPath, QTypeInfo<T> typeInfo = QTypeInfo<T>())
    {
        T ret;
        int oldTop = lua_gettop(m_pLuaState);
        
        QStringList path = fullPath.split('.');
        lua_getglobal(m_pLuaState, path.at(0).toStdString().c_str());
        for (int i = 1; i < path.size(); ++i) {
            lua_pushstring(m_pLuaState, path.at(i).toStdString().c_str());
            lua_gettable(m_pLuaState, -2);
            if (lua_isnoneornil(m_pLuaState, -1)) {
                break;
            }
        }
        ObjectTranslator::checkValue(m_pLuaState, -1, ret);

        lua_settop(m_pLuaState, oldTop);
        return ret;
    }

    LuaFunction getFunction(const QString& fullPath);

    template<typename Class, typename Func>
    void registerFunction(const QString& name, Class* pointerThis, Func func)
    {
        LuaMethodWrapper<Func>::registerFunction(m_onwer, name.toStdString(), pointerThis, func);
        ObjectTranslator::MethodTranslatorHelper helper = { name, &LuaMethodWrapper<Func>::luaCall };
        setObject(name, helper);
    }

    template<typename Func>
    void registerFunction(const QString& name, Func func)
    {
        LuaMethodWrapper<Func>::registerFunction(m_onwer, name.toStdString(), func);
        ObjectTranslator::MethodTranslatorHelper helper = { name, &LuaMethodWrapper<Func>::luaCall };
        setObject(name, helper);
    }

    /**
     * @brief 针对lambda的优化版本，相比于registerFunction来说，减少了拷贝构造的开销
     */
    template<typename Func>
    void registerLambda(const QString& name, Func&& func)
    {
        LuaMethodWrapper<decltype(&Func::operator())>::registerLambda(m_onwer, name.toStdString(), std::move(func));
        ObjectTranslator::MethodTranslatorHelper helper = { name, &LuaMethodWrapper<decltype(&Func::operator())>::luaCall };
        setObject(name, helper);
    }

    /**
     * @brief 针对lambda的优化版本，相比于registerFunction来说，减少了拷贝构造的开销
     */
    template<typename Func>
    void registerLambda(const QString& name, const Func& func)
    {
        LuaMethodWrapper<decltype(&Func::operator())>::registerLambda(m_onwer, name.toStdString(), func);
        ObjectTranslator::MethodTranslatorHelper helper = { name, &LuaMethodWrapper<decltype(&Func::operator())>::luaCall };
        setObject(name, helper);
    }

    void registerFunction(const QString& libName, luaL_Reg* regs)
    {
        luaL_register(m_pLuaState, libName.toStdString().c_str(), regs);
    }

    void newTable(const QString& fullPath);
    QVariantList doString(const QString& chunk);
    QVariantList doFile(const QString& fileName);

    template<typename ...Args>
    QVariantList callFunction(const LuaFunction& function, const Args&... args)
    {
        if (m_pLuaState != function.m_pLuaState) {
            throw std::runtime_error("Invalid Call: The Function Was Created By Another lua_State.");
        }

        int oldTop = lua_gettop(m_pLuaState);
        function.push();
        ObjectTranslator::pushValue(m_pLuaState, args...);
        if (lua_pcall(m_pLuaState, sizeof...(args), LUA_MULTRET, 0) == 0) {
            return ObjectTranslator::popValues(m_pLuaState, oldTop);
        }
        else {
            throwExceptionFromError(oldTop);
        }
    }

    int openlibs(lua_CFunction func)
    {
        return func(m_pLuaState);
    }

private:

    void throwExceptionFromError(int oldTop);

    lua_State* m_pLuaState;
    LuaState m_onwer;

};

#endif // LUAINTERFACE_H
