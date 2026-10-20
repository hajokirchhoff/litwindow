# Real, publishable registry port: fetches a tagged GitHub release of this
# very repository. This is what consumers (e.g. via a custom vcpkg git
# registry pointed at this repo) actually use.
#
# For local development, see vcpkg-overlay/ports/litwindow instead, which
# builds directly from the working checkout.

vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO hajokirchhoff/litwindow
	REF "v${VERSION}"
	SHA512 ff3c5adb7956d342a8f9dcce8aea87c112a31a8eaa161b898e559b4890080b1b358032ac1f2db75598ac3e8f31311eadcb82db7437fe364a5f81fad3f0216638
	HEAD_REF main
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
	FEATURES
		odbc LITWINDOW_BUILD_ODBC
		wx   LITWINDOW_BUILD_LWWX
)

vcpkg_cmake_configure(
	SOURCE_PATH "${SOURCE_PATH}"
	OPTIONS
		${FEATURE_OPTIONS}
		-DBUILD_TESTING=OFF
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME litwindow CONFIG_PATH lib/cmake/litwindow)
vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")
