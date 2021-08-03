#include "luainterface.h"

extern "C" {
    int luaopen_bit(lua_State* L);
    int luaopen_lfs(lua_State* L);
}

namespace
{

void luaStateDeleter(lua_State* L)
{
    if (L != nullptr) {
        lua_close(L);
    }
}

}

using namespace::std;

Lua::Lua() :
    m_pLuaState(lua_open()),
    m_onwer(m_pLuaState, luaStateDeleter)
{
    luaL_openlibs(m_pLuaState);
    luaopen_bit(m_pLuaState);
    luaopen_lfs(m_pLuaState);
}

Lua::~Lua()
{
    
}

LuaFunction Lua::getFunction(const QString& fullPath)
{
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
    LuaFunction ret(lua_ref(m_pLuaState, 1), m_onwer);

    lua_settop(m_pLuaState, oldTop);
    return ret;
}

void Lua::newTable(const QString& fullPath)
{
    int oldTop = lua_gettop(m_pLuaState);

    QStringList path = fullPath.split('.');
    if (path.length() == 1) {
        lua_newtable(m_pLuaState);
        lua_setglobal(m_pLuaState, fullPath.toStdString().c_str());
    }
    else {
        lua_getglobal(m_pLuaState, path.at(0).toStdString().c_str());
        for (int i = 1; i < path.size() - 1; ++i) {
            lua_pushstring(m_pLuaState, path.at(i).toStdString().c_str());
            lua_gettable(m_pLuaState, -2);
        }
        lua_pushstring(m_pLuaState, path.last().toStdString().c_str());
        lua_newtable(m_pLuaState);
        lua_settable(m_pLuaState, -3);
    }

    lua_settop(m_pLuaState, oldTop);
}

QVariantList Lua::doString(const QString& chunk)
{
    int oldTop = lua_gettop(m_pLuaState);
    string stdChunk = chunk.toStdString();

    if (luaL_loadbuffer(m_pLuaState, stdChunk.c_str(), stdChunk.size(), "chunk") == 0) {
        try {
            if (lua_pcall(m_pLuaState, 0, LUA_MULTRET, 0) == 0) {
                return ObjectTranslator::popValues(m_pLuaState, oldTop);
            }
            else {
                throwExceptionFromError(oldTop);
            }
        }
        catch (runtime_error& e) {
            int length = (stdChunk.size() > 50) ? 50 : stdChunk.size();
            throw runtime_error(stdChunk.substr(0, length) + "..." + e.what());
        }
    }
    else {
        throwExceptionFromError(oldTop);
    }

    return QVariantList();
}

QVariantList Lua::doFile(const QString& fileName)
{
    int oldTop = lua_gettop(m_pLuaState);
    string stdFileName = fileName.toStdString();

    if (luaL_loadfile(m_pLuaState, stdFileName.c_str()) == 0) {
        try {
            if (lua_pcall(m_pLuaState, 0, LUA_MULTRET, 0) == 0) {
                return ObjectTranslator::popValues(m_pLuaState, oldTop);
            }
            else {
                throwExceptionFromError(oldTop);
            }
        }
        catch (runtime_error& e) {
            throw runtime_error(stdFileName + " " + e.what());
        }
    }
    else {
        throwExceptionFromError(oldTop);
    }

    return QVariantList();
}

void Lua::throwExceptionFromError(int oldTop)
{
    QString err = ObjectTranslator::getObject(m_pLuaState, -1).toString();
    lua_settop(m_pLuaState, oldTop);

    throw runtime_error(err.toStdString());
}
