require conf/machine/include/common-imx6.inc

# Deploy the uuu script that installs U-Boot in the NAND.
EXTRA_IMAGEDEPENDS:append = " imx6sx-slx-som-uuu"
