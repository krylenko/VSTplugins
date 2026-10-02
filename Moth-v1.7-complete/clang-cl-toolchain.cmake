# clang-cl-toolchain.cmake — cross-compile Windows x86-64 from Linux
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

set(XWIN /opt/xwin-sdk)

set(CMAKE_C_COMPILER   /usr/local/bin/cricket-clang-cl)
set(CMAKE_CXX_COMPILER /usr/local/bin/cricket-clang-cl)
set(CMAKE_RC_COMPILER  /usr/bin/llvm-rc-18)
set(CMAKE_LINKER       /usr/local/bin/lld-link)
set(CMAKE_MT           /usr/local/bin/llvm-mt)
set(CMAKE_AR           /usr/local/bin/llvm-lib)

set(CMAKE_C_COMPILER_TARGET   x86_64-pc-windows-msvc)
set(CMAKE_CXX_COMPILER_TARGET x86_64-pc-windows-msvc)

# Include paths: MSVC CRT + Windows SDK (um, shared, ucrt)
set(SDK_INC "${XWIN}/sdk/include/10.0.26100")
set(INCFLAGS "-imsvc ${XWIN}/crt/include -imsvc ${SDK_INC}/ucrt -imsvc ${SDK_INC}/um -imsvc ${SDK_INC}/shared -imsvc ${SDK_INC}/winrt")

set(CMAKE_C_FLAGS_INIT   "${INCFLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${INCFLAGS}")

# Library search paths
set(LIBFLAGS "/LIBPATH:${XWIN}/crt/lib/x86_64 /LIBPATH:${SDK_INC_LIB}/ucrt/x86_64 /LIBPATH:${XWIN}/sdk/lib/ucrt/x86_64 /LIBPATH:${XWIN}/sdk/lib/um/x86_64")
set(CMAKE_EXE_LINKER_FLAGS_INIT    "${LIBFLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${LIBFLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${LIBFLAGS}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
