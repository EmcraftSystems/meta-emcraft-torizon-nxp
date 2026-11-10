# /etc is volatile in a signed build of imx6sx-slx-som, so the first-login
# expiry of the torizon password would recur at every boot.
pkg_postinst_ontarget:${PN}:imx6sx-slx-som:cfs-signed = ""

# The password of the user torizon. Torizon sets it as a fixed hash of
# "torizon" in USERADD_PARAM. A build directory sets its own with
# EMCRAFT_TORIZON_PASSWORD_HASH in conf/local.conf, a hash from
# "openssl passwd -6"; unset, Torizon's stands.
EMCRAFT_TORIZON_PASSWORD_HASH ??= ""
EMCRAFT_TORIZON_DEFAULT_PASSWORD_ARG = " -p GudJRR5U.mLp2 "

python () {
    h = d.getVar('EMCRAFT_TORIZON_PASSWORD_HASH')
    if not h:
        return
    if "'" in h or not h.startswith('$'):
        bb.fatal('EMCRAFT_TORIZON_PASSWORD_HASH is not a crypt hash: %s' % h)
    v = 'USERADD_PARAM:' + d.getVar('PN')
    p = d.getVar(v)
    old = d.getVar('EMCRAFT_TORIZON_DEFAULT_PASSWORD_ARG')
    if old not in p:
        bb.fatal('%s no longer holds "%s"; check torizon-users' % (v, old.strip()))
    # useradd.bbclass puts the value in a double-quoted shell string, so each
    # $ of the hash is escaped; the quotes keep it whole through its eval.
    d.setVar(v, p.replace(old, " -p '%s' " % h.replace('$', '\\$')))
}
