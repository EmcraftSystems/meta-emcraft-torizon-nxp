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
