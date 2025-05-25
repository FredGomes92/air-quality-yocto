# Recipe created by recipetool
# This is the basis of a recipe and may need further editing in order to be fully functional.
# (Feel free to remove these comments when editing.)

# Unable to find any files that looked like license statements. Check the accompanying
# documentation and source headers and set LICENSE and LIC_FILES_CHKSUM accordingly.
#
# NOTE: LICENSE is being set to "CLOSED" to allow you to at least start building - if
# this is not accurate with respect to the licensing of the software being built (it
# will not be in most cases) you must specify the correct value before using this
# recipe for anything other than initial testing/development!
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

SRC_URI = "git://git@github.com/FredGomes92/pms5003.git;protocol=ssh;branch=master"

# Modify these as desired
PV = "1.0+git"
SRCREV = "bb830f5b884e118cca711bdfc6c7a5241088ee53"

S = "${WORKDIR}/git"

inherit cmake

SUMMARY = "pms5003 PM sensor C++ library and example"
# LICENSE = "MIT"
# LIC_FILES_CHKSUM = "file://LICENSE;md5=5d2484c4e9e3fbc7b87893827f572c88"

SRC_URI = "git://github.com/FredGomes92/pms5003.git;protocol=https;branch=master"
PV = "1.0+git"

S = "${WORKDIR}/git"
B = "${WORKDIR}/build"

inherit cmake


# do_compile() {
#     cmake ${S} -B ${B}
#     cmake --build ${B}
# }

# do_install() {
#     install -d ${D}${bindir}
#     install -m 0755 ${B}/pms_read ${D}${bindir}

#     install -d ${D}${libdir}
#     install -m 0755 ${B}/libpms5003.so ${D}${libdir}

#     # Add the symlink for the linker to find -lpms5003
#     # ln -sf libpms5003.so ${D}${libdir}/libpms5003.so

#     install -d ${D}${includedir}
#     install -m 0644 ${S}/pms5003.h ${D}${includedir}/pms5003.h
# }

# Ensure proper packaging
FILES:${PN} += "${bindir}/pms_read ${libdir}/libpms5003.so.*"
FILES:${PN}-dev += "${includedir}/pms5003.h ${libdir}/libpms5003.so"

