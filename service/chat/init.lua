package.path = package.path..";../service/?.lua"
require("Common")

local serviceId
local conns = {}

function OnInit(id)
    serviceId = id
    LOG_INFO("[lua] cahr OnInit id: " .. id)
    sunnet.Listen(8002, id)
end

function OnAcceptMsg(listenFd, clientFd)
    LOG_INFO("[lua] chat OnAcceptMsg " .. clientFd)

    conns[clientFd] = true
end

function OnSocketData(fd, buff)
    LOG_INFO("[lua] char OnSocketData " .. fd .. " " .. buff)
    for fd, _ in pairs(conns) do
        local ret = sunnet.Write(fd, buff)
    end
end

function OnSocketClose(fd)
    LOG_INFO("[lua] char OnSocketClose " .. fd)

    conns[fd] = nil
end










