DESCRIPTION = "serviceapp service for enigma2"
AUTHOR = "Maroš Ondrášek <mx3ldev@gmail.com>"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263"

DEPENDS = "enigma2 uchardet openssl libsigc++-2.0 glib-2.0 python-pyrex-native"
RDEPENDS_${PN} = "enigma2 uchardet openssl"

# Fork mit DreamOS-Anpassungen als echte Commits (Branch "dreamos"):
# Build-Kompat, compat/-Header, Track-Listen-Lock, sigc statt PSignal1
# (GIL-Korruption aus dem Backend-Thread). Upstream mx3L/serviceapp ist
# seit Jahren eingefroren. Gepinnt -- bei Fork-Aenderungen SRCREV bumpen.
SRC_URI = "git://github.com/Haxe18/serviceapp.git;protocol=https;branch=dreamos;name=plugin"
SRCREV_plugin = "21d849ebf0d5a8eb51a01d1ea0d5e25c11b028c8"
SRCREV = "${SRCREV_plugin}"

S = "${WORKDIR}/git"

inherit autotools gitpkgv pythonnative pkgconfig python-dir

TARGET_CPPFLAGS = "-I${STAGING_INCDIR}/enigma2 \
                   -I${STAGING_INCDIR}/python2.7 \
                   -I${STAGING_INCDIR}/sigc++-2.0 \
                   -I${STAGING_LIBDIR}/sigc++-2.0/include \
                   -I${STAGING_INCDIR}/glib-2.0 \
                   -I${STAGING_LIBDIR}/glib-2.0/include \
                   -I${STAGING_INCDIR}/uchardet"

EXTRA_OECONF = "\
        BUILD_SYS=${BUILD_SYS} \
        HOST_SYS=${HOST_SYS} \
        STAGING_INCDIR=${STAGING_INCDIR} \
        STAGING_LIBDIR=${STAGING_LIBDIR} \
        PYTHON_CPPFLAGS="-I${STAGING_INCDIR}/${PYTHON_DIR}" \
        PYTHON_LDFLAGS="-L${STAGING_LIBDIR} -l${PYTHON_DIR}" \
        "

do_install_append() {
    rm -f ${D}${libdir}/enigma2/python/Plugins/SystemPlugins/ServiceApp/*.pyc
    rm -f ${D}${libdir}/enigma2/python/Plugins/SystemPlugins/ServiceApp/*.la
}

FILES_${PN} = "\
    ${libdir}/enigma2/python/Plugins/SystemPlugins/ServiceApp/*.pyo \
    ${libdir}/enigma2/python/Plugins/SystemPlugins/ServiceApp/*.py \
    ${libdir}/enigma2/python/Plugins/SystemPlugins/ServiceApp/locale/*/LC_MESSAGES/ServiceApp.mo \
    ${libdir}/enigma2/python/Plugins/SystemPlugins/ServiceApp/serviceapp.so"

FILES_${PN}-dev = ""
