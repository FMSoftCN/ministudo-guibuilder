---------------------------------------

osname="linux" -- "windows" or others

if string.find(package.cpath, ".dll") ~= nil then
	osname = "windows"
end


-- function to get the xvfb execute file
local function get_xvfb_exec()

	-- try get from env
	local guibuilder_path = os.getenv("GUIBUILDER_PATH")

	if guibuilder_path and #guibuilder_path > 0 then
        if osname == "linux" then
            guibuilder_path = string.format("%s/bin/gvfb", guibuilder_path)
        elseif osname == "windows" then
            guibuilder_path = string.format("%s\\wvfb2.exe", guibuilder_path)
        end
		return guibuilder_path
	end

	if osname == "linux" then
		return "/usr/local/bin/gvfb"
	elseif osname == "windows" then
		return "c:\\windows\\wvfb2.exe"
	end

	return ""
end

local function get_xvfb_caption()
	return "miniStudio Emulator"
end

local function get_resource_path()

	local guibuilder_path = os.getenv("GUIBUILDER_PATH")

	if guibuilder_path and #guibuilder_path > 0 then
        if osname == "linux" then
            guibuilder_path = string.format("%s/share/minigui/res/", guibuilder_path)
        elseif osname == "windows" then
            guibuilder_path = string.format("%s\\config\\minigui-res\\", guibuilder_path)
        end
		return guibuilder_path
	end

	if osname == "linux" then
		return "/usr/local/share/minigui/res/"
	elseif osname == "windows" then
		return "c:\\minigui-res\\"
	end
	return ""
end

local function get_cursor_path()
	local cur_path = get_resource_path()

	if osname == "linux" then
		return cur_path .. "cursor/"
	elseif osname == "windows" then
		return cur_path .. "cursor\\"
	end

	return ""
end

local function get_depth_mode(depth)
	local depth_mode= { "16bpp.rgb565", "16bpp.rgb555","24bpp","32bpp"}
	if depth ~= nil and depth >= 0 and depth < #depth_mode then
		return depth_mode[depth+1]
	else
		return "16bpp"
	end
end

---------------------------------------
--[[
--- dump the cfginfo
function dump_table(prefix, table)
	if type(table) ~= "table" then return end

	-- set prefix
	prefix = prefix or ''

	for k,v in pairs(table) do
		print(prefix, k, v)
		if type(v) == "table" then
			dump_table(prefix .. "\t", v)
		end
	end

end

function dump_cfginfo()
	print("======================================================")

	if not cfginfo then
		print("cfginfo is nil")
	else
		print("cfginfo table:")
		dump_table("\t", cfginfo)
	end

	print("======================================================")

end

--- dump cfginfo
dump_cfginfo()
]]
---------------------------------------


local function out_fonts(out, str_format, count, fontname)

	if not cfginfo.fonts then
		out:write(string.format(str_format, count, ""))
		return
	end

	-- get font table
	local font_table = cfginfo.fonts[fontname]
	local font_infos = ""

	if font_table then
		for k,v in pairs(font_table) do
			font_infos = font_infos .. string.format("name%d=%s\nfontfile%d=%s\n", count, k, count, v)
			count = count + 1
		end
	end

	out:write(string.format(str_format, count, font_infos))

end

if not cfginfo then
	print("Error: cfginfo is nil")
	return
end


----- open the MiniGUI.cfg
out = io.open(cfginfo.projectDir .. "/MiniGUI.cfg", "wt")

if not out then
	print("cannot open " .. cfginfo.projectDir .. "/MiniGUI.cfg")
	return
end


out:write([[
# MiniGUI Ver 2.2.1
# This configuration file is for MiniGUI Look and Feel.
#
# Copyright (C) 2002~2010 FMSoft.
# Copyright (C) 1998~2002 Wei Yongming.
#
# Web:   http://www.minigui.com
# Web:   http://www.minigui.org
#
# This configuration file must be installed in /etc,
# /usr/local/etc or your home directory. When you install it in your
# home directory, it should be named ".MiniGUI.cfg".
#
# The priority of above configruation files is ~/.MiniGUI.cfg,
# /usr/local/etc/MiniGUI.cfg, and then /etc/MiniGUI.cfg.
#
# If you change the install path of MiniGUI resource, you should
# modify this file to meet your configuration.
#
# NOTE:
# The format of this configuration file has changed since the last release.
# Please DONT forget to provide the latest MiniGUI.cfg file for your MiniGUI.
#

[system]
# GAL engine and default options
gal_engine=pc_xvfb
]])



out:write(string.format("defaultmode=%dx%d-%s\n", cfginfo.screen.width, cfginfo.screen.height, get_depth_mode(cfginfo.screen.depth)))

