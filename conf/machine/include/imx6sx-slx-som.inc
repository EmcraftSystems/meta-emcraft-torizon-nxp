require conf/machine/include/common-imx6.inc

# U-Boot is in the NAND: the card carries the root file system alone, at 8 MiB.
WKS_FILE:sota = "imx6sx-slx-som-sota.wks"

# Deploy the uuu script that installs U-Boot in the NAND.
EXTRA_IMAGEDEPENDS:append = " imx6sx-slx-som-uuu"

# A signed image resets /etc at every boot; slxsom-persist keeps what this
# board's functions write there.
MACHINE_EXTRA_RRECOMMENDS:append:cfs-signed = " slxsom-persist"
