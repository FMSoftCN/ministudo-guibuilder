
out = nil

varname = ""

function dump_tmpl(tmpl)
	print("---------------------------")
	print(string.format("Dialog Template : %s\n",tmpl.name))
	print(string.format("id = %s\n",tmpl.id))
	print(string.format("serial-number=%u\n",tmpl.serial))
	print(string.format("ctrolClass=%s\n",tmpl.ctrlClass))
	print(string.format("x=%d,y=%d,w=%d,h=%d\n",tmpl.x, tmpl.y, tmpl.w, tmpl.h))
	print(string.format("sytle=%u\n",tmpl.style))
	print(string.format("exstyle=%u\n",tmpl.exstyle))
	print(string.format("caption=%s\n",tmpl.caption))
	print("---------------------------")
end

----------------------------------------------------
-- template_table keys:
--  name : string
--  id : string, ID name of template
--  serial : serial num
--  ctrlClass : string
--  x, y, w, h : int
--  style, exstyle : int
--  bk_color : DWORD
--  caption : string
--  props : { id1=value1, id2=value2, ... }
--  rdr_info : { gbl_rdr="", ctrl_rdr="", ctrl_class="", elements={id1=value1, ...}}
--
----------------------------------------------------
function dlgtmp_trans(inst)
	local ctrl_tmpl = compinst:getCompTemplateData(inst)

	--print("dlgtmp_trans")
	--print(ctrl_tmpl)

	local format = [[
	{
		"%s",/*ctrl class*/
		0x%X, /*dwstyle*/
		%d,%d,%d,%d, /*x, y, w, h*/
		%s, /*id*/
		"%s", /*caption*/
		0, /*dwAddData*/
		0x%X, /*exstyle*/
		NULL,
		NULL
	},
]]

	--print(format)

	--dump_tmpl(ctrl_tmpl)

	local str = string.format(format, ctrl_tmpl.ctrlClass, ctrl_tmpl.style, ctrl_tmpl.x, ctrl_tmpl.y, ctrl_tmpl.w, ctrl_tmpl.h, ctrl_tmpl.id, ctrl_tmpl.caption, ctrl_tmpl.exstyle)

	--print(str)

	out:write(str)
end

-- main function
function main(file, args, inst)
	out = assert(io.open(file, "wt"));

	local ctrl_data_name = ""

	if inst == 0 then
		return
	end

	--print("--------------")
	
	local dlgtmpl = compinst:getCompTemplateData(inst)

	--print("-------2");
	--print(dlgtmpl);

	if dlgtmpl == nil then
		return
	end

	local x = string.gsub(dlgtmpl.name, "ID_(%w+)", "%1")
	varname = string.gsub(x, "(%w+)_?", function (w) 
							return string.upper(string.sub(w, 0, 1)) .. string.lower(string.sub(w, 2))
							end)

	--print(varname)

	local child = compinst:getInstChildren(inst)	
	if child ~= 0 then
		ctrl_data_name = string.format("_ctrl_%s", varname)
		out:write(string.format("static CTRLDATA %s [] = {\n", ctrl_data_name))
		while child ~= 0 do
			dlgtmp_trans(child)
			child = compinst:getNextInstance(child)
		end
		out:write("\n};\n")
	end

	local controlnr = "0"
	local controls = "NULL"
	if ctrl_data_name ~= "" then
		controlnr = string.format("sizeof(%s)/sizeof(CTRLDATA)",ctrl_data_name)
		controls = ctrl_data_name
	end


	local format = [[
static DLGTEMPLATE _%s_templ = {
	0x%X, /*dwStyle*/
	0x%X, /*dwExStyle*/
	%d, %d, %d, %d, /*x, y, w, h*/
	"%s", /*caption*/
	0, /* hIcon */
	0, /* hMenu */
	%s,/*controlnr*/
	%s,/*controls*/
	NULL
};

]]

	out:write(string.format(format,
			varname,
			dlgtmpl.style,
			dlgtmpl.exstyle,
			dlgtmpl.x, dlgtmpl.y, dlgtmpl.w, dlgtmpl.h,
			dlgtmpl.caption,
			controlnr,
			controls
		))

end

