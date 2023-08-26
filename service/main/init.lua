package.path = package.path..";../service/?.lua"
require("Common")

LOG_INFO("run lua init.lua")

function OnInit(id)
    LOG_INFO("[lua] main OnInit id: " .. id)
    Test()

    sunnet.NewService("chat")

end

function OnExit()
    LOG_INFO("[lua] main OnExit")
end

function Test()
    local tbl = {1, true, "2131你的还", 0.2313, 'c'}
    local ud = msgpck.pack(tbl)
    print(ud)
    local newTbl = msgpck.unpack(ud)
    print(newTbl)
    for k, v in pairs(newTbl) do
        print(k, v)
    end
end

