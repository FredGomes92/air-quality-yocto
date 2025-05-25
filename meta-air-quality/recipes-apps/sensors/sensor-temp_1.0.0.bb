SUMMARY = "C++ MQTT Temperature Publisher"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
SRC_URI = "file://sensor-temp.cpp file://sensor-temp.service file://am2330_read.c file://sensor-temp.timer"

S = "${WORKDIR}/sources"
UNPACKDIR = "${S}"

DEPENDS = "paho-mqtt-cpp"
LICENSE = "MIT"

do_compile() {
    ${CXX} ${LDFLAGS} sensor-temp.cpp -o sensor-temp \
      -lpaho-mqttpp3 -lpaho-mqtt3as

    ${CC} ${LDFLAGS} ${CFLAGS} am2330_read.c -o am2330_read
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 sensor-temp ${D}${bindir}/sensor-temp
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 sensor-temp.service ${D}${systemd_system_unitdir}/sensor-temp.service

    install -m 0644 sensor-temp.timer ${D}${systemd_system_unitdir}/sensor-temp.timer

    # Install the am2330_read binary
    install -d ${D}${bindir}
    install -m 0755 am2330_read ${D}${bindir}/am2330_read
}

inherit systemd

SYSTEMD_SERVICE:${PN} = "sensor-temp.timer"
SYSTEMD_AUTO_ENABLE = "enable"
FILES:${PN} += "${systemd_system_unitdir}/sensor-temp.service"
FILES:${PN} += "${systemd_system_unitdir}/sensor-temp.timer"


