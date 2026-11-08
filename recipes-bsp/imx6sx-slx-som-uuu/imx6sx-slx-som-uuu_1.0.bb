SUMMARY = "uuu script that installs U-Boot in the Emcraft SLX-SOM's NAND"
DESCRIPTION = "Host-side uuu script: loads U-Boot over the boot ROM's serial \
downloader, then writes it to the NAND with its boot control blocks. It opens \
the bootloader by its deployed name, so it is deployed beside it."
HOMEPAGE = "https://github.com/EmcraftSystems/meta-emcraft-torizon-nxp"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://imx6sx-slx-som-nand.uuu"

S = "${WORKDIR}"

inherit deploy

do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_deploy() {
    install -m 0644 ${S}/imx6sx-slx-som-nand.uuu ${DEPLOYDIR}/imx6sx-slx-som-nand.uuu
}
addtask deploy before do_build after do_compile

COMPATIBLE_MACHINE = "imx6sx-slx-som"
