# Extend file search path
FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

# Add your template file to the source
SRC_URI += "file://wpa_supplicant.conf.in"
# SRC_URI:remove = "file://0001-macsec_linux-Hardware-offload-requires-Linux-headers.patch"


# S = "${WORKDIR}/sources"
# UNPACKDIR = "${S}"

# Defaults if not overridden in local.conf or elsewhere
WIFI_SSID ?= "default_ssid"
WIFI_PSK ?= "default_password"

do_configure:append() {
    echo "Checking for wpa_supplicant.conf.in in ${WORKDIR}"
    ls -l ${WORKDIR}/sources-unpack
    if [ ! -f ${WORKDIR}/sources-unpack/wpa_supplicant.conf.in ]; then
        echo "ERROR: wpa_supplicant.conf.in is missing!"
        exit 1
    fi

    sed -e "s|@SSID@|${WIFI_SSID}|" \
        -e "s|@PSK@|${WIFI_PSK}|" \
        ${WORKDIR}/sources-unpack/wpa_supplicant.conf.in > ${WORKDIR}/wpa_supplicant.conf
}

do_install:append() {
    install -d ${D}${sysconfdir}
    install -m 0600 ${WORKDIR}/wpa_supplicant.conf ${D}${sysconfdir}/wpa_supplicant.conf
}
