# Signing for a signed build of imx6sx-slx-som, required only when tdx-signed
# is in OVERRIDES (u-boot-imx_%.bbappend).

# meta-toradex-security signs U-Boot for the i.MX 6 SoCs it lists only; the
# 6SoloX takes the same HABv4 procedure as the 6DualLite.
uboot_assemble_fitimage_helper:append:imx6sx-slx-som() {
    type="$1"
    binary="$2"
    imx6_imx7_sign_habv4 "IMX6" "${type}" "${binary}"
}

# The FIT public key goes into U-Boot's control device tree. poky's
# concat_dtb cannot rebuild a u-boot.imx; the security layer's helpers do, as
# for the 6ULL and 7. They run before the HABv4 signing above, so the signed
# U-Boot is the one that carries the key.
concat_dtb:prepend:imx6sx-slx-som() {
    concat_dtb_pre_common "$@"
}

concat_dtb:append:imx6sx-slx-som() {
    # dts/dt.dtb depends only on the U-Boot ELF, which is older, so make would
    # never take it again from EXT_DTB, and u-boot.imx would keep the tree
    # without the key.
    rm -f dts/dt.dtb
    concat_dtb_post_common "$@"
    tail -c $(stat -c%s ${UBOOT_DTB_SIGNED}) u-boot.bin | cmp -s - ${UBOOT_DTB_SIGNED} || \
        bbfatal "u-boot.bin does not carry the FIT public key"
}

# NXP's bootm authenticates the kernel with HAB and refuses a FIT; leave a FIT
# to its signature check, as the security layer does on its modules with
# NXP's U-Boot.
SRC_URI:append:imx6sx-slx-som = " ${TDX_UBOOT_HAB_PATCHES_DOWNSTREAM}"

# hab_status fails on any HAB event; the 6SoloX boot ROM's RNG self-test event
# is a known one (see the patch). Applied right after the security layer's
# hab_status patches, which it changes.
TDX_UBOOT_HAB_STATUS_PATCHES:append:imx6sx-slx-som = " \
    file://0002-imx6sx-slx-som-hab_status-known-RNG-self-test-event-on-the-6SoloX.patch \
"

# The default, upstream patch sets do not apply to NXP's U-Boot.
TDX_UBOOT_SECBOOT_PATCHES:imx6sx-slx-som = "${TDX_UBOOT_SECBOOT_PATCHES_DOWNSTREAM}"
TDX_UBOOT_HARDENING_PATCHES:imx6sx-slx-som = "${TDX_UBOOT_HARDENING_PATCHES_DOWNSTREAM}"

# The debug build of the hardening, for tests only, fakes a closed part from
# preboot, which this U-Boot runs only with CONFIG_USE_PREBOOT.
SRC_URI:append:imx6sx-slx-som = " \
    ${@oe.utils.conditional('TDX_UBOOT_HARDENING_ENABLE_DBG', '1', 'file://u-boot-preboot.cfg', '', d)} \
"

# No class tracks the signing keys' contents, so a key replaced at the same
# path would otherwise get a U-Boot signed with the old one from sstate.
do_uboot_assemble_fitimage[file-checksums] += "\
    ${TDX_IMX_HAB_CST_SRK}:True \
    ${TDX_IMX_HAB_CST_CSF_CERT}:True \
    ${TDX_IMX_HAB_CST_IMG_CERT}:True \
    ${UBOOT_SIGN_KEYDIR}/${UBOOT_SIGN_KEYNAME}.crt:True \
"

# The sdp configuration (machine file) needs a defconfig of its own: poky
# builds each UBOOT_CONFIG type in ${B}/<defconfig>, so two types naming one
# defconfig would share a build directory. It is the NAND U-Boot's.
do_configure:prepend:imx6sx-slx-som() {
    cp ${S}/configs/imx6sx_slx_som_defconfig ${S}/configs/imx6sx_slx_som_sdp_defconfig
}

SLXSOM_UBOOT_SDP_CONFIG = "${@(d.getVarFlag('UBOOT_CONFIG', 'sdp') or '').split(',')[0]}"
SLXSOM_UBOOT_NAND_CONFIG = "${@(d.getVarFlag('UBOOT_CONFIG', 'nand') or '').split(',')[0]}"

do_configure:append:imx6sx-slx-som() {
    sdp="${B}/${SLXSOM_UBOOT_SDP_CONFIG}"
    # preboot comes from the SD card's environment, which Linux writes; only
    # the debug build of the hardening, for tests, may run it.
    if [ "${TDX_UBOOT_HARDENING_ENABLE_DBG}" != "1" ] && \
       grep -q "^CONFIG_USE_PREBOOT=y" "$sdp/.config"; then
        bbfatal "the sdp U-Boot runs preboot outside the debug build"
    fi
}

do_deploy:append:imx6sx-slx-som() {
    [ "$(readlink ${DEPLOYDIR}/${UBOOT_BINARY})" = "u-boot-nand-${PV}-${PR}.${UBOOT_SUFFIX}" ] || \
        bbfatal "${UBOOT_BINARY} is not the NAND U-Boot"
}
