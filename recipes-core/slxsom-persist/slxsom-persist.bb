SUMMARY = "Settings of the SLX-SOM that survive a reboot of a signed image"
DESCRIPTION = "On a signed image /etc is reset at every boot. This keeps \
under /var/rootdirs/etc the flag that turns on device monitoring once the \
device is provisioned, and the SSH host keys."
HOMEPAGE = "https://github.com/EmcraftSystems/meta-emcraft-torizon-nxp"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://fluent-bit-persist.conf \
    file://slxsom-fluent-bit-save.path \
    file://slxsom-fluent-bit-save.service \
    file://slxsom-persist.conf \
    file://50-slxsom-hostkeys.conf \
"

S = "${WORKDIR}"

inherit systemd allarch

SYSTEMD_SERVICE:${PN} = "slxsom-fluent-bit-save.path"

do_install() {
    install -d ${D}${systemd_system_unitdir}/fluent-bit.service.d
    install -m 0644 ${WORKDIR}/fluent-bit-persist.conf \
        ${D}${systemd_system_unitdir}/fluent-bit.service.d/50-persist.conf
    for u in slxsom-fluent-bit-save.path slxsom-fluent-bit-save.service; do
        install -m 0644 ${WORKDIR}/$u ${D}${systemd_system_unitdir}/
    done

    install -d ${D}${nonarch_libdir}/tmpfiles.d
    install -m 0644 ${WORKDIR}/slxsom-persist.conf ${D}${nonarch_libdir}/tmpfiles.d/

    install -d ${D}${sysconfdir}/ssh/sshd_config.d
    install -m 0644 ${WORKDIR}/50-slxsom-hostkeys.conf ${D}${sysconfdir}/ssh/sshd_config.d/
}

FILES:${PN} += " \
    ${systemd_system_unitdir} \
    ${nonarch_libdir}/tmpfiles.d/slxsom-persist.conf \
    ${sysconfdir}/ssh/sshd_config.d/50-slxsom-hostkeys.conf \
"
