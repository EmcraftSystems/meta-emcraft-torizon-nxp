# Signing for a signed build of imx6sx-slx-som, required only when tdx-signed
# is in OVERRIDES (u-boot-imx_%.bbappend).

# meta-toradex-security signs U-Boot for the i.MX 6 SoCs it lists only; the
# 6SoloX takes the same HABv4 procedure as the 6DualLite.
uboot_assemble_fitimage_helper:append:imx6sx-slx-som() {
    type="$1"
    binary="$2"
    imx6_imx7_sign_habv4 "IMX6" "${type}" "${binary}"

    # The boot data length runs to the end of the CSF area (CONFIG_CSF_SIZE),
    # past the end of the signed file. nandbcb records in the FCB only the
    # pages the file fills, and the boot ROM loads nothing from a NAND image
    # shorter than its boot data says. So the file is padded with zeros to
    # the boot data length, less the 1 KiB before the IVT it starts with.
    signed="${UBOOT_BINARYNAME}-${type}.${UBOOT_SUFFIX}"
    [ -n "${type}" ] || signed="${UBOOT_BINARY}"
    len=$(od -An -tu4 -j36 -N4 "$signed" | tr -d ' ')
    [ -n "$len" ] && [ "$len" -gt $(stat -c%s "$signed") ] || \
        bbfatal "$signed: boot data length '$len' does not reach past the file"
    truncate -s $(expr $len - 1024) "$signed"
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
# path would otherwise get a U-Boot signed with the old one from sstate. The
# SRK fuse file is read here too, for fuse-cmds.txt and the fusing inputs.
do_uboot_assemble_fitimage[file-checksums] += "\
    ${TDX_IMX_HAB_CST_SRK}:True \
    ${TDX_IMX_HAB_CST_SRK_FUSE}:True \
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

# Installing a closed part over the serial downloader: Toradex's Fastboot
# protection, ported to NXP's fastboot (see the patch), lets the sdp U-Boot
# run fastboot and the NAND install's commands under the hardening. It is
# built into sdp only: the NAND U-Boot keeps refusing fastboot on a closed
# part. The symbols depend on TDX_SECBOOT_HARDENING, which the hardening's
# fragment sets after the defconfig, so they come in a fragment merged into
# sdp's build directory, as the hardening merges its own per-type fragment.
# It is not in SRC_URI, where poky would merge it into every type.
SRC_URI:append:imx6sx-slx-som = " \
    file://0003-imx6sx-slx-som-Toradex-Fastboot-protection-on-NXP-s-fastboot-for-the-NAND-install.patch \
"

do_configure:append:imx6sx-slx-som() {
    sdp="${B}/${SLXSOM_UBOOT_SDP_CONFIG}"
    nand="${B}/${SLXSOM_UBOOT_NAND_CONFIG}"
    printf '%s\n' CONFIG_TDX_FB_PROTECTION=y CONFIG_TDX_FB_PROTECTION_ALLOW_CMD=y \
        > ${WORKDIR}/sdp-install.cfg
    merge_config.sh -m -O "$sdp" "$sdp/.config" ${WORKDIR}/sdp-install.cfg
    oe_runmake -C ${S} O="$sdp" oldconfig

    for sym in TDX_FB_PROTECTION TDX_FB_PROTECTION_ALLOW_CMD; do
        grep -q "^CONFIG_$sym=y" "$sdp/.config" || \
            bbfatal "CONFIG_$sym is not set in the sdp U-Boot"
        ! grep -q "^CONFIG_$sym=y" "$nand/.config" || \
            bbfatal "CONFIG_$sym is set in the NAND U-Boot"
    done
    # Torizon's fusing steps through the fuse words with setexpr; without it
    # every word would go to the first one.
    grep -q "^CONFIG_CMD_SETEXPR=y" "$nand/.config" || \
        bbfatal "the NAND U-Boot has no setexpr, which Torizon's fusing needs"
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

# Inputs for Torizon's fusing on this board, made from the SRK hash the build
# just wrote to imx-config.fuse, so that no value is typed by hand. A wrong
# word written to a fuse cannot be undone.
#   fuse-firstboot-dryrun.env  fw_setenv -s: first-boot fusing, dry run, with
#                              the close, to rehearse on a part never fused
#   fuse-firstboot-srk.env     fw_setenv -s: first-boot fusing of the SRK
#                              hash, no close
#   fuse-firstboot-close.env   fw_setenv -s: first-boot fusing of the SRK
#                              hash and the close, without Torizon Cloud
#   fuse-dryrun.yml            fuse package, eight zero words, no close:
#                              programs nothing even without a dry run
#   fuse-srk.yml               fuse package, the SRK hash, no close
#   fuse-close.yml             fuse package, the SRK hash and the close
# The environment is on the SD card with the module's own settings, such as
# ethaddr, so the .env files are merged into it from Linux with fw_setenv -s,
# not written over it. The .yml files are in TorizonCore Builder's canonical
# form, as its own fuse push would make them; the hardware ID is
# imx6sx-slx-som-fuses.
do_deploy:append:imx6sx-slx-som() {
    cfg=${WORKDIR}/imx-config.fuse
    [ -e "$cfg" ] || bbfatal "no imx-config.fuse to make the fuse files from"
    words=$(sed -n 's/^H:F:3:\([0-7]\):\(0x[0-9A-Fa-f]\{1,8\}\)$/\1 \2/p' $cfg | sort -n | \
        while read w v; do printf '0x%x\n' $v; done)
    [ $(echo "$words" | wc -l) = 8 ] || bbfatal "imx-config.fuse does not hold bank 3 words 0 to 7"
    sed -n 's/^H:F:3:\([0-7]\):.*/\1/p' $cfg | tr '\n' ' ' | grep -qx '0 1 2 3 4 5 6 7 ' || \
        bbfatal "imx-config.fuse's SRK words are not bank 3 words 0 to 7, in order"
    grep -qx 'H:C:0:6:0x00000002' $cfg || \
        bbfatal "imx-config.fuse closes with something other than bank 0 word 6, 0x2"
    srk=$(hexdump -v -e '/4 "0x%x\n"' ${TDX_IMX_HAB_CST_SRK_FUSE})
    [ "$words" = "$srk" ] || bbfatal "imx-config.fuse differs from ${TDX_IMX_HAB_CST_SRK_FUSE}"
    list=$(echo $words)
    printf 'fuse_prog_list=%s\nfuse_prog_close=1\nfuse_status=pending\nfuse_dry_run=1\n' "$list" \
        > ${DEPLOYDIR}/fuse-firstboot-dryrun.env
    printf 'fuse_prog_list=%s\nfuse_prog_close=0\nfuse_status=pending\nfuse_dry_run=0\n' "$list" \
        > ${DEPLOYDIR}/fuse-firstboot-srk.env
    printf 'fuse_prog_list=%s\nfuse_prog_close=1\nfuse_status=pending\nfuse_dry_run=0\n' "$list" \
        > ${DEPLOYDIR}/fuse-firstboot-close.env
    { echo 'fuses:'; echo '  fuse-close: false'
      for i in 1 2 3 4 5 6 7 8; do echo "  fuse-val$i: 0x0"; done; } > ${DEPLOYDIR}/fuse-dryrun.yml
    { echo 'fuses:'; echo '  fuse-close: false'
      i=1; for v in $words; do echo "  fuse-val$i: $v"; i=$(expr $i + 1); done; } > ${DEPLOYDIR}/fuse-srk.yml
    { echo 'fuses:'; echo '  fuse-close: true'
      i=1; for v in $words; do echo "  fuse-val$i: $v"; i=$(expr $i + 1); done; } > ${DEPLOYDIR}/fuse-close.yml
    for f in fuse-firstboot-dryrun.env fuse-firstboot-srk.env fuse-firstboot-close.env \
             fuse-dryrun.yml fuse-srk.yml fuse-close.yml; do
        ! grep -q '[[:space:]]$' ${DEPLOYDIR}/$f || bbfatal "$f has trailing whitespace"
    done
}
