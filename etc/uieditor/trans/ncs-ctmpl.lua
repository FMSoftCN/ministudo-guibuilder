--- lua tranlsate New Control Set C Source
--
------------------------------------------------------------------------------
-- dubeg  dump inst
local function dump_table(debug_out, prefix, t)
    if t == nil then return end
    for k, v in pairs(t) do 
        debug_out:write(string.format("%s%s:\t%s\n",prefix, tostring(k) , tostring(v))) 
        if type(v) == "table" then
            dump_table(debug_out,prefix .. "\t", v)
        end
    end
end

local function dump_inst(debug_out, prefix, inst)
    dump_table(debug_out, prefix, inst)
    
    -- get events
    debug_out:write(string.format("%s------- dump events ----\n",prefix))
    dump_table(debug_out, prefix .. "\t", compinst.getInstanceEvents(inst))
    debug_out:write(string.format("%s------- dump connects ---\n", prefix))
    dump_table(debug_out, prefix .. "\t", compinst.getInstListens(inst))
    debug_out:write(string.format("%s------- dump children ---\n", prefix))
    local child = compinst.getInstChildren(inst)
    while child do
        dump_inst(debug_out, prefix .. "\t", child)
        child = compinst.getNextInstance(child)
    end
end

local function dump_insts(file, inst)
    local debug_out = io.open(file .. ".debug", "wt")

    dump_inst(debug_out, "", inst)

    debug_out:close()
end

local function save_table(prefix,out, t)
    if t == nil then return end
    for k,v in pairs(t) do
        if type(k) == "string" then
            out:write(string.format("%s%s=",prefix, tostring(k)));
        else
            out:write(prefix)
        end
        if type(v) == "string" then out:write(string.format("\"%s\",\n", tostring(v)))
        elseif type(v) == "integer" then out:write(string.format("%d,\n", tostring(v)))
        elseif type(v) == "table" then 
            out:write("{")
            save_table(prefix .. "\t", out, v)
            out:write(string.format("%s},\n",prefix))
        else 
            out:write(string.format("%s,\n",tostring(v))) 
        end
    end
end

function save_next(out, inst)
   local next_ist = compinst.getNextInstance(inst)
   if next_ist ~= nil then   save_next(out, next_ist) end
   save_inst(out, inst)
end

function save_inst(out, inst)

    local child = compinst.getInstChildren(inst)
    if child ~= nil then
        save_next(out, child)
    end

    out:write(string.format("%s = {\n\tid=\"%s\",\n\tname=\"%s\",\n\tserial=%u,\n",inst.name, inst.id, inst.name, inst.serial));
    out:write("\tevents= { \n")
    save_table("\t\t",out, compinst.getInstanceEvents(inst));
    out:write("\t},\n")
    out:write("\tlistens = {\n")
    save_table("\t\t",out, compinst.getInstListens(inst));
    out:write("\t}, \n")

    child = compinst.getInstChildren(inst)
    if child ~= nil then
        out:write(string.format("\tchildren = %s,", child.name))
    end
    local nxt = compinst.getNextInstance(inst)
    if nxt ~= nil then
        out:write(string.format("\tnext = %s", nxt.name))
    end
    out:write("\n}\n")
end

local function save_insts(file, inst)
    local out = io.open(file .. "-obj.lua", "wt")
    save_inst(out, inst)
    out:close(out)
end


--------------------------------------------------------------------------------
-- global variables
event_handlers={}
connect_handers={}
funcs_decleared={}
main_serial = 0


-----------------------------------------------------------------------------------------------

local function get_inst_name(handler_name)
	local x = string.gsub(handler_name, "ID_(%w+)", "%1")
	return string.gsub(x, "(%w+)_?", function (w) 
							return string.upper(string.sub(w, 0, 1)) .. string.lower(string.sub(w, 2))
							end)
end

local function_format = [==[

//$func %s%u %s -- Need by merge, don't modify
static %s 
{
	//TODO:
	%s
}

]==]

