# A signed build of imx6sx-slx-som asks the boot script for the U-Boot
# hardening's kernel command-line overlay, as the security layer's own
# u-boot-distro-boot bbappend does in builds with meta-toradex-bsp-common,
# which the NXP manifest does not carry. The security layer is in the build
# only when it is signed.
require ${@'recipes-bsp/u-boot/u-boot-distro-boot-sec-features.inc' if d.getVar('MACHINE') == 'imx6sx-slx-som' and 'tdx-signed' in d.getVar('OVERRIDES').split(':') and not bb.utils.contains('BBFILE_COLLECTIONS', 'toradex-bsp-common-layer', True, False, d) else ''}

# Torizon's boot script is not the BSP script the security layer's amendments
# edit; Torizon turns them off only for its torizon-distro and common-distro
# overrides.
TDX_AMEND_BOOT_SCRIPT:imx6sx-slx-som = "0"

# Torizon's boot script carries its fusing code (first-boot fusing and the
# fuse package's handler) only for the SoCs it lists; the 6SoloX is not
# among them, so it would get the dummy block. These are Torizon's own
# values for its i.MX 6 modules (6Quad, 6DualLite): SRK hash in bank 3,
# words 0 to 7, closed by SEC_CONFIG[1] in bank 0 word 6. An unsigned build
# keeps the dummy block.
BANK_WORD:imx6sx-slx-som = "${IMX6_COMMON_BANK_WORD}"
FUSE_NUM:imx6sx-slx-som = "${FUSE_NUM_8}"
STATUS_CMD:imx6sx-slx-som = "${HAB_STATUS_CMD}"
CLOSE_CMD:imx6sx-slx-som = "${IMX6_COMMON_CLOSE_CMD}"
FUSE_SUPPORT_DEFAULT:imx6sx-slx-som = "${FUSE_SUPPORT_SIGNED_BUILD}"
FUSE_SUPPORT_TYPE:imx6sx-slx-som = "NXP"

do_compile:append:imx6sx-slx-som() {
    if [ "${FUSE_SUPPORT}" = "1" ]; then
        grep -q '^prog_fuses=if test' uEnv.txt || \
            bbfatal "a signed build's uEnv.txt lacks Torizon's fusing code"
        grep -q '^fuse_bank_word_8=3 7$' uEnv.txt || \
            bbfatal "uEnv.txt does not map fuse 8 to bank 3 word 7"
        grep -q '^fuse_num=1 2 3 4 5 6 7 8$' uEnv.txt || \
            bbfatal "uEnv.txt does not list eight fuses"
        grep -qF 'hab_status;' uEnv.txt || \
            bbfatal "uEnv.txt does not check hab_status before the close"
        grep -qF 'fuse prog -y 0 6 0x00000002;' uEnv.txt || \
            bbfatal "uEnv.txt does not close by bank 0 word 6, 0x2"
    fi
}
