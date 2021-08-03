# LuaInterface

1. 参考Google开源的采用C#实现的LuaInterface库；本项目使用C++实现其部分接口

2. 最核心的部分是使用C++的模板等特性，实现LuaMethoWrapper

3. 关于Qt部分，只是使用了QVariant（重要）和QReadWriteLock，实际上本项目可以只依赖于C++标准库实现