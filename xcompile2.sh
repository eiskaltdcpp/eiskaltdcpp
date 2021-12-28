#!/bin/bash
#
# @ref https://community.kde.org/Android/Environment_via_Container 
# @ref https://www.proli.net/2018/06/22/kde-on-android-ci-cd-sdk/

# This container dockerfile:
# https://phabricator.kde.org/source/sysadmin-ci-tooling/browse/master/system-images/android/sdk/Dockerfile
# ""inspired by rabits/qt which we use for the gcc toolkit""

#
# Possible examples
# - okular
# - marble - https://community.kde.org/Android

set -eu

recreate_dir() {
	if [[ -d $1 ]] ; then
		rm -fr "$1"
	fi
	mkdir "$1"
}

main() {

	if [[ $1 == "make-deps" ]] ; then
		./xcompile2.sh make-bzip2
		./xcompile2.sh make-openssl
		#./xcompile2.sh make-gettext
		./xcompile2.sh make-pcre
		./xcompile2.sh make-idn
		
	elif [[ $1 == "make-apt-deps" ]] ; then
		# this container is overall quite good but is missing patch
		sudo apt update
		sudo apt install patch

	elif [[ $1 == "make-bzip2" ]] ; then
		# n.b. should run under docker
		recreate_dir ./build-bzip2
		cd build-bzip2
		git clone git://sourceware.org/git/bzip2.git
		cd bzip2
		
		# @ref https://developer.android.com/ndk/guides/other_build_systems
		export LDFLAGS='-static-libstdc++'
		make \
			CC=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi29-clang \
			AR=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar \
			RANLIB=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ranlib \
			bzip2
			
	elif [[ $1 == "make-openssl" ]] ; then
		recreate_dir ./build-openssl
		cd build-openssl
		git clone https://github.com/openssl/openssl.git
		cd openssl
		
		# Bionic libc does not allow atexit()
		patch -p1 < ../../openssl-no-atexit.patch
		
		# @ref https://proandroiddev.com/tutorial-compile-openssl-to-1-1-1-for-android-application-87137968fee
		export PATH=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin:$ANDROID_NDK_ROOT/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin:$PATH
		export LDFLAGS='-static-libstdc++'
		./Configure android-arm -D__ANDROID_API__=29
		make
		
	elif [[ $1 == "make-gettext" ]] ; then
		recreate_dir ./build-gettext
		cd build-gettext
		git clone https://github.com/autotools-mirror/gettext # https://git.savannah.gnu.org/git/gettext.git
		# https://ftp.gnu.org/gnu/gettext/gettext-0.21.tar.gz
		cd gettext
		
		# @ref https://github.com/alexa/avs-device-sdk/issues/825#issuecomment-407002856 
		mkdir output-prefix
		export PATH=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin:$ANDROID_NDK_ROOT/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin:$PATH
		export SYSROOT=$ANDROID_NDK_ROOT/sysroot
		export LDFLAGS='-static-libstdc++'
		#autoreconf -i
		./configure --prefix="$(cd output-prefix ; pwd)" #--host=arm-linux-androideabi
		make

	elif [[ $1 == "make-pcre" ]] ; then
		recreate_dir ./build-pcre
		cd build-pcre
		git clone https://github.com/PhilipHazel/pcre2.git
		cd pcre2
		
		mkdir build
		cd build
		export PATH=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin:$ANDROID_NDK_ROOT/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin:$PATH
		export CC=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi29-clang
		export LD=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/ld
		export LDFLAGS='-static-libstdc++'
		cmake .. 
		make
		
	elif [[ $1 == "make-idn" ]] ; then
		recreate_dir ./build-idn
		cd build-idn
		curl https://ftp.gnu.org/gnu/libidn/libidn2-latest.tar.gz > libidn2-latest.tar.gz
		tar xaf libidn2-latest.tar.gz
		mv libidn2-2* libidn2-latest
		cd libidn2-latest
		
		# @ref https://developer.android.com/ndk/guides/other_build_systems
		export PATH=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin:$ANDROID_NDK_ROOT/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin:$PATH
		export CC=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi29-clang
		export LD=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/ld
		export AR=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar
		export RANLIB=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ranlib
		export CFLAGS='-Wno-format-nonliteral -Wno-overlength-strings'
		export LDFLAGS='-static-libstdc++'
		./configure --host aarch64-linux-android
		make
		
	elif [[ $1 == "make-in-docker" ]] ; then

		sudo docker run -it --rm --volume="${PWD}:/home/user/project" rabits/qt:5.15-android \
			/bin/bash -c "cd /home/user/project && ./xcompile2.sh make-native" ;

	elif [[ $1 == "make-native" ]] ; then

		recreate_dir ./build
		cd build

		cmake .. -G Ninja \
			-DCMAKE_BUILD_TYPE:STRING=Release \
			-DANDROID_ABI:STRING=armeabi-v7a \
			-DANDROID_BUILD_ABI_armeabi-v7a:BOOL=ON \
			-DANDROID_BUILD_ABI_arm64-v8a:BOOL=OFF \
			-DANDROID_BUILD_ABI_x86:BOOL=OFF \
			-DANDROID_BUILD_ABI_x86_64:BOOL=OFF \
			"-DANDROID_NATIVE_API_LEVEL:STRING=${ANDROID_NATIVE_API_LEVEL}" \
			"-DANDROID_SDK:PATH=${ANDROID_SDK_ROOT}" \
			"-DANDROID_NDK:PATH=${ANDROID_NDK_ROOT}" \
			"-DCMAKE_PREFIX_PATH:PATH=${QT_ANDROID}" \
			"-DCMAKE_FIND_ROOT_PATH:STRING=${QT_ANDROID}" \
			"-DBZIP2_LIBRARIES:STRING=/home/user/project/build-bzip2/bzip2/libbz2.a" \
			"-DBZIP2_INCLUDE_DIR:PATH=/home/user/project/build-bzip2/bzip2/" \
			"-DQt5_LRELEASE_EXECUTABLE:STRING=/opt/Qt/5.15.2/android/bin/lrelease" \
			-DOPENSSL_CRYPTO_LIBRARY:PATH=/home/user/project/build-openssl/openssl/libcrypto.a \
			-DOPENSSL_SSL_LIBRARY:PATH=/home/user/project/build-openssl/openssl/libssl.a \
			-DOPENSSL_INCLUDE_DIR:PATH=/home/user/project/build-openssl/openssl/include \
			-DIDN2_LIBRARY:PATH=/home/user/project/build-idn/libidn2-latest/lib/.libs/libidn2.a \
			-DIDN2_INCLUDE_DIR:PATH==/home/user/project/build-idn/libidn2-latest/lib/ \
			"-DPCRE_LIBRARIES:STRING=/home/user/project/build-pcre/pcre2/build/libpcre2-8.a" \
			"-DPCRE_INCLUDE_DIR:STRING=/home/user/project/build-pcre/pcre2/build/" \
			"-DCMAKE_TOOLCHAIN_FILE:PATH=${ANDROID_NDK_ROOT}/build/cmake/android.toolchain.cmake"
			
		# Prevents linking libeiskalt .so because some dependency isn't doing this as well
		sed -i 's/-static-libstdc++//' ./build.ninja
		
		# Disable debug info
		sed -i 's/ -g / -g0 /' ./build.ninja
		sed -i 's/ -O2 / -Os /' ./build.ninja
		cmake --build . --config Release
		
		# just need to get libc.so linked dynamically instead of statically
		
		# /opt/android-sdk/ndk/21.0.6113669/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++ --target=armv7-none-linux-androideabi29 --gcc-toolchain=/opt/android-sdk/ndk/21.0.6113669/toolchains/llvm/prebuilt/linux-x86_64 --sysroot=/opt/android-sdk/ndk/21.0.6113669/toolchains/llvm/prebuilt/linux-x86_64/sysroot -fPIC -g0 -DANDROID -fdata-sections -ffunction-sections -funwind-tables -fstack-protector-strong -no-canonical-prefixes -D_FORTIFY_SOURCE=2 -march=armv7-a -mthumb -Wformat -Werror=format-security   -std=c++14 -pipe -Wformat -Werror=format-security -fPIC -D_FORTIFY_SOURCE=2 -Os -g0 -Oz -DNDEBUG  -Wl,--exclude-libs,libgcc_real.a -Wl,--exclude-libs,libatomic.a  -Wl,--build-id -Wl,--fatal-warnings -Wl,--exclude-libs,libunwind.a -Wl,--no-undefined -Qunused-arguments  -Wl,--gc-sections -Wl,--strip-all -Wl,--wrap=atexit -shared -Wl,-soname,libeiskaltdcpp-qt_armeabi-v7a.so -o android-build/libs/armeabi-v7a/libeiskaltdcpp-qt_armeabi-v7a.so eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/main.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_ADLS.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_ADLSModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_ActionCustomizer.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_AntiSpamFrame.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_Antispam.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_ArenaWidget.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_ArenaWidgetFactory.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_ArenaWidgetManager.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_AutoToolTip.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_ChatEdit.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_CmdDebug.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_CustomFontModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_DebugHelper.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_DownloadQueue.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_DownloadQueueModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_DownloadToHistory.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_EiskaltApp.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_EmoticonDialog.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_EmoticonFactory.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_EmoticonObject.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_FavoriteHubModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_FavoriteHubs.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_FavoriteUsers.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_FavoriteUsersModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_FileBrowserModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_FileHasher.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_FinishedTransfers.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_FinishedTransfersModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_FlowLayout.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_GlobalTimer.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_HashProgress.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_HistoryInterface.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_HubFrame.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_HubManager.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_IPFilterFrame.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_IPFilterModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_LineEdit.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_Magnet.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_MainWindow.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_MultiLineToolBar.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_Notification.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_PMWindow.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_PoolAlloc.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_PoolItem.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_PublicHubModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_PublicHubs.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_PublicHubsList.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_QueuedUsers.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_QuickConnect.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_SearchBlacklist.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_SearchBlacklistDialog.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_SearchFrame.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_SearchModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_Secretary.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_Settings.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_SettingsAdvanced.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_SettingsConnection.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_SettingsDownloads.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_SettingsGUI.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_SettingsHistory.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_SettingsInterface.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_SettingsLog.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_SettingsNotification.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_SettingsPersonal.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_SettingsSharing.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_SettingsShortcuts.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_SettingsUC.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_ShareBrowser.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_ShareBrowserSearch.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_ShellCommandRunner.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_ShortcutEdit.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_ShortcutGetter.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_ShortcutManager.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_SideBar.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_SpyFrame.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_SpyModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_TabButton.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_TabFrame.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_ToolBar.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_TransferView.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_TransferViewModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_UCModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_UserListModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_WulforSettings.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/moc_WulforUtil.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/qtsingleapp/moc_qtsinglecoreapplication.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/ADLS.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/ADLSModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/ActionCustomizer.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/AntiSpamFrame.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/Antispam.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/ArenaWidget.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/ArenaWidgetManager.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/AutoToolTip.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/ChatEdit.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/CmdDebug.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/CustomFontModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/DebugHelper.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/DownloadQueue.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/DownloadQueueModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/EmoticonDialog.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/EmoticonFactory.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/FavoriteHubModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/FavoriteHubs.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/FavoriteUsers.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/FavoriteUsersModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/FileBrowserModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/FileHasher.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/FinishedTransfers.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/FinishedTransfersModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/FlowLayout.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/GlobalTimer.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/HashProgress.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/HubFrame.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/HubManager.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/IPFilterFrame.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/IPFilterModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/LineEdit.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/Magnet.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/MainWindow.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/MultiLineToolBar.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/Notification.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/PMWindow.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/PublicHubModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/PublicHubs.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/PublicHubsList.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/QueuedUsers.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/QuickConnect.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/SearchBlacklist.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/SearchBlacklistDialog.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/SearchFrame.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/SearchModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/Secretary.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/Settings.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/SettingsAdvanced.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/SettingsConnection.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/SettingsDownloads.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/SettingsGUI.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/SettingsHistory.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/SettingsLog.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/SettingsNotification.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/SettingsPersonal.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/SettingsSharing.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/SettingsShortcuts.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/SettingsUC.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/ShareBrowser.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/ShareBrowserSearch.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/ShellCommandRunner.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/ShortcutEdit.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/ShortcutGetter.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/ShortcutManager.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/SideBar.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/SpyFrame.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/SpyModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/TabButton.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/TabFrame.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/ToolBar.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/TransferView.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/TransferViewModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/UCModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/UserListModel.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/WulforSettings.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/WulforUtil.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/atexit.cpp.o eiskaltdcpp-qt/CMakeFiles/eiskaltdcpp-qt.dir/src/qtsingleapp/qtsinglecoreapplication.cpp.o  /opt/Qt/5.15.2/android/lib/libQt5Widgets_armeabi-v7a.so  /opt/Qt/5.15.2/android/lib/libQt5Xml_armeabi-v7a.so  /opt/Qt/5.15.2/android/lib/libQt5Multimedia_armeabi-v7a.so  /opt/Qt/5.15.2/android/lib/libQt5Concurrent_armeabi-v7a.so  /opt/Qt/5.15.2/android/lib/libQt5Network_armeabi-v7a.so extra/libextra.a  dcpp/libeiskaltdcpp.a /opt/Qt/5.15.2/android/lib/libQt5Gui_armeabi-v7a.so  /opt/Qt/5.15.2/android/lib/libQt5Core_armeabi-v7a.so  extra/libextra.a  dht/libdht.a  ../build-bzip2/bzip2/libbz2.a ../build-openssl/openssl/libssl.a ../build-openssl/openssl/libcrypto.a -lc -Wl,-Bstatic -latomic -lm -lz
		
		
		cmake --build . --config Release -t apk || (
			
			# The wrong build-tools version is loaded from somewhere
			# But it needs to be generated first
			sed -i 's/28.0.3/29.0.3/' ./android-build/build.gradle
			
			cmake --build . --config Release -t apk
		)

	else
		echo "unknown '$1'" >&2
		exit 1
	fi

}

main "$@"
