package.path = package.path..";../service/?.lua"
require("Common")

function OnInit(id)
    LOG_INFO("[lua] main OnInit id: " .. id)

    sunnet.NewService("chat")

end

function OnExit()
    LOG_INFO("[lua] main OnExit")
end
