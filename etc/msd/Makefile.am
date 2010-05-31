

RESPATH :=$(shell awk -F = '/^respath/ {print $$2}' < ../MiniGUI.cfg)
ETCDIR=$(RESPATH)/bmp/msd

DataFiles=skin_arrows.gif \
	skin_arrows_shell.bmp \
	skin_bborder.bmp \
	skin_bkgnd_b.bmp \
	skin_bkgnd.bmp \
	skin_bkgnd_c.bmp \
	skin_caption_b.gif \
	skin_caption.gif \
	skin_checkbtn.bmp \
	skin_cpn_btn.gif \
	skin_header.bmp \
	skin_lborder.bmp \
	skin_pb_hchunk.bmp \
	skin_pb_htrack.gif \
	skin_pb_vchunk.bmp \
	skin_pb_vtrack.gif \
	skin_pushbtn.gif \
	skin_radiobtn.gif \
	skin_rborder.bmp \
	skin_sb_arrows.bmp \
	skin_sb_hshaft.bmp \
	skin_sb_hthumb.bmp \
	skin_sb_vshaft.bmp \
	skin_sb_vthumb.bmp \
	skin_tab.gif \
	skin_tb_horz.gif \
	skin_tborder.bmp \
	skin_tbslider_h.bmp \
	skin_tbslider_h.gif \
	skin_tbslider_v.bmp \
	skin_tbslider_v.gif \
	skin_tb_vert.gif \
	skin_tree.bmp

install-data-local:
	if [ ! -d $(ETCDIR) ]; then $(INSTALL) -d $(ETCDIR); fi


	$(INSTALL_DATA) $(DataFiles) $(ETCDIR)



EXTRA_DIST= $(DataFiles)
