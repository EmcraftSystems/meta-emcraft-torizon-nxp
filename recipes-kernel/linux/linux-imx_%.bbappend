FILESEXTRAPATHS:prepend:imx6sx-slx-som := "${THISDIR}/linux-imx:"

# torizon.cfg is meta-toradex-torizon's; linux-torizon.inc adds it only for the
# machines it lists.
SRC_URI:append:imx6sx-slx-som = " \
    file://0001-ARM-dts-imx6sx-add-the-Emcraft-SLX-SOM.patch \
    file://torizon.cfg \
"

# A signed build: linux-torizon.inc adds the composefs configuration as .scc
# files, which linux-imx does not read. Its .cfg files, which ship in
# meta-toradex-torizon, go through DELTA_KERNEL_DEFCONFIG; they must also be in
# SRC_URI, since do_merge_delta_config skips a file missing from WORKDIR
# without an error.
SRC_URI:append:imx6sx-slx-som:cfs-support = " file://composefs.cfg"
DELTA_KERNEL_DEFCONFIG:append:imx6sx-slx-som:cfs-support = " composefs.cfg"
SRC_URI:append:imx6sx-slx-som:cfs-signed = " file://composefs-fsverity.cfg"
DELTA_KERNEL_DEFCONFIG:append:imx6sx-slx-som:cfs-signed = " composefs-fsverity.cfg"
require ${@'linux-imx-slx-som-signed.inc' if d.getVar('MACHINE') == 'imx6sx-slx-som' and 'tdx-signed' in d.getVar('OVERRIDES').split(':') else ''}
