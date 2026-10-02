set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

set(XWIN /opt/xwin-sdk)
set(SDKVER 10.0.26100)

set(CMAKE_C_COMPILER   /usr/local/bin/benzene-clang-cl)
set(CMAKE_CXX_COMPILER /usr/local/bin/benzene-clang-cl)
set(CMAKE_RC_COMPILER  /usr/local/bin/llvm-rc)
set(CMAKE_LINKER       /usr/local/bin/lld-link)
set(CMAKE_MT           /usr/local/bin/llvm-mt)
set(CMAKE_AR           /usr/local/bin/llvm-lib)

# Static CRT (dynamic import libs not in the fetched package set)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
# Force lld-link as the linker driver target
set(CMAKE_CXX_USING_LINKER_LLD "lld-link")

set(INCFLAGS "-imsvc ${XWIN}/crt/include -imsvc ${XWIN}/sdk/include/${SDKVER}/ucrt -imsvc ${XWIN}/sdk/include/${SDKVER}/shared -imsvc ${XWIN}/sdk/include/${SDKVER}/um -imsvc ${XWIN}/sdk/include/${SDKVER}/winrt")

set(CMAKE_C_FLAGS_INIT   "${INCFLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${INCFLAGS}")

set(LIBFLAGS "/LIBPATH:${XWIN}/crt/lib/x86_64 /LIBPATH:${XWIN}/sdk/lib/um/x86_64 /LIBPATH:${XWIN}/sdk/lib/ucrt/x86_64")
set(CMAKE_EXE_LINKER_FLAGS_INIT    "${LIBFLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${LIBFLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${LIBFLAGS}")

set(CMAKE_FIND_ROOT_PATH ${XWIN})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
