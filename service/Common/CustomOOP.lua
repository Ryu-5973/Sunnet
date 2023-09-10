local ClassMgr = {}
local __Cls_Id = 0

local function clone(object)
    local lookup_table = {}
    local function _copy(object)
        if type(object) ~= "table" then 
            return object 
        elseif lookup_table[object] then
            return lookup_table[object]
        end
        local new_table = {}
        lookup_table[object] = new_table
        for key, value in pairs(object) do
            new_table[_copy(key)] = _copy(value)
        end
        return setmetatable(new_table, getmetatable(object))
    end
    return _copy(object)
end

local function InitCommonMemberFunc(cls)
    function cls:Clone()
        return clone(cls)
    end

end

function Class(ClassName, ...)
    if not ClassName then
        return
    end
    local clsType = setmetatable({
        __custom_info__ = {
            __cls_id = __Cls_Id,
            __class_name = ClassName
        },
        GetClsId = function(self)
            return self.__custom_info__.__cls_id
        end,
        GetClsName = function(self)
            return self.__custom_info__.__class_name
        end,

    }, {
        __tostring = function(tbl)
            return "ClassType " .. tbl:GetClsName()
        end,
        __newindex = function(cls, key, value)
            if type(value) == "function" then
                rawset(cls, key, value)
                return
            end
            LOG_WARN("Can't Regist Class " .. cls:GetClsName() .. " Member \"" .. key .. "\" Directly")
        end,
    })
    __Cls_Id = __Cls_Id + 1

    rawset(clsType, "new", function(self)
        local cls = ClassMgr[self:GetClsId()]
        if not cls then
            print("Invalid Class Type")
            return
        end
        if clsType.Ctor then
            clsType:Ctor()
        end
        return cls
    end)

    local cls = setmetatable({}, {
        __name = "Class " .. ClassName,
    })
    InitCommonMemberFunc(cls)

    ClassMgr[clsType:GetClsId()] = cls

    return clsType
end

-- todo: 扩展下condition
function RegistClassMember(clsType, memberName, condition)
    clsType.memberName = true
end