require conf/machine/include/common-imx6.inc

# U-Boot is in the NAND: the card carries the root file system alone, and its
# first 8 MiB, where U-Boot keeps its environment, are not written.
WKS_FILE:sota = "imx6sx-slx-som-sota.wks"

# Deploy the uuu script that installs U-Boot in the NAND.
EXTRA_IMAGEDEPENDS:append = " imx6sx-slx-som-uuu"
