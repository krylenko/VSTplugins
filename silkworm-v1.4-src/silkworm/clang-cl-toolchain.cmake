# clang-cl-toolchain.cmake — Windows x86-64 cross-compile from Linux
# Requires: xwin SDK at /opt/xwin-sdk, cricket-clang-cl wrapper

set(CMAKE_SYSTEM_NAME    Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Compilers (wrapper strips CMake's extraneous "--" separator)
set(CMAKE_C_COMPILER     /usr/local/bin/cricket-clang-cl)
set(CMAKE_CXX_COMPILER   /usr/local/bin/cricket-clang-cl)
set(CMAKE_RC_COMPILER    /usr/local/bin/llvm-rc)
set(CMAKE_LINKER         /usr/local/bin/lld-link)
set(CMAKE_MT             /usr/local/bin/llvm-mt)
set(CMAKE_AR             /usr/local/bin/llvm-lib)

# SDK paths
set(XWIN /opt/xwin-sdk)

# Include paths: MSVC CRT headers, then Windows SDK headers
set(IMSVC_FLAGS
    "-imsvc${XWIN}/crt/include"
    "-imsvc${XWIN}/sdk/include/ucrt"
    "-imsvc${XWIN}/sdk/include/um"
    "-imsvc${XWIN}/sdk/include/shared"
)
string(JOIN " " IMSVC_STR ${IMSVC_FLAGS})

set(CMAKE_C_FLAGS_INIT   "${IMSVC_STR}")
set(CMAKE_CXX_FLAGS_INIT "${IMSVC_STR}")

# Library search paths for the linker
set(LINK_DIRS
    "/LIBPATH:${XWIN}/crt/lib/x86_64"
    "/LIBPATH:${XWIN}/sdk/lib/um/x86_64"
    "/LIBPATH:${XWIN}/sdk/lib/ucrt/x86_64"
)
string(JOIN " " LINK_STR ${LINK_DIRS})

set(CMAKE_EXE_LINKER_FLAGS_INIT    "${LINK_STR}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${LINK_STR}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${LINK_STR}")

# Prevent CMake from finding Linux system libs
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
