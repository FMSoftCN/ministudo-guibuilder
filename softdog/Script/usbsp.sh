#!/bin/sh

#script path , inside kernel pre-2.6.11
SP="/etc/hotplug/usb/usbel"
UP="/etc/hotplug/usb.usermap"

#50-udev.rules path , inside kernel post-2.6.11
RuleP="/etc/udev/rules.d/50-udev.rules"
RuleB="/etc/udev/rules.d/50-udev.rules.backup"

#Whether the current user is root or not !
if test $(id -ur) != 0 ; then
  echo "Please log in as root user"
  exit 0
fi

#Get Linux kernel release
VER2=$(uname -r | cut -d. -f2 )
VER3=$(uname -r | cut -d. -f3 | cut -d- -f1)

if [ "${VER2}" -le "6"  -a  "${VER3}" -lt "11" ]  ; then
  # --- linux version < 2.6.11 ---
  echo "usbel 0x0003 0x0471 0x485d 0x0000 0x0000 0x00 0x00 0x00 0x00 0x00 0x00 0x00000000" >> "${UP}"
  echo "#!/bin/sh" > "${SP}"
  echo "if [ \"\${ACTION}\" = \"add\" ] && [ -f \"\${DEVICE}\" ] ; then" >> "${SP}"
  echo "chmod 666 \"\${DEVICE}\"" >> "${SP}"
  echo "fi" >> "${SP}"
  chmod 711 "${SP}"
elif [ "${VER3}" -gt "11" ] ; then
  # --- linux version > 2.6.11 ---
  mv ${RuleP} ${RuleB}
  sed '/DEVPATH=="\/module\/usbcore"/s/$/, GROUP="users", MODE="0766"/' ${RuleB} > ${RuleP}
  mv ${RuleP} ${RuleB}
  sed '/SUBSYSTEM="usb_device"/s/$/, OWNER="root", GROUP="users", MODE="0766", OPTIONS="last_rule"/' ${RuleB} > ${RuleP}
else
  # --- linux version = 2.6.11 ---
  echo "Please check usb conf by hand."
fi
