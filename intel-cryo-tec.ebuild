# Copyright 2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit cmake

DESCRIPTION="Intel Cryo TEC monitoring application and GUI"
HOMEPAGE="https://github.com/yourusername/intel-cryo-tec"
SRC_URI="https://github.com/yourusername/intel-cryo-tec/archive/v${PV}.tar.gz -> ${P}.tar.gz"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE="gui"

DEPEND="
    dev-libs/boost
    dev-cpp/nlohmann_json
    gui? ( dev-qt/qtwidgets:5 )
"
RDEPEND="${DEPEND}"

src_configure() {
    local mycmakeargs=(
        -DCMAKE_BUILD_TYPE=Release
        -DBUILD_GUI=$(usex gui)
    )
    cmake_src_configure
}

src_install() {
    cmake_src_install

    newinitd "${FILESDIR}"/intel-cryo-tec.initd intel-cryo-tec
    newconfd "${FILESDIR}"/intel-cryo-tec.confd intel-cryo-tec

    if use gui; then
        domenu "${FILESDIR}"/intel-cryo-tec-gui.desktop
    fi
}
