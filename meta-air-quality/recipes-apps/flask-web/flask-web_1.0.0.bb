DESCRIPTION = "Flask Web App"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
SRC_URI = "file://web-app.service \
           file://app.py \
           file://requirements.txt \
           file://templates/index.html \
           file://templates/chart.html"

S = "${WORKDIR}/sources"
UNPACKDIR = "${S}"

RDEPENDS:${PN} += "python3 python3-flask python3-paho-mqtt python3-flask-socketio python3-eventlet"



do_install() {
    install -d ${D}/opt/flask-web/templates
    install -m 0755 app.py ${D}/opt/flask-web/
    install -m 0644 requirements.txt ${D}/opt/flask-web/
    install -m 0644 templates/index.html ${D}/opt/flask-web/templates/
    install -m 0644 templates/chart.html ${D}/opt/flask-web/templates/

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 web-app.service ${D}${systemd_system_unitdir}/web-app.service
}

FILES:${PN} += "/opt/flask-web"

inherit systemd
SYSTEMD_SERVICE:${PN} = "web-app.service"
SYSTEMD_AUTO_ENABLE = "enable"

FILES:${PN} += "${systemd_system_unitdir}/web-app.service"





