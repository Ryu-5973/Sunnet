package.path = package.path..";../service/?.lua"
require("Common")

local serviceId
local conns = {}

function OnInit(id)
    serviceId = id
    LOG_INFO("[lua] chat OnInit id: " .. id)
    sunnet.Listen(8002, id, EnumNetProtoType.TCP)
    sunnet.Listen(8002, id, EnumNetProtoType.UDP)
end

function OnAcceptMsg(listenFd, clientFd)
    LOG_INFO("[lua] chat OnAcceptMsg " .. clientFd)
    conns[clientFd] = true
end

function OnSocketData(fd, buff)
    LOG_INFO("[lua] chat OnSocketData " .. fd .. " " .. buff)
    Boardcast(buff)
end

function OnSocketClose(fd)
    LOG_INFO("[lua] chat OnSocketClose " .. fd)
    conns[fd] = nil
end

function OnUDPMsg(msg)
    LOG_INFO("[lua] OnUDPMsg " .. msg)
    Boardcast(msg)
end

function Boardcast(msg)
    for fd, _ in pairs(conns) do
        local ret = sunnet.Write(fd, msg)
    end
end










