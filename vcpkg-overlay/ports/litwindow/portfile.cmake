# NOTE: This is a *local/dev* overlay port, meant for testing litwindow's
# vcpkg consumption story (feature wiring, install/export rules, find_package
# support) without publishing anything. Instead of fetching a release archive
# via vcpkg_from_github(), it builds directly from this checkout.
#
# To turn this into a real, publishable port (e.g. for vcpkg-registries or a
# PR to the vcpkg community registry), replace the SOURCE_PATH line below
# with a vcpkg_from_github()/vcpkg_from_git() call that fetches a tagged
# release, and add a SHA512.

set(SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../..")

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
