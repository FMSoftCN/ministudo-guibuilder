��non-root�û�����Ҫ�޸ĵ�EliteIV deviceȨ�ޣ����ܷ��ʾ���IV�豸��

�޸ķ������£�
��һ���Զ��޸ģ����нű�usbsp.sh��

�������ֶ���дEliteIV device����Ȩ�ޣ�

1���ں˰汾���� 2.6.11 ,ʹ��HOTPLUG��ʽ��
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

2���ں˰汾���� 2.6.11 ,ʹ��udev��ʽ
(1) Edit the file /etc/udev/rules.d/50-udev.rules, add OWNER,GROUP,MODE,OPTIONS to the SUBSYSTEM="usb_device" section.
#libusb device access
DEVPATH=="/module/usbcore",ACTION=="add", ..., GROUP="users", MODE="0766"
SUBSYSTEM="usb_device", ..., OWNER="root", GROUP="users", MODE="0766", OPTIONS="last_rule"
