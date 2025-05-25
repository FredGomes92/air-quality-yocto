SUMMARY = "C++ MQTT PMS5003 Publisher"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
SRC_URI = "file://sensor-pms.cpp file://sensor-pms.service file://sensor-pms.timer"

S = "${WORKDIR}/sources"
UNPACKDIR = "${S}"

DEPENDS += "paho-mqtt-cpp \
            pms5003 \
            "
do_compile() {
    echo "Statiging lib dir ${STAGING_LIBDIR}"
    ${CXX} ${LDFLAGS} sensor-pms.cpp -o sensor-pms \
        -I${STAGING_INCDIR} \
        -L${STAGING_LIBDIR} \
        -lpaho-mqttpp3 -lpaho-mqtt3as -lpms5003

    # ${CC} ${LDFLAGS} ${CFLAGS} pms_read.c -o pms_read_test
}
EXTRA_OECONF += ""


do_install() {
    install -d ${D}${bindir}
    install -m 0755 sensor-pms ${D}${bindir}/sensor-pms
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 sensor-pms.service ${D}${systemd_system_unitdir}/sensor-pms.service
    install -m 0644 sensor-pms.timer ${D}${systemd_system_unitdir}/sensor-pms.timer

}

inherit systemd

SYSTEMD_SERVICE:${PN} = "sensor-pms.timer"
SYSTEMD_AUTO_ENABLE = "enable"
FILES:${PN} += "${systemd_system_unitdir}/sensor-pms.service"
FILES:${PN} += "${systemd_system_unitdir}/sensor-pms.timer"
