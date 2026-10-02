# Windows x86-64 cross toolchain: clang-cl 18 + lld-link + xwin SDK
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 10.0)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

set(XWIN "/opt/xwin-sdk" CACHE PATH "xwin splat output")

set(CMAKE_C_COMPILER   /usr/local/bin/cricket-clang-cl)
set(CMAKE_CXX_COMPILER /usr/local/bin/cricket-clang-cl)
set(CMAKE_LINKER       /usr/local/bin/lld-link)
set(CMAKE_AR           /usr/local/bin/llvm-lib)
set(CMAKE_RC_COMPILER  /usr/bin/llvm-rc-18)
set(CMAKE_MT           /usr/local/bin/llvm-mt)
set(CMAKE_C_COMPILER_TARGET   x86_64-pc-windows-msvc)
set(CMAKE_CXX_COMPILER_TARGET x86_64-pc-windows-msvc)

set(_inc "/imsvc ${XWIN}/crt/include /imsvc ${XWIN}/sdk/include/ucrt /imsvc ${XWIN}/sdk/include/um /imsvc ${XWIN}/sdk/include/shared")
set(CMAKE_C_FLAGS_INIT   "${_inc}")
set(CMAKE_CXX_FLAGS_INIT "${_inc} /EHsc")
set(CMAKE_RC_FLAGS_INIT  "-I${XWIN}/sdk/include/um -I${XWIN}/sdk/include/shared -I${XWIN}/crt/include -I${XWIN}/sdk/include/ucrt")

set(_lib "/libpath:${XWIN}/crt/lib/x86_64 /libpath:${XWIN}/sdk/lib/um/x86_64 /libpath:${XWIN}/sdk/lib/ucrt/x86_64")
set(CMAKE_EXE_LINKER_FLAGS_INIT    "${_lib}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${_lib}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${_lib}")

# Static CRT: no VC++ redistributable needed on the target machine
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
