-- 暴露给C++的一些lua配置
package.path = package.path..";../service/?.lua"
require("Common/NetConfig")