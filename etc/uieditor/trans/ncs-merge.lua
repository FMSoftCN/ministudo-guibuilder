------------------------------------------------------
-- common functions

funcs_decleared={}
connect_handler=nil
main_handlers_seted=false

local function get_func_event_id(line)
	for id, event in line:gmatch("//%$func%s+([#@!%$%%&%*][%d%a_]+)%s+([%a%d_]+)") do
		return id, event
	end
	return nil,nil
end

local function get_handle_entry_name(entry)
	return entry:match("([a-zA-Z0-9_%+-]+)%s*%)?%s*,")
end

local function get_connect_entry_name(entry)
	local  name = ""
	for w in entry:gmatch("([a-zA-Z0-9_%+-]+)%s*,") do
		name = name .. w
	end
	return name;
end


--
----------------------------------------------------

----------------------------------------------
-- test file .c
-- read a event handler chunk
-- a chunk include a serial function and a 
-- NCS_EVENT_HANDLER array
--
-- e.g (c file)
-- .....................................................
-- //$func [serialnumber] [event-name]
-- static [return-type] [ctrlname]_[event-name]([param lists])
-- {
-- ...
-- }
--
-- //$func [2030304] [onClicked]
-- static void button1_onClicked(...)
-- {
-- }
-- 
-- ...
-- //$handle [2030304]
-- static NCS_EVENT_HANDLER [name] [] = {
-- ...
-- }
--.....................................................
--
--chunk = { id=2030304
-- name=[event-name]
-- 0 = [function]
-- 1 = [function]
-- ...
-- }
--
-- function = {
-- id="2030304"
-- event="onClicked"
-- name="Button1_onClicked"
-- prototype = "static void Button1_onClieck ... "
-- }
--
---------------------------------------------

function parse_func(f,line)
	local func={seted=false}
	-- get id, event first
	
	func.id, func.event = get_func_event_id(line)

	if func.id == nil or func.event == nil then return nil end

	-- print("-- func id=" .. func.id .. "  event=" .. func.event)

	-- get property
	local line = f:read("*l")
	if line == nil then
		return nil
	end

	local property = ""
	local p = string.match(line,".*%)%s*$")
	while p == nil do
		property = property .. line
		line = f:read("*l")
		if line == nil then
			break
		end
		p = line:match(".*%)%*$")
	end

	if p == nil then
		return nil
	end

	property = property .. p

	if property == nil or property:len() <= 0 then
		return nil
	end

	-- print("-- func property = " .. property )

	-- get function name from property
	local name = property:match("([%a%d_]+)%s*%(.*%)")

	if name == nil or name:len() <= 0 then
		return nil
	end

	-- print("-- func name = " .. name)

	func.name = name
	func.property = property

	-- get the function content
	-- get '{'
	local content ;
	while line and line:match("{")==nil do line = f:read("*l") end
	if line == nil then return func end
	content = line:match("{%s*(.*)\n")
	if content == nil then
		content = ""
	else
		content = "\t" .. content
	end
	local backet_count = 1 -- '{' count
	line = f:read("*l")
	while line  do
		-- print("--- content : " .. line)
		if line:match("{") then 
			backet_count = backet_count + 1 
		elseif line:match("}") then
			if backet_count <= 1 then
				break
			end
			backet_count = backet_count - 1
		end
		content = content .. "\n" .. line
		line = f:read("*l")
	end

	func.content = content
	

	return func
end

function parse_handler_entry(f, line, get_entry_name)
	local entries = {}
	-- get '{' , ready read entrys
	while line ~= nil and line:match("{") == nil do line = f:read("*l") end

	if line == nil then return end
	
	p = line:match("{%s*(.+)")
	if p ~= nil then
		line = p
	else
		line = f:read("*l")
	end

	while line ~= nil do
		if line:match("};") ~= nil or line:match("-1%s*,") or line:match("//%$user") then
			break
		end
		local entry_name = get_entry_name(line)
		--print(entry_name)
		if entry_name ~= nil and entry_name:match("^%s*$") == nil then
			entries[entry_name] = line
			--print("-- handler (" .. entry_name .. ")" .. " = " .. line)
		end
		line = f:read("*l")
	end
	return entries
end

function parse_handler(f, line, get_id, name_match, get_entry_name)
	local handler = {seted=false}
	-- get id of handler
	local id =  get_id(line) --line:match("//%$handle%s+([%a%d]+)")
	if id == nil then
		return nil
	end

	handler.id = id

	-- print("-- hanlder id = " .. id)

	-- get name of handler
	local p
	repeat 
		line = f:read("*l")
		if line == nil then
			return nil
		end
		-- print("--line :" .. line)
		-- print("-- name_match :" .. name_match)
		p = line:match(name_match) --"NCS_EVENT_HANDLER%s+([a-zA-Z0-9_]+)%s*%[.*%]")
	until p ~= nil

	if p == nil then
		return nil
	end
	handler.name = p

	-- print ("-- hanlder name = " .. p)

	handler.entries = parse_handler_entry(f, line, get_entry_name)
	
	return handler
end


function parse_mainwnd_entry(f, line)
	local mainwnd_entry={seted = false}
	
	line = f:read("*l")
	while line:match("NCS_WND_EXPORT") == nil do line = f:read("*l") end
	
	if line == nil then
		return nil
	end

	local content= line .. "\n";
	line = f:read("*l")
	while line do
		content = content .. line .. "\n";
		if line:match("}%s*$") then
			break;
		end
		line = f:read("*l")
	end
	mainwnd_entry.content = content;

	return mainwnd_entry;

end

function read_templates(filename)
	local lists = {}
	local f = io.open(filename, "r")
	local line = f:read("*l")
	local t

	while line ~= nil do
		if line:match("//%$func") then
			t = parse_func(f, line)
			if t ~= nil then
				local handle = lists[t.id]
				local funcs 
				if handle == nil then
					funcs = { }
					lists[t.id] = { id=t.id, funcs=funcs }
				else
					funcs = handle.funcs
				end
				funcs[t.event] = t
			end
		elseif line:match("//%$handle") then
			t = parse_handler(f, line, 
					function (line) return line:match("//%$handle%s+(@[%a%d]+)") end,
					"NCS_EVENT_HANDLER%s+([a-zA-Z0-9_]+)%s*%[.*%]",
					get_handle_entry_name)
			if t ~= nil then
				local handler = lists[t.id]
				if handler == nil then
					lists[t.id] = t
				else
					-- copy
					for k,v in pairs(t) do handler[k] = v end
				end
			end
		elseif line:match("//%$connect") then
			local ct = parse_handler(f, line, 
				function(line) return line:match("//%$connect%s+(#[%a%d]+)") end,
				"NCS_EVENT_CONNECT_INFO%s+([a-zA-Z0-9_]+)%s*%[.*%]",
				get_connect_entry_name)
			if ct ~= nil then
				local connect = lists[ct.id]
				if connect == nil then
					lists[ct.id] = ct
				else
					-- copy
					for k, v in pairs(ct) do connect[k] = v end
				end
			end
		elseif line:match("//%$mainhandle") then
			lists._main_ = parse_handler(f, line,
				function (line) return "_main_" end,
				"NCS_EVENT_HANDLER_INFO%s+([a-zA-Z0-9_]+)%s*%[.*%]",
				get_handle_entry_name)

		elseif line:match("//%$mainwnd_entry") then
			lists.mainwnd_entry = parse_mainwnd_entry(f, line)		
		end
		line = f:read("*l")
	end
	f:close()
	return lists
end


function dump_table(t, prefix)
	if t == nil then
		print( prefix .. "nil")
		return
	end

	for k,v in pairs(t) do
		if type(v) == "table" then
			print(prefix .. k .. " : ")
			dump_table(v, prefix .. "\t")
		else
			print(prefix .. k .. " : " .. tostring(v))
		end
	end
end

function  dump_template(lists)
	dump_table(lists, "")
end

--lists = read_templates("test1.c")
--dump_template(lists)


-------------------------------------------------------------------------
-- merge the file

function merge_func(fdst, fout, line, templs)


	fout:write(line .. "\n")

	-- get id and event
	local id, event = get_func_event_id(line)
	if id == nil or event == nil then
		return
	end

	local more = line:match("{%s*(.*)")

	-- get handle
	local handle = templs[id]
	if handle == nil or handle.funcs == nil then return end
	-- get func
	local func = handle.funcs[event]
	if func == nil or func.seted  then return end


	if funcs_decleared[func.name] == true then
		return
	end

	-- out put the function property
	fout:write(func.property .. "\n")
	fout:write("{\n")
	if more ~= nil then
		fout:write(more .. "\n")
	else
		-- skip the "{"
		repeat
			line = fdst:read("*l")
		until line and line:match("{") ~= nil
	end

	func.seted = true
	funcs_decleared[func.name] = true
end

function merge_handle_entry(fdst, fout, handle, get_entry_name)
	-- get begin of handle entry
	local line = fdst:read("*l")
	while line and line:match("{") == nil do line = fdst:read("*l") end
	if line == nil then return end
	-- local more = line:match("{%s*(.*)\n")
	-- if more ~= nil then
	--	line = more
	--else
	--	line = fdst:read("*l")
	--end

	local entries = handle.entries
	if entries == nil then
		fout:write(line .. "\n")
		return
	end
	
	--local seted_entries = {}
	--while line and line:match("};") == nil and line:match("-1%s*,") == nil and line:match("//%$user") ==nil do
		-- print("++ line:" .. line)
		--if line:match("^%s*//") then
		--	fout:write(line .. "\n")
		--else
		--	local entry_name = get_entry_name(line)
		--	if entry_name == nil or entries[entry_name] == nil then
		--		fout:write(line .. "\n")
		--	else
		--		fout:write(entries[entry_name] .. "\n")
		--		seted_entries[entry_name] = true
		--	end
		-- end
		
	--	line = fdst:read("*l")
	--end
	-- out all the entries
	for k, h in pairs(entries) do
		if k ~= "-1" and k ~= "0" then
			fout:write(h .. "\n")
		end
	end

	-- continue the user data
	while line do
		if line:match("};") or line:match("-1%s*,") or line:match("//%$user") then
			break
		end
		line = fdst:read("*l")
	end

	if line and line:match("//%$user") == nil then
		fout:write("\t//$user -- TODO: add your handlers here\n");
	end

	while line:match("};") == nil do
		fout:write(line .. "\n")
		line = fdst:read("*l")
	end

	fout:write(line .. "\n")
	
	--if line then
		-- out the new insert entries
	--	for k,h in pairs(entries) do
			-- print("+++ k=" .. k)
	--		if seted_entries[k] == nil and  k:match("-1") == nil and k ~= "0" then
				-- out put the endtry
	--			fout:write(h .. "\n")
	--		end
	--	end
	--	fout:write(line .. "\n")
	--end

end

function merge_handle(fdst, fout, line, templs, get_id, head_format, get_entry_name)
	-- get handle id
	local id = get_id(line) --line:match("//%$handle%s+([%a%d]+)")
	if id == nil then 
		fout:write(line .. "\n")
		return 
	end

	-- get handle
	local handle = templs[id]
	if handle == nil then 
		fout:write(line .. "\n")
		return 
	end
    
    if handle.id:match("#[%w%d_]+") then -- is connect
        connect_handler = handle
    end
	
	-- get all then functions new inserted
	local funcs = handle.funcs
	if funcs ~= nil then
		for k, f in pairs(funcs) do
			--print("+++ handle " .. f.name)
			--print(f.seted)
			-- print functions
			if f.seted == false  and funcs_decleared[f.name] ~= true then
				fout:write(string.format("//$func %s %s --Need by merge, don't modify\n", f.id, f.event))
				fout:write(f.property .. "\n")
				fout:write("{\n" .. f.content .. "\n}\n\n")
				funcs_decleared[f.name] = true
			end
		end
	end

	fout:write(line .. "\n")
	
	-- print the new name of handle
	-- print("---",handle.name)
	fout:write(string.format(head_format, handle.name))
	--"static NCS_EVENT_HANDLER %s [] = {\n", handle.name))
	
	-- merge entries
	merge_handle_entry(fdst, fout, handle, get_entry_name)

	handle.seted = true
end

function merge_main_handle(fdst, fout, templs, line)
	local handle = templs._main_	
	if handle == nil then return end
	
	for k, h in pairs(templs) do
		--print ("++++ h=" .. k)
        ---- out put the new handlers
		if h ~= handle and h.seted == false and h.funcs then
			-- out the functions
			for m, f in pairs(h.funcs) do
				if funcs_decleared[f.name] ~= true then
					fout:write(string.format("//$func %s %s -- Need by merge, don't modify\n", f.id, f.event))
					fout:write(f.property .. "\n")
					fout:write("{\n" .. f.content .. "\n}\n\n")
					funcs_decleared[f.name] = true
				end
			end
			-- out the handler
			-- is connect
            --- we should not out the connect known, because the conects may be write after the main handler
			if h.id:match("#[%w%d_]+") then
            --[[
				fout:write(string.format("//$connect %s -- Need by merge, don't modify\n", h.id))
				fout:write(string.format("static NCS_EVENT_CONNECT_INFO %s[] = {\n", h.name))
				local entries = h.entries
				for n, e in pairs(entries) do
					if n ~="-1-10" and n ~="0" then
						fout:write(e .. "\n")
					end
				end
				fout:write("//$user -- TODO add your handlers here\n\t{-1, -1, 0, NULL}\n};\n\n");
            ]]
                connect_handler = h -- record this info
			else -- event handler
				fout:write(string.format("//$handle %s -- Need by merge, don't modify\n", h.id))
				fout:write(string.format("static NCS_EVENT_HANDLER %s [] = {\n" , h.name));
				local entries = h.entries
				for n, e in pairs(entries) do
					if n ~= "-1" and n ~= "0" then
						fout:write(e .. "\n")
					end
				end
				fout:write("\t//$user -- TODO add your handlers here\n\t{-1,NULL}\n};\n\n");
			end
		end
	end
	
	-- print the new name of handle
	fout:write(line .. "\n")
	fout:write(string.format("NCS_EVENT_HANDLER_INFO %s [] = {\n" , handle.name))

	-- merget entries
	merge_handle_entry(fdst, fout, handle, get_handle_entry_name)
    main_handlers_seted = true
end

function merge_mainwnd_entry(fdst, fout, templs)
	local mainwnd_entry = templs.mainwnd_entry
	-- print("+++ " .. tostring(mainwnd_entry))
	if mainwnd_entry == nil then return end

    if not main_handlers_seted then 
        local handle = templs._main_
        fout:write("\n//$mainhandle -- Need by merge, don't modify\n")
        fout:write(string.format("NCS_EVENT_HANDLER_INFO %s[] = {\n", handle.name))
        for n, e in pairs(handle.entries) do
            if n ~= "-1" and n ~= "0" then
                fout:write(e .. "\n")
            end
        end
        fout:write("\t//$user -- TODO add your handlers here\n\t{-1, NULL}\n};\n")
    end

    if connect_handler ~= nil and not connect_handler.seted then --out the connect
        local h = connect_handler
        fout:write(string.format("//$connect %s -- Need by merge, don't modify\n", h.id))
        fout:write(string.format("static NCS_EVENT_CONNECT_INFO %s[] = {\n", h.name))
        for n, e in pairs(h.entries) do
            if n ~="-1-10" and n ~="0" then
                fout:write(e .. "\n")
            end
        end
        fout:write("\t//$user -- TODO add your handlers here \n\t{-1, -1, 0, NULL}\n};\n\n")
    end
	
	-- print info
	fout:write("//$mainwnd_entry -- don't modify\n")
	
	-- paint content
	fout:write(mainwnd_entry.content)

	-- skip the fdst
	local line = fdst:read("*l")
	while line and line:match("}%s*$") == nil do line = fdst:read("*l") end
	
	mainwnd_entry.seted = true
	
end

function skip_disable(fdst, fout)
	local line = fdst:read("*l")
	--print("skip---- " .. line)
	fout:write(line .. "\n")
	
	while line ~= nil and line:match("^[ \t\r]*$") do
		line = fdst:read("*l")
		fout:write(line .. "\n")
		--print("skip---- " .. line)
	end
	
	if line == nil then
		return
	end

	if not line:match("#if[ \t]+0") then
		return
	end 
	
	local endif_match = 1	
	line = fdst:read("*l")
	fout:write(line .. "\n")
	--print("skip---- " .. line)
	while line ~= nil and endif_match > 0 do
		if line:match("#if") or line:match("#ifdef") or	line:match("#ifndef") then
			endif_match = endif_match + 1 
		elseif line:match("#endif") then
			endif_match = endif_match - 1
		end
		if endif_match > 0 then
			line = fdst:read("*l")
			fout:write(line .. "\n")
			--print("skip---- " .. line)
		end
	end
end

function merge(destfile, srcfile, outfile)
	--print(destfile)
	--print(srcfile)
	--print(outfile)
	local fdst = io.open(destfile,"r")
	local fout = io.open(outfile, "w")
	local templs = read_templates(srcfile)
	--dump_template(templs)

	--skip #if 0 ... #endif
	skip_disable(fdst, fout)

	local line = fdst:read("*l")

    if (line ~= nil) then

        while line ~= nil  do
            if line:match("//%$func") then
                merge_func(fdst, fout, line, templs)
            elseif line:match("//%$handle") then
                merge_handle(fdst, fout, line, templs,
                    function (line) return line:match("//%$handle%s+(@[%a%d]+)") end, -- get_id
                    "static NCS_EVENT_HANDLER %s [] = {\n",
                    get_handle_entry_name)
            elseif line:match("//%$connect") then
                merge_handle(fdst,fout, line, templs, 
                    function (line) return line:match("//%$connect%s+(#[%a%d]+)") end, -- get_id
                    "static NCS_EVENT_CONNECT_INFO %s [] = {\n",
                    get_connect_entry_name)
            elseif line:match("//%$mainhandle") then
                merge_main_handle(fdst, fout, templs, line)
            elseif line:match("//%$mainwnd_entry") then
                -- print("+++ mainwnd entry")
                merge_mainwnd_entry(fdst, fout, templs)
            else
                fout:write(line .. "\n")
            end
            line = fdst:read("*l")

        end

        if templs ~= nil and templs.mainwnd_entry ~= nil and not templs.mainwnd_entry.seted then
            merge_mainwnd_entry(fdst, fout, templs)
        end

    else
        local f = io.open(srcfile, "r");
        local newline = f:read("*l");
        while(newline ~= nil) do
            fout:write(newline .. "\n");
            newline = f:read("*l");
        end
        f:close();
    end
	fdst:close()
	fout:close()
end

-- testing

--merge("test.c", "test.c.tmpl", "test1.c.tmp")

