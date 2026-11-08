# imx8mp-navqp builds voxelbotics' U-Boot 2023.04; every other machine keeps meta-imx's.
require ${@'recipes-bsp/u-boot/u-boot-imx-navqp.inc' if d.getVar('MACHINE') == 'imx8mp-navqp' else ''}
