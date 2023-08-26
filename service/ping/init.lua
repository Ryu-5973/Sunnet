package.path = package.path..";../service/?.lua"
require("Common")

local serviceId

function OnInit(id)
    LOG_INFO("[lua] ping OnInit id: " .. id)

    serviceId = id
end

function OnServiceMsg(source, buff)
    LOG_INFO("[lua] ping OnServiceMsg id: " .. serviceId)

    if string.len(buff) > 50 then
        sunnet.KillService(serviceId)
        return 
    end

    sunnet.Send(serviceId, source, buff .. "i")
end

function OnExit()
    LOG_INFO("[lua] print OnExit")
end
