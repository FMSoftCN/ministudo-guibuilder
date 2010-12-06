对non-root用户，需要修改的EliteIV device权限，才能访问精锐IV设备。

修改方法如下：
（一）自动修改，运行脚本usbsp.sh。

（二）手动改写EliteIV device访问权限：

1，内核版本低于 2.6.11 ,使用HOTPLUG方式。
(1)add a line in /etc/hotplug/usb.usermap
#EliteIV VID=0471 PID=485d
usbel 0x0003 0x0471 0x485d 0x0000 0x0000 0x00 0x00 0x00 0x00 0x00 0x00 0x00000000

(2)create a script file named "usbel" in /etc/hotplug/usb/
#!/bin/bash
if [ "${ACTION}" = "add" ] && [ -f "${DEVICE}" ]
then
chmod 666 "${DEVICE}"
fi

(3)
chmod 711 usbel

2，内核版本高于 2.6.11 ,使用udev方式
(1) Edit the file /etc/udev/rules.d/50-udev.rules, add OWNER,GROUP,MODE,OPTIONS to the SUBSYSTEM="usb_device" section.
#libusb device access
DEVPATH=="/module/usbcore",ACTION=="add", ..., GROUP="users", MODE="0766"
SUBSYSTEM="usb_device", ..., OWNER="root", GROUP="users", MODE="0766", OPTIONS="last_rule"
