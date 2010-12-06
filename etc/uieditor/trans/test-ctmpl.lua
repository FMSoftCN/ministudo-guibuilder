
---- create a object for cmpinst table

compinst = {
    getNextInstance = function(inst)
        if inst ~= nil then
            return inst.next
        end
    end,

    getInstChildren = function(inst)
        return inst.children
    end,

    getInstanceEvents = function (inst)
       return inst.events 
    end,

    getProperties = function (inst)
        return inst.properties
    end,

    getInstListens = function (inst)
        return inst.listens
    end,

    getCompTemplateData = function (inst)
        return inst.templtable
    end
}


local label1 = {
    id = "ID_INFO",
    name = "ID_INFO",
    serial = 2939293043003,
    properties = {
    },

    listens = {
        {
            listener = "ID_INFO",
            listenType = "mLabel",
            listenSerial = 2939293043003,
            sender = "ID_EDIT",
            sendType = "mEdit",
            sendSerial=388293992934,
            event = "onTextChanged",
            prototype = "info_edit_onTextChanged"
        }
    }
}

local button1 = {
    id = "ID_BUTTON1",
    name = "ID_BUTTON1",
    serial = 533023020300,
    next = label1
}


local panel_children = {
    id = "Panel::-1",
    name = "Panel_1",
    serial = 323032043234,
    properties = {
    },
    children = button1
}

local base_inst = {
   id = "ID_MAINWND1",
   name = "ID_MAINWND1",
   serial = 4203023004234,
   properties = {
       
   },
   events = {
       {
           name = "onCreate",
           funcName = "onCreate",
           prototype = "BOOL @(mWidget* self, LPARAM lParam)",
           cotent = "return TRUE;", 
           code = "MSG_CREATE" 
       },
       {
           name = "onMouseMove",
           funcName = "onMouseMove",
           prototype = "BOOL @(mWidget* self, int x, int y, DWORD flags)",
           content = "return NCSR_CONTINUE_MSG;",
           code = "MSG_MOUSEMOVE"
       },
       {
           name = "onPaint",
           funcName = "onPaint",
           prototype = "void @(mWidget* self, HDC hdc)",
           code = "MSG_PAINT"
       }
   },
   listens = {
       {
           listener = "ID_MAINWND1",
           listenType = "mMainWnd",
           listenSerial = 4203023004234,
           sender = "ID_BUTTON1",
           sendType = "mButton",
           sendSerial = 533023020300,
           event = "NCSN_WIDGET_CLICK",
           prototype = "mainwnd1_button1_onClick"
       }
   },

   children = panel_children
}

--dofile("mainframe.c.tmpl-obj.lua")
--test_inst = ID_MAINFRAME

if test_inst == nil then
    test_inst = base_inst
end

function dump_comm_table(prefix, t)
    for k,v in pairs(t) do 
        print(string.format("%s%s = %s\n", prefix, tostring(k), tostring(v)))
        if type(v) == "table" then
            dump_comm_table(prefix .. "\t", v)
        end
    end
end

dofile("ncs-ctmpl.lua")

---dump_comm_table("", test_inst)

trans_main("ctmpl-test.c", test_inst)

dump_comm_table("", event_handlers)


