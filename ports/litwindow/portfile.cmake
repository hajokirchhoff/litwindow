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
	SHA512 ba7722f82d32e51489cda78675d8788df0399672104430a2401ad1c3e5ee2597120e68356e6d0b676f6f6fad48681f2adc2829f617d3e9882e357a1d246059bc
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