----------------------------------------------------------------------------
-- gen the functions
function gen_instance_events(out, inst)
    local handlers = compinst.getInstanceEvents(inst)

    if handlers == nil  or #handlers <= 0 then
        return nil
    end
    
    local event_list = {}
   
    -- get name of events
    local varname = get_inst_name(inst.name)
    
    for _, handler in pairs(handlers) do
        --- funcName, e.g  onCreate, onPaint,
        local func_name = varname .. "_" .. handler.funcName
        --- func_name would be mainwnd1_onCreate, e.g
        local event_name = handler.name
        -- get declar of function  eg.  BOOL @(mWidget* self, DWORD dwAdd) , replace '@'
        local func_declear = string.gsub(handler.prototype, "@", func_name)
        -- get the return of function
        --local func_return = string.gsub(handlers[i].prototype, "(%w+%s*%*?)%s*@%s(.*%s)", "%1")
        --func_return = string.gsub(func_return, "%s", "")
        
        --if func_return == nil then func_return = "int" end

        local func_content = handler.content
        if func_content == nil then func_content = "" end

        -- out function
        out:write(string.format(function_format, "@", inst.serial, event_name, func_declear, func_content))
        
        -- apend function name and function code into event list 
        event_list[handler.code] = func_name

    end
    
    --- out put the handler array
    local strformat=[==[
//$handle @%u -- Need by merge, don't modify
static NCS_EVENT_HANDLER %s_handlers [] = {
]==]
    out:write(string.format(strformat, inst.serial, varname))

    for code,func in pairs(event_list) do
        out:write(string.format("\t{%s,%s},\n", code, func))
    end
    -- out null
    out:write("\n\t//$user --TODO: Add your handlers here\n\t{-1, NULL}\n};\n")
        
    event_handlers[inst.serial] = {varname, inst.name }
end

local function gen_instance_connects(out, inst)
    if inst == nil then return end

    local listens = compinst.getInstListens(inst)

    if listens == nil then return end

	local funcFormat=[[
//$func #%u %s_%u_%u -- Need by merge, don't modify
static BOOL %s (m%s *self, m%s* sender, int id, DWORD param)
{
	//TODO:

	return TRUE; /* allow the event to go next */
}

]]

    for k, v in pairs(listens) do
        local event = v.event:match("%(([%w%d_]+)%)")
        if event ~= nil then v.event = event end
        if not funcs_decleared[v.prototype] then
            -- gen functions
            out:write(string.format(funcFormat, main_serial, v.event, v.sendSerial, v.listenSerial, 
                v.prototype, v.listenType, v.sendType))
            --- mind the prototype, to avoid the redeclear the C function
            funcs_decleared[v.prototype] = true
        end
        --print("---- insert : event, send, listners:", v.event, v.sender, v.listener)
        table.insert(connect_handers, v)
    end
    
end

local function gen_instance(out, inst)
    gen_instance_events(out, inst)
    gen_instance_connects(out, inst)
    
    --gen children

    local child = compinst.getInstChildren(inst)

    while child ~= nil do
        gen_instance(out, child)
        child = compinst.getNextInstance(child)
    end
end


----------------------------------------------------------------------------
--main function
function trans_main(file, inst)

    if inst == nil then return end

    --dump_insts(file, inst)
    --save_insts(file, inst)

    local out = assert(io.open(file, "wt"));

    if out == nil then return end

    main_serial = inst.serial

    out:write([==[
/**************************************************************
*  This file is generated automatically, don't modify it.
*  Beijing FMSoft Technologies Co., Ltd
*
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#include <mgncs/mgncs.h>

#include "resource.h"
#include "ncs-windows.h"


]==]
)
    local handlers = gen_instance(out, inst)
    local main_var_name = get_inst_name(inst.name)

    -- save all handlers
    local strformat = [==[
//$mainhandle -- Need by merge, don't modify
static NCS_EVENT_HANDLER_INFO mainwnd_%s_handlers[] = {
]==]
    out:write(string.format(strformat, main_var_name))

    for serial, handler in pairs(event_handlers) do
        out:write(string.format("\t{%d, %s_handlers}, /*%s*/\n", serial, handler[1], handler[2]))
    end
    -- save end
    out:write("\n\t//$user --TODO Add your handlers here\n\t{ -1, NULL } \n};\n")

    -- out the connections
    local have_connects = #connect_handers > 0

    if have_connects then
        out:write(string.format([==[
//$connect #%u -- Need by merge, don't modify
static NCS_EVENT_CONNECT_INFO %s_connects[] = {
]==], inst.serial, main_var_name))
        for k, v in pairs(connect_handers) do
            --print("----",k,v,v.sendSerial, v.listenSerial, v.event, v.prototype, v.sender, v.listene )
            local connect_str =string.format("\t{%d, %d, %s, (NCS_CB_ONOBJEVENT)%s},/* %s - %s */ \n",
                v.sendSerial, v.listenSerial, v.event, v.prototype, v.sender, v.listener)
            --print(connect_str)
            out:write(connect_str)
        end
        out:write("\n\t//$user --TODO: Add your handlers here\n\t{-1, -1, 0, NULL } \n};\n\n")
    end

	-- out the main entry
	local str_main_entry_format=[[

//$mainwnd_entry -- don't modify this function
NCS_WND_EXPORT mMainWnd* ntCreate%sEx(HPACKAGE package, HWND hParent, HICON h_icon, HMENU h_menu, DWORD user_data)
{
	// please don't change me
	return ncsCreateMainWindowIndirectFromID(package,
		%s, /* window id */
		hParent,
		h_icon,
		h_menu,
		mainwnd_%s_handlers,
		%s,
		user_data);
}

]]

    local connects_name = have_connects and main_var_name .. "_connects" or "NULL"

    out:write(string.format(str_main_entry_format, main_var_name, inst.id, main_var_name, connects_name))

    out:close()

end
