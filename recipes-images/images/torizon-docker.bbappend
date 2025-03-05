IMAGE_ROOTFS_MAXSIZE = "2650000"

# NOTE: ignore OTA bootloader until the recipe is implemented
UBOOT_BINARY_OTA_IGNORE:imx8mp-navqp = "1"

IMAGE_FSTYPES:append:imx8mp-navqp = " teziimg"
IMAGE_FSTYPES:append:imx8mp-navqp = " wic"
TEZI_USE_BOOTFILES:imx8mp-navqp = "false"
UBOOT_BINARY_TEZI_EMMC:stm32mp25-eval = "imx-boot"
OFFSET_BOOTROM_PAYLOAD:imx8mp-navqp = "0"
TORADEX_PRODUCT_IDS:imx8mp-navqp = "0100"
UBOOT_ENV_TEZI_EMMC:imx8mp-navqp = ""

KERNEL_DEVICETREE_BASENAME:imx8mp-navqp = "imx8mp-navq"

CORE_IMAGE_BASE_INSTALL:append = " \
    e2fsprogs-resize2fs \
"
