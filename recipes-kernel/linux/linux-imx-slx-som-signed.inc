# Required only for imx6sx-slx-som with tdx-signed in OVERRIDES
# (linux-imx_%.bbappend).

# No class tracks the FIT key's contents, so a key replaced at the same path
# would otherwise get a FIT signed with the old one from sstate.
do_assemble_fitimage[file-checksums] += "\
    ${UBOOT_SIGN_KEYDIR}/${UBOOT_SIGN_KEYNAME}.key:True \
    ${UBOOT_SIGN_KEYDIR}/${UBOOT_SIGN_KEYNAME}.crt:True \
"
do_assemble_fitimage_initramfs[file-checksums] += "\
    ${UBOOT_SIGN_KEYDIR}/${UBOOT_SIGN_KEYNAME}.key:True \
    ${UBOOT_SIGN_KEYDIR}/${UBOOT_SIGN_KEYNAME}.crt:True \
"

# The U-Boot hardening's kernel command-line overlay, as the security layer's
# add-secboot-kargs-overlay.inc writes it for Toradex's device-tree-overlays
# recipes; here the kernel builds it from a .dtso.
gen_secboot_kargs_slx_som() {
    dir=${S}/arch/arm/boot/dts/nxp/imx
    cat > $dir/imx6sx-slx-som-secboot-kargs_overlay.dtso <<EOT
/* Automatically generated file - do not edit. */

/dts-v1/;
/plugin/;

&{/} {
	chosen {
		toradex,secure-boot {
			required-bootargs = "${TDX_SECBOOT_REQUIRED_BOOTARGS}";
		};
	};
};
EOT
    row='dtb-$(CONFIG_SOC_IMX6SX) += imx6sx-slx-som-secboot-kargs_overlay.dtbo'
    grep -qxF "$row" $dir/Makefile || echo "$row" >> $dir/Makefile
}
do_patch[postfuncs] += "${@'gen_secboot_kargs_slx_som' if d.getVar('TDX_UBOOT_HARDENING_ENABLE') == '1' else ''}"
do_patch[vardeps] += "TDX_SECBOOT_REQUIRED_BOOTARGS"
