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
	SHA512 07816923c4d3ad3d20730393b8acf0a286ef7b1337552353a60071d588cc606821463ad489a222e49d803d5471903f03d157dc2042aea283a9611ad0821ab02e
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
