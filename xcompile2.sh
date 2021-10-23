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

	elif [[ $1 == "make-bzip2" ]] ; then
		# n.b. should run under docker
		recreate_dir ./build-bzip2
		cd build-bzip2
		git clone git://sourceware.org/git/bzip2.git
		cd bzip2
		
		# @ref https://developer.android.com/ndk/guides/other_build_systems
		export LDFLAGS='-static-libstdc++'
		make \
			CC=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang \
			AR=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar \
			RANLIB=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ranlib \
			bzip2
			
	elif [[ $1 == "make-openssl" ]] ; then
		recreate_dir ./build-openssl
		cd build-openssl
		git clone https://github.com/openssl/openssl.git
		cd openssl
		
		# @ref https://proandroiddev.com/tutorial-compile-openssl-to-1-1-1-for-android-application-87137968fee
		export PATH=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin:$ANDROID_NDK_ROOT/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin:$PATH
		export LDFLAGS='-static-libstdc++'
		./Configure android-arm64 -D__ANDROID_API__=29
		make
		
	elif [[ $1 == "make-gettext" ]] ; then
		recreate_dir ./build-gettext
		cd build-gettext
		git clone https://github.com/autotools-mirror/gettext # https://git.savannah.gnu.org/git/gettext.git
		# https://ftp.gnu.org/gnu/gettext/gettext-0.21.tar.gz
		cd gettext
		
		# @ref https://github.com/alexa/avs-device-sdk/issues/825#issuecomment-407002856 
		mkdir output-prefix
		export PATH=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin:$ANDROID_NDK_ROOT/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin:$PATH
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
		export PATH=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin:$ANDROID_NDK_ROOT/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin:$PATH
		export CC=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang
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
		export PATH=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin:$ANDROID_NDK_ROOT/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin:$PATH
		export CC=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang
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
			-DANDROID_ABI:STRING=arm64-v8a \
			-DANDROID_BUILD_ABI_armeabi-v7a=OFF \
			-DANDROID_BUILD_ABI_arm64-v8a:BOOL=ON \
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
			
		cmake --build .

		cmake --build . -t apk || (
			
			# The wrong build-tools version is loaded from somewhere
			# But it needs to be generated first
			sed -i 's/28.0.3/29.0.3/' ./android-build/build.gradle
			
			cmake --build . -t apk
		)

	else
		echo "unknown '$1'" >&2
		exit 1
	fi

}

main "$@"