out:write([[
# IAL engine
ial_engine=pc_xvfb
mdev=/dev/input/mice
mtype=IMPS2

[fbcon]
defaultmode=1024x768-16bpp

[qvfb]
defaultmode=1024x768-16bpp
display=0

#{{ifdef _MGGAL_PCXVFB
[pc_xvfb]
]])

out:write(string.format([[
defaultmode=%dx%d-%s
window_caption=%s
exec_file=%s
#}}

]], cfginfo.screen.width, cfginfo.screen.height, get_depth_mode(cfginfo.screen.depth), get_xvfb_caption(), get_xvfb_exec() ))


out:write([[
#{{ifdef _MGGAL_SHADOW
[shadow]
real_engine=pc_xvfb
defaultmode=1024x768-16bpp
rotate_screen=normal
#}}

#{{ifdef _MGGAL_MLSHADOW
[mlshadow]
real_engine=qvfb
defaultmode=1024x768-16bpp
def_bgcolor=0x00FF00
double_buffer=enable
#}}

]])

local def_font_count   = 2
local def_cap_font_id  = 1
local def_ctrl_font_id = 1
local def_fonts = ""

if cfginfo.caption_font then
	def_cap_font_id =  def_font_count
	def_font_count = def_font_count + 1
	def_fonts = string.format("font%d=%s\n", def_cap_font_id, cfginfo.caption_font)
end

if cfginfo.control_font then
	def_ctrl_font_id = def_font_count
	def_font_count = def_font_count + 1
	def_fonts = def_fonts .. string.format("font%d=%s\n", def_ctrl_font_id, cfginfo.control_font)
end


out:write(string.format([[
# The first system font must be a logical font using RBF device font.
[systemfont]
font_number=%d
font0=rbf-FixedSys-rrncnn-8-16-ISO8859-1
font1=upf-unifont-rrncnn-16-16-UTF-8
%s

]], def_font_count, def_fonts))


out:write(string.format([[

default=0
wchar_def=1
fixed=1
caption=%d
menu=1
control=%d

]],def_cap_font_id, def_ctrl_font_id))


out:write([[
[rawbitmapfonts]
font_number=0
name0=rbf-fixed-rrncnn-8-16-ISO8859-1
fontfile0=font/8x16-iso8859-1.bin

[varbitmapfonts]
font_number=0
name0=vbf-Courier-rrncnn-8-13-ISO8859-1
fontfile0=font/Courier-rr-8-13.vbf

]])


local upf_str = [[
[upf]
font_number=%d
name0=upf-unifont-rrncnn-16-16-ISO8859-1,ISO8859-15,GB2312,BIG5,GBK,UTF-8,UTF-16LE,UTF-16BE
fontfile0=font/unifont_160_50.upf
%s

]]
out_fonts(out, upf_str, 1, "upf")


out:write([[
[qpf]
font_number=0

]])


local ttf_format=[[
[truetypefonts]
font_number=%d
%s

]]
out_fonts(out, ttf_format, 0, "ttf")

out:write([[
[type1fonts]
font_number=0

]])


out:write([[
[mouse]
dblclicktime=300

[event]
timeoutusec=300000
repeatusec=50000

]])

out:write(string.format([[
[cursorinfo]
# Edit following line to specify cursor files path
cursorpath=%s
cursornumber=23
cursor0=d_arrow.cur
cursor1=d_beam.cur
cursor2=d_pencil.cur
cursor3=d_cross.cur
cursor4=d_move.cur
cursor5=d_sizenwse.cur
cursor6=d_sizenesw.cur
cursor7=d_sizewe.cur
cursor8=d_sizens.cur
cursor9=d_uparrow.cur
cursor10=d_none.cur
cursor11=d_help.cur
cursor12=d_busy.cur
cursor13=d_wait.cur
cursor14=g_rarrow.cur
cursor15=g_col.cur
cursor16=g_row.cur
cursor17=g_drag.cur
cursor18=g_nodrop.cur
cursor19=h_point.cur
cursor20=h_select.cur
cursor21=ho_split.cur
cursor22=ve_split.cur

]],get_cursor_path()))


out:write([[
[imeinfo]
imetabpath=/usr/local/share/minigui/res/imetab/
imenumber=1
ime0=pinyin

[appinfo]
apprespath=/usr/local/share/shared/miniguiapps/

]])

out:write(string.format([[
[resinfo]
respath=%s

]], get_resource_path()))

