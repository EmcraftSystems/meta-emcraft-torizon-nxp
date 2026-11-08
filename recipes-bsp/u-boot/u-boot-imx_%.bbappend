# imx8mp-navqp builds voxelbotics' U-Boot 2023.04; every other machine keeps meta-imx's.
require ${@'recipes-bsp/u-boot/u-boot-imx-navqp.inc' if d.getVar('MACHINE') == 'imx8mp-navqp' else ''}

FILESEXTRAPATHS:prepend:imx6sx-slx-som := "${THISDIR}/u-boot-imx:"

SRC_URI:append:imx6sx-slx-som = " \
    file://0001-imx6sx-add-the-Emcraft-SLX-SOM.patch \
    file://slx-som-boot.cfg \
"
