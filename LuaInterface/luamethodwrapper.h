#ifndef LUAMETHODWRAPPER_H
#define LUAMETHODWRAPPER_H

#include <tuple>
#include <functional>
#include <map>
#include <unordered_map>
#include <string>

#include <QReadWriteLock>

#include "lua51/lua.hpp"

#include "objecttranslator.h"

template <typename Tuple, std::size_t N = std::tuple_size<Tuple>::value>
class TupleTraversal {
public:

    static void checkValue(lua_State* L, Tuple& tuple)
    {
        ObjectTranslator::checkValue(L, N, std::get<N - 1>(tuple));
        TupleTraversal<Tuple, N - 1>::checkValue(L, tuple);
    }

    template<typename RealTuple>
    static void pushValue(lua_State* L, Tuple& tuple, int& refNum)
    {
        constexpr std::size_t index = std::tuple_size<Tuple>::value - N; // left to right
        using type = std::tuple_element_t<index, RealTuple>;

        if (std::is_const<std::remove_reference_t<type>>::value == false && std::is_reference<type>::value == true) {
            ObjectTranslator::pushValue(L, std::get<index>(tuple));
            refNum += 1;
        }
        TupleTraversal<Tuple, N - 1>::template pushValue<RealTuple>(L, tuple, refNum);
    }

};

template<typename Tuple>
class TupleTraversal<Tuple, 1> {
public:

    static void checkValue(lua_State* L, Tuple& tuple) {
        ObjectTranslator::checkValue(L, 1, std::get<0>(tuple));
    }

    template<typename RealTuple>
    static void pushValue(lua_State* L, Tuple& tuple, int& refNum)
    {
        constexpr std::size_t index = std::tuple_size<Tuple>::value - 1; // left to right
        using type = std::tuple_element_t<index, RealTuple>;

        if (std::is_const<std::remove_reference_t<type>>::value == false && std::is_reference<type>::value == true) {
            ObjectTranslator::pushValue(L, std::get<index>(tuple));
            refNum += 1;
        }
    }

};

template<typename Tuple>
class TupleTraversal<Tuple, 0> {
public:

    static void checkValue(lua_State* L, Tuple& tuple)
    {

    }

    template<typename RealTuple>
    static void pushValue(lua_State* L, Tuple& tuple, int& refNum)
    {

    }

};

template<std::size_t ..._Indexes>
struct IndexTuple
{

};

template<std::size_t _Num, typename _Tuple = IndexTuple<>>
struct Indexes;

template<std::size_t ..._Indexes>
struct Indexes<0, IndexTuple<_Indexes...>>
{
    using type = IndexTuple<_Indexes...>;
};

template<std::size_t _Num, std::size_t ..._Indexes>
struct Indexes<_Num, IndexTuple<_Indexes...>> : Indexes<_Num - 1, IndexTuple<_Indexes..., sizeof...(_Indexes)>>
{

};

template<typename Sig>
struct FunctionTypeInfo : public FunctionTypeInfo<decltype(&Sig::operator())>
{

};

template<typename R, typename... Args>
struct FunctionTypeInfo<R(*)(Args...)>
{
    typedef R RetType;
    using Class = void;

    using TupleIndexs = typename Indexes<sizeof...(Args)>::type;
    using RealTuple = typename std::tuple<Args...>;
    using Tuple = typename std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...>;

    using Function = typename std::function<R(Args...)>;
};

template<typename T, typename R, typename... Args>
struct FunctionTypeInfo<R(T::*)(Args...)>
{
    typedef R RetType;
    typedef T Class;

    using TupleIndexs = typename Indexes<sizeof...(Args)>::type;
    using RealTuple = typename std::tuple<Args...>;
    using Tuple = typename std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...>;

    using Function = typename std::function<R(Args...)>;
};

template<typename T, typename R, typename... Args>
struct FunctionTypeInfo<R(T::*)(Args...)const>
{
    typedef R RetType;
    typedef T Class;

    using TupleIndexs = typename Indexes<sizeof...(Args)>::type;
    using RealTuple = typename std::tuple<Args...>;
    using Tuple = typename std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...>;

    using Function = typename std::function<R(Args...)>;
};


template<typename RetType, typename Tuple, typename Func, std::size_t ...indexs>
RetType callFunctionWithTuple(const Func& func, Tuple& paras, IndexTuple<indexs...>)
{
    return func(std::get<indexs>(paras)...);
}

template<typename Func>
class LuaMethodWrapperBase
{
public:

    using TupleIndexs = typename FunctionTypeInfo<Func>::TupleIndexs;
    using RealTuple = typename FunctionTypeInfo<Func>::RealTuple;
    using Tuple = typename FunctionTypeInfo<Func>::Tuple;

    using Function = typename FunctionTypeInfo<Func>::Function;

    template<typename Class>
    static void registerFunction(const LuaState& L, const std::string& name, Class* pointerThis, Func cppFunc)
    {
        registerFunction(L, name, pointerThis, cppFunc, TupleIndexs());
    }

