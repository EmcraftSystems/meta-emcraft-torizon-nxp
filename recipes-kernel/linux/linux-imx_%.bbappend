FILESEXTRAPATHS:prepend:imx6sx-slx-som := "${THISDIR}/linux-imx:"

# torizon.cfg is meta-toradex-torizon's (linux-torizon.inc); its SRC_URI append
# names only Toradex's own machines.
SRC_URI:append:imx6sx-slx-som = " \
    file://0001-ARM-dts-imx6sx-add-the-Emcraft-SLX-SOM.patch \
    file://torizon.cfg \
"
