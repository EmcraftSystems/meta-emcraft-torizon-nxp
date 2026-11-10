# imx8mp-navqp builds voxelbotics' U-Boot 2023.04; every other machine keeps meta-imx's.
require ${@'recipes-bsp/u-boot/u-boot-imx-navqp.inc' if d.getVar('MACHINE') == 'imx8mp-navqp' else ''}

FILESEXTRAPATHS:prepend:imx6sx-slx-som := "${THISDIR}/u-boot-imx:"

SRC_URI:append:imx6sx-slx-som = " \
    file://0001-imx6sx-add-the-Emcraft-SLX-SOM.patch \
    file://slx-som-boot.cfg \
"

# A signed build signs U-Boot for HABv4; see the include. CONFIG_IMX_HAB
# (u-boot-hab.cfg) makes the DCD's CSF line put a CSF pointer in the IVT.
require ${@'u-boot-imx-slx-som-signed.inc' if d.getVar('MACHINE') == 'imx6sx-slx-som' and 'tdx-signed' in d.getVar('OVERRIDES').split(':') else ''}