    static void registerFunction(const LuaState& L, const std::string& name, Func cppFunc)
    {
        registerFunction(L, name, cppFunc, TupleIndexs());
    }

    static void registerLambda(const LuaState& L, const std::string& name, Function&& lambda)
    {
        registerFunction(L, name, std::move(lambda));
    }

    static void registerLambda(const LuaState& L, const std::string& name, const Function& lambda)
    {
        registerFunction(L, name, lambda);
    }

protected:

    static void GC()
    {
        for (auto iter = m_luaStates.begin(); iter != m_luaStates.end();) {
            if (iter->second.expired()) {
                m_cppFuncMap.erase(iter->first);
                iter = m_luaStates.erase(iter);
            }
            else {
                ++iter;
            }
        }
    }

    template<typename Class, std::size_t ...indexs>
    static void registerFunction(const LuaState& L, const std::string& name, Class* pointerThis, Func cppFunc, IndexTuple<indexs...>)
    {
        Function f = std::bind(cppFunc, pointerThis, std::_Ph<indexs + 1>()...);
        QWriteLocker lock(&m_lock);
        m_cppFuncMap[L.get()][name] = std::move(f);
        m_luaStates[L.get()] = L;
        GC();
    }

    template<std::size_t ...indexs>
    static void registerFunction(const LuaState& L, const std::string& name, Func cppFunc, IndexTuple<indexs...>)
    {
        Function f = std::bind(cppFunc, std::_Ph<indexs + 1>()...);
        QWriteLocker lock(&m_lock);
        m_cppFuncMap[L.get()][name] = std::move(f);
        m_luaStates[L.get()] = L;
        GC();
    }

    static void registerFunction(const LuaState& L, const std::string& name, Function&& lambda)
    {
        QWriteLocker lock(&m_lock);
        m_cppFuncMap[L.get()][name] = std::move(lambda);
        m_luaStates[L.get()] = L;
        GC();
    }

    static void registerFunction(const LuaState& L, const std::string& name, const Function& lambda)
    {
        QWriteLocker lock(&m_lock);
        m_cppFuncMap[L.get()][name] = lambda;
        m_luaStates[L.get()] = L;
        GC();
    }

    static std::map<lua_State*, std::unordered_map<std::string, Function>> m_cppFuncMap;
    static std::map<lua_State*, LuaStateWeak> m_luaStates;
    static QReadWriteLock m_lock;

};

template<typename Func>
std::map<lua_State*, std::unordered_map<std::string, typename LuaMethodWrapperBase<Func>::Function>> LuaMethodWrapperBase<Func>::m_cppFuncMap;

template<typename Func>
std::map<lua_State*, LuaStateWeak> LuaMethodWrapperBase<Func>::m_luaStates;

template<typename Func>
QReadWriteLock LuaMethodWrapperBase<Func>::m_lock;

template<typename Func, typename RetType = typename FunctionTypeInfo<Func>::RetType>
class LuaMethodWrapper : public LuaMethodWrapperBase<Func>
{
public:

    using Tuple = typename LuaMethodWrapperBase<Func>::Tuple;
    using RealTuple = typename LuaMethodWrapperBase<Func>::RealTuple;
    using TupleIndexs = typename LuaMethodWrapperBase<Func>::TupleIndexs;

    static int luaCall(lua_State* L)
    {
        QReadLocker lock(&LuaMethodWrapperBase<Func>::m_lock);

        //
        Tuple paras;
        TupleTraversal<Tuple>::checkValue(L, paras);
        int retNum = 1;

        //
        std::string name = lua_tostring(L, lua_upvalueindex(1));
        RetType ret = callFunctionWithTuple<RetType>(LuaMethodWrapperBase<Func>::m_cppFuncMap[L][name], paras, TupleIndexs());

        //
        ObjectTranslator::pushValue(L, ret);
        TupleTraversal<Tuple>::template pushValue<RealTuple>(L, paras, retNum);
        return retNum;
    }

};

template<typename Func>
class LuaMethodWrapper<Func, void> : public LuaMethodWrapperBase<Func>
{
public:

    using Tuple = typename LuaMethodWrapperBase<Func>::Tuple;
    using RealTuple = typename LuaMethodWrapperBase<Func>::RealTuple;
    using TupleIndexs = typename LuaMethodWrapperBase<Func>::TupleIndexs;

    static int luaCall(lua_State* L)
    {
        QReadLocker lock(&LuaMethodWrapperBase<Func>::m_lock);

        //
        Tuple paras;
        TupleTraversal<Tuple>::checkValue(L, paras);
        int retNum = 0;

        //
        std::string name = lua_tostring(L, lua_upvalueindex(1));
        callFunctionWithTuple<void>(LuaMethodWrapperBase<Func>::m_cppFuncMap[L][name], paras, TupleIndexs());

        //
        TupleTraversal<Tuple>::template pushValue<RealTuple>(L, paras, retNum);
        return retNum;
    }

};

#endif // LUAMETHODWRAPPER_H