out:write([=[
[classic]
# Note that max number defined in source code is 5.
iconnumber=5
icon0=form.ico
icon1=failed.ico
icon2=help.ico
icon3=warning.ico
icon4=excalmatory.ico

# default icons for new OpenFileDialogBox
dir=folder.ico
file=textfile.ico

# default icons for TreeView control
treefold=fold.ico
treeunfold=unfold.ico

# bitmap used by BUTTON control
radiobutton=classic_radio_button.bmp
checkbutton=classic_check_button.bmp

# background picture, use your favirate photo
bgpicture=none
bgpicpos=center
# bgpicpos=upleft
# bgpicpos=downleft
# bgpicpos=upright
# bgpicpos=downright
# bgpicpos=upcenter
# bgpicpos=downcenter
# bgpicpos=vcenterleft
# bgpicpos=vcenterright
# bgpicpos=none

#window element metrics
caption=20
menu=25
border=2
scrollbar=16

#window element colors
fgc_active_caption=0xFFFFFFFF
bgca_active_caption=0xFF6A240A
bgcb_active_caption=0xFF6A240A

fgc_menu=0xFF000000
bgc_menu=0xFFCED3D6


fgc_msgbox=0xFF000000

fgc_tip=0xFF000000
bgc_tip=0xFFE7FFFF

fgc_active_border=0xFFCED3D6
fgc_inactive_border=0xFFCED3D6

fgc_inactive_caption=0xFFC8D0D4
bgca_inactive_caption=0xFF808080
bgcb_inactive_caption=0xFF808080

fgc_window=0xFF000000
bgc_window=0xFFFFFFFF

fgc_3dbox=0xFF000000
mainc_3dbox=0xFFCED3D6

fgc_selected_item=0xFFFFFFFF
bgc_selected_item=0xFF6B2408
bgc_selected_lostfocus=0xFFBDA69C

fgc_disabled_item=0xFF848284
bgc_disabled_item=0xFFCED3D6

fgc_hilight_item=0xFFFFFFFF
bgc_hilight_item=0xFF6B2408

fgc_significant_item=0xFFFFFFFF
bgc_significant_item=0xFF6B2408

bgc_desktop=0xFFC08000

#{{ifdef _MGLF_RDR_FLAT
[flat]
# Note that max number defined in source code is 5.
iconnumber=5
icon0=form-flat.ico
icon1=failed-flat.ico
icon2=help-flat.ico
icon3=warning-flat.ico
icon4=excalmatory-flat.ico

# default icons for new OpenFileDialogBox
dir=folder-flat.ico
file=textfile-flat.ico

# default icons for TreeView control
treefold=fold-flat.ico
treeunfold=unfold-flat.ico

# bitmap used by BUTTON control
radiobutton=flat_radio_button.bmp
checkbutton=flat_check_button.bmp

# background picture, use your favirate photo
bgpicture=none
bgpicpos=center

#window element metrics
caption=20
menu=25
border=1
scrollbar=16

#window element colors
fgc_active_caption=0xFFFFFFFFF
bgca_active_caption=0xFF000000
bgcb_active_caption=0xFF000000

fgc_inactive_caption=0xFF000000
bgca_inactive_caption=0xFFFFFFFF
bgcb_inactive_caption=0xFFFFFFFF

fgc_menu=0xFF000000
bgc_menu=0xFFD8D8D8

fgc_msgbox=0xFF000000

fgc_tip=0xFF000000
bgc_tip=0xFFE7FFFF

fgc_active_border=0xFF000000
fgc_inactive_border=0xFF848284

fgc_window=0xFF000000
bgc_window=0xFFFFFFFF

fgc_3dbox=0xFF000000
mainc_3dbox=0xFFFFFFFF

fgc_selected_item=0xFFFFFFFF
bgc_selected_item=0xFF000000
bgc_selected_lostfocus=0xFFBDA69C

fgc_disabled_item=0xFF848284
bgc_disabled_item=0xFF000000

fgc_hilight_item=0xFFFFFFFF
bgc_hilight_item=0xFF000000

fgc_significant_item=0xFFFFFFFF
bgc_significant_item=0xFF000000

bgc_desktop=0xFFC08000

flat_tab_normal_color=0xFFC6D2CF
#}}

#{{ifdef _MGLF_RDR_SKIN
[skin]
# Note that max number defined in source code is 5.
iconnumber=5
icon0=form.ico
icon1=failed.ico
icon2=help.ico
icon3=warning.ico
icon4=excalmatory.ico

# default icons for new OpenFileDialogBox
dir=folder.ico
file=textfile.ico

# default icons for TreeView control
treefold=fold.ico
treeunfold=unfold.ico

# background picture, use your favirate photo
bgpicture=none
bgpicpos=center

#window element metrics
caption=25
menu=25
border=1
scrollbar=17

fgc_active_caption=0xFFFFFFFF
bgca_active_caption=0xFFE35400
bgcb_active_caption=0xFF686868

fgc_menu=0xFF000000
#bgc_menu=0xFFD4D6FF
bgc_menu=0xFFD8E9EC

fgc_msgbox=0xFF000000

fgc_tip=0xFF000000
bgc_tip=0xFFFFFFFF

fgc_active_border=0xFFC8D0D4
fgc_inactive_border=0xFFC8D0D4

fgc_inactive_caption=0xFFF8E4D8
bgca_inactive_caption=0xFFDF967A
bgcb_inactive_caption=0xFF686868

fgc_window=0xFF000000
bgc_window=0xFFFFFFFF

fgc_3dbox=0xFF000000
mainc_3dbox=0xFFD8E9EC

fgc_selected_item=0xFFFFFFFF
bgc_selected_item=0xFFC56A31
bgc_selected_lostfocus=0xFFD8E9EC

fgc_disabled_item=0xFF99A8AC
bgc_disabled_item=0xFFFFFFFF

fgc_hilight_item=0xFFFFFFFF
bgc_hilight_item=0xFFC56A31

fgc_significant_item=0xFFFFFFFF
bgc_significant_item=0xFFC56A31

bgc_desktop=0xFF984E00

skin_bkgnd=skin_bkgnd.bmp
skin_caption=skin_caption.gif
skin_caption_btn=skin_cpn_btn.gif

#for scrollbar
skin_scrollbar_hshaft=skin_sb_hshaft.bmp
skin_scrollbar_vshaft=skin_sb_vshaft.bmp
skin_scrollbar_hthumb=skin_sb_hthumb.bmp
skin_scrollbar_vthumb=skin_sb_vthumb.bmp
skin_scrollbar_arrows=skin_sb_arrows.bmp

#for border
skin_tborder=skin_tborder.bmp
skin_bborder=skin_bborder.bmp
skin_lborder=skin_lborder.bmp
skin_rborder=skin_rborder.bmp

skin_arrows=skin_arrows.gif
skin_arrows_shell=skin_arrows_shell.bmp

skin_pushbtn=skin_pushbtn.gif
skin_radiobtn=skin_radiobtn.gif
skin_checkbtn=skin_checkbtn.bmp

#for treeview
skin_tree=skin_tree.bmp

skin_header=skin_header.bmp
skin_tab=skin_tab.gif

#for trackbar
skin_tbslider_h=skin_tbslider_h.gif
skin_tbslider_v=skin_tbslider_v.gif
skin_trackbar_horz=skin_tb_horz.gif
skin_trackbar_vert=skin_tb_vert.gif

#for progressbar
skin_progressbar_htrack=skin_pb_htrack.gif
skin_progressbar_vtrack=skin_pb_vtrack.gif
skin_progressbar_hchunk=skin_pb_htruck.bmp
skin_progressbar_vchunk=skin_pb_vtruck.bmp
#}}


[fashion]
# Note that max number defined in source code is 5.
iconnumber=5
icon0=form.ico
icon1=failed.ico
icon2=mg_help.ico
icon3=warning.ico
icon4=excalmatory.ico

# default icons for new OpenFileDialogBox
dir=folder.ico
file=textfile.ico

# default icons for TreeView control
treefold=fold.ico
treeunfold=unfold.ico

# bitmap used by BUTTON control
radiobutton=fashion_radio_btn.bmp
checkbutton=fashion_check_btn.bmp

# background picture, use your favirate photo
bgpicture=none
bgpicpos=center

#window element metrics
caption=25
menu=25
border=1
scrollbar=17

fgc_active_caption=0xFFFFFFFF
bgca_active_caption=0xFFE35400
bgcb_active_caption=0xFFFF953D

fgc_menu=0xFF000000
bgc_menu=0xFFFFE4BF

fgc_msgbox=0xFF000000

fgc_tip=0xFF000000
bgc_tip=0xFFFFFFFF

fgc_active_border=0xFFC8D0D4
fgc_inactive_border=0xFF000000

fgc_inactive_caption=0xFFF8E4D8
bgca_inactive_caption=0xFFDF967A
bgcb_inactive_caption=0xFFEBB99D

fgc_window=0xFF000000
bgc_window=0xFFEBB99D

fgc_3dbox=0xFF000000
mainc_3dbox=0xFFD8E9EC

fgc_selected_item=0xFFFFFFFF
bgc_selected_item=0xFFC56A31
bgc_selected_lostfocus=0xFFD8E9EC

fgc_disabled_item=0xFF99A8AC
bgc_disabled_item=0xFFFFFFFF

fgc_hilight_item=0xFFFFFFFF
bgc_hilight_item=0xFFC56A31

fgc_significant_item=0xFFFFFFFF
bgc_significant_item=0xFFC56A31

bgc_desktop=0xFF984E00

]=])

out:close()

--- end out MiniGUI.cfg
