#include "objecttranslator.h"

#include <QTextCodec>

namespace
{

QString getCorrectUnicode(const QByteArray& ba)
{
    QTextCodec::ConverterState state;
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QString text = codec->toUnicode(ba.constData(), ba.size(), &state);
    if (state.invalidChars > 0) {
        text = QTextCodec::codecForName("GBK")->toUnicode(ba);
    }
    else {
        text = ba;
    }
    return text;
}

}

QVariantMap ObjectTranslator::getTable(lua_State* L, int index)
{
    QVariantMap table;
    if (index == -1) {
        index = -2;
    }
    
    lua_pushnil(L);
    while (lua_next(L, index) != 0) {
        QString key = getObject(L, -2).toString();
        if (key.isEmpty()) {
            lua_pop(L, 1);
            continue;
        }
        QVariant value = getObject(L, -1);
        table.insert(key, value);
        lua_pop(L, 1);
    }

    return table;
}

QVariant ObjectTranslator::getObject(lua_State* L, int index)
{
    int type = lua_type(L, index);

    switch (type)
    {
    case LUA_TNUMBER:
        return lua_tonumber(L, index);
        
    case LUA_TSTRING:
        return getCorrectUnicode(lua_tostring(L, index));

    case LUA_TBOOLEAN:
        return lua_toboolean(L, index);

    case LUA_TTABLE:
        return getTable(L, index);

    default:
        return QVariant();
    }
}

QVariantList ObjectTranslator::popValues(lua_State* L, int oldTop)
{
    int newTop = lua_gettop(L);
    if (newTop == oldTop) {
        return QVariantList();
    }
    else {
        QVariantList values;
        for (int i = oldTop + 1; i <= newTop; ++i) {
            values.push_back(getObject(L, i));
        }
        lua_settop(L, oldTop);

        return values;
    }
}

void ObjectTranslator::pushValue(lua_State* L, const QVariant& value)
{
    switch (value.type())
    {
    case QVariant::Bool:
        lua_pushboolean(L, value.toBool());
        
        break;
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Double:
    case QVariant::Char:
        lua_pushnumber(L, value.toDouble());
        
        break;
    case QVariant::String:
        lua_pushstring(L, value.toString().toStdString().c_str());
        
        break;
    case QVariant::ByteArray:
        lua_pushstring(L, value.toByteArray().toStdString().c_str());
        
        break;
    case QVariant::StringList:
    {
        QStringList v = value.toStringList();
        lua_createtable(L, v.size(), 0);
        for (int i = 0; i < v.size(); ++i) {
            lua_pushinteger(L, i + 1);
            lua_pushstring(L, v.at(i).toStdString().c_str());
            lua_settable(L, -3);
        }
    }
        
        break;
    default:
        lua_pushnil(L);

        break;
    }
}
