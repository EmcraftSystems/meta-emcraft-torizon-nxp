require recipes-bsp/u-boot/u-boot-ota.inc

FILESEXTRAPATHS:prepend := "${THISDIR}/u-boot-imx:"
FILESEXTRAPATHS:prepend := "${THISDIR}/u-boot-ota:"

SRC_URI = "git://github.com/voxelbotics/u-boot-imx.git;protocol=https;branch=imx-6.1.22-vb;nobranch=1"

SRC_URI += "\
	file://envimport.cfg \
"

SRCREV = "ef538bb0d8dccb987053988dbceb94780dcab1b9"

# NOTE: this fixes release build with the overridden SRCREV in site.conf
PV = "2023.04-${SRCPV}"

do_install:append () {
        ln -sf ${sysconfdir}/u-boot-imx-initial-env ${D}/${sysconfdir}/u-boot-initial-env
}

FILES:${PN}-env += " ${sysconfdir}/u-boot-initial-env "
LOCALVERSION = "-1.1.14-446-01231723-fc6d7c3511"
