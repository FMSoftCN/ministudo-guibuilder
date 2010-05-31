--- lua tranlsate New Control Set C Source
--

-- global variable
event_handlers = {}
varname = ""
func_declear = ""
func_return = ""
func_name = ""
func_content = ""
main_handlers_name = ""
main_handlers_id = "-1"
main_serial = 0
event_name = ""
out = nil
inst_serial = 0
is_start_wnd = false

funcs_decleared={}

local function dump_handlers(handlers)
	if handlers then
		print (string.format("instance id : %s\n", handlers.id))
		print (string.format("instance name : %s\n", handlers.name))
		print (string.format("event count : %d\n", handlers.propCount))
		
		local i = 0
		while i < handlers.propCount do
			print (string.format("\tevent name : %s\n", handlers[i].name))
			print (string.format("event funcName : %s\n", handlers.funcName))
			print (string.format("\tevent prototype : %s\n", handlers[i].prototype))
			print (string.format("\tevent code : %s\n", handlers[i].code))
			i = i + 1
		end

	end
end

local function out_function(prefix)
	if funcs_decleared[func_name] == true then
		return
	end

	local strformat = [==[

//$func %s%u %s -- Need by merge, don't modify
static %s 
{
	//TODO:
	%s
}

]==]
	
	local x = string.format(strformat, prefix, inst_serial, event_name, func_declear, func_content)
	-- print(x)
	out:write(x)

	funcs_decleared[func_name] = true
end

local function get_inst_name(handler_name)
	local x = string.gsub(handler_name, "ID_(%w+)", "%1")
	return string.gsub(x, "(%w+)_?", function (w) 
							return string.upper(string.sub(w, 0, 1)) .. string.lower(string.sub(w, 2))
							end)
end


function trans_inst(inst)
	local handlers = compinst:getInstanceEvents(inst)

	if handlers == nil then
		return
	end

	 --print(out) 
	 --
	-- change id name , e.g ID_MAIN_TEST => MainTest
	varname = get_inst_name(handlers.name)
	-- get serial number of instance
	inst_serial = handlers.serial
	if main_serial == 0 then main_serial = inst_serial end
	if main_handlers_name == "" then
		main_handlers_name = varname
		main_handlers_id = handlers.id
	end

	--print ("varname:" .. varname)
	--for each event, save delcears
	local i = 0
	while i < handlers.propCount do
		func_name = varname .. "_" .. handlers[i].funcName
		event_name = handlers[i].name
		-- get declear of function
		func_declear = string.gsub(handlers[i].prototype, "@", func_name)
		--get the return type of function
		func_return = string.gsub(handlers[i].prototype, "(%w+%s*%*?)%s*@%s*%(.*%)", "%1")
		func_return = string.gsub(func_return, "%s", "")
		func_content = handlers[i].content
		
		if not func_return then
			func_return = "int"
		end

		--print ("func_name:" .. func_name)
		--print ("func_declear:" .. func_declear)
		--print ("func_return:" .. func_return )
		--print(out_function)

		out_function("@")

		--save handlers info

		i = i + 1
	end

	--- save handlers
	if handlers.propCount > 0 then
		local strfromat = [==[
//$handle @%u -- Need by merge, don't modify
static NCS_EVENT_HANDLER %s_handlers [] = {
]==]

		out:write(string.format(strfromat, inst_serial, varname))
		i = 0
		while i < handlers.propCount do
			func_name = varname .. "_" .. handlers[i].funcName
			out:write(string.format("\t{%s, %s},\n",handlers[i].code , func_name ))
			
			i = i + 1
		end
		--out null
		out:write("\n//$user --TODO: Add your handlers here\n\t{-1, NULL}\n};\n\n")

		-- save the name of handler
		event_handlers[handlers.id] = varname .. "_handlers"
	end
	
	local child = compinst:getInstChildren(inst)
	while child ~= 0 do
		trans_inst(child)
		child = compinst:getNextInstance(child)
	end

end

-- out the event listen
function trans_event_listens(inst)
	if inst == nil then
		return 0
	end

	local listens = compinst:getInstListens(inst)

	-- print("listens " .. tostring(listens))

	if listens == nil then return 0 end

	local count = 0

	local funcFormat=[[
//$func #%u %s_%u_%u -- Need by merge, don't modify
static BOOL %s (m%s *self, m%s* sender, int id, DWORD param)
{
	//TODO:

	return TRUE; /* allow the event to go next */
}

]]
	for k,v in pairs(listens) do
		--parse event
		-- print(string.format("listen=%s, sender=%s, event=%s, prototype=%s\n",v.listener, v.sender, v.event, v.prototype))
		local event = v.event:match("%(([%w%d_]+)%)")
		if event ~= nil then v.event = event; end
		if funcs_decleared[v.prototype] ~= true then
			--gen functions
			out:write(string.format(funcFormat, main_serial,v.event, v.sendSerial, v.listenSerial,
					v.prototype, v.listenType, v.sendType))
			count = count + 1
			funcs_decleared[v.prototype] = true
		end
	end

	-- out the connections	
	local connectFormat=[[
//$connect #%u -- Need by merge, don't modify
static NCS_EVENT_CONNECT_INFO %s_connects[] = {
]]
	if count > 0 then
		out:write(string.format(connectFormat, main_serial, main_handlers_name))

		for k, v in pairs(listens) do
			out:write(string.format("\t{%s, %s, %s, (NCS_CB_ONOBJEVENT)%s},\n", v.sender, v.listener, v.event, v.prototype))
		end
		out:write("\n//$user --TODO: Add your handlers here\n\t{-1, -1, 0, NULL }\n};\n\n")
	end

	return count;

end

-- main function
function trans_main(file, args, inst)

	-- print(file)
	-- print(args)
	--
	
	if string.find(args, "%-start%-wnd") then
		is_start_wnd = true
	end

	-- print(args)
	-- print(is_start_wnd)

	out = assert(io.open(file, "wt"));

	out:write([==[
/**************************************************************
*  This file is generated automatically, don't modify it.
*  Feynman software Technology Co.,Ltd
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
);

	local first_inst = inst;
	while inst ~= 0 do
		trans_inst(inst)
		inst = compinst:getNextInstance(inst)	
	end

	local connects = trans_event_listens(first_inst)

	-- save all handlers
	local strformat = [==[
//$mainhandle -- Need by merge, don't modify
static NCS_EVENT_HANDLER_INFO mainwnd_%s_handlers[] = {
]==]
	out:write(string.format(strformat, main_handlers_name))
	for k,v in pairs(event_handlers) do
		out:write(string.format("\t{%s, %s},\n", k, v))
	end

	out:write("\n//$user --TODO: Add your handlers here\n\t{-1, NULL}\n};\n");	

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
	
	local connect_handler_name = "NULL"

	if connects > 0 then
		connect_handler_name = main_handlers_name .. "_connects"
	end
	
	-- print(str_main_entry_format)
	out:write(string.format(str_main_entry_format, main_handlers_name, main_handlers_id, main_handlers_name, connect_handler_name))
		

	out:close();

end

