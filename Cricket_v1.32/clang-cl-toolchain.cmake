set(CMAKE_SYSTEM_NAME      Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

set(CMAKE_C_COMPILER   /usr/local/bin/cricket-clang-cl)
set(CMAKE_CXX_COMPILER /usr/local/bin/cricket-clang-cl)
set(CMAKE_LINKER       /usr/bin/lld-link)
set(CMAKE_AR           /usr/local/bin/llvm-lib)
set(CMAKE_RC_COMPILER  /usr/bin/llvm-rc)
set(CMAKE_MT           /usr/bin/llvm-mt)

set(_SDK  /opt/xwin-sdk)
set(_VER  10.0.26100)

set(_BASE "/MT /EHsc /GR /D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH /wd4819 /w")

# -imsvc: system-include semantics — searched AFTER clang's builtin headers,
# so clang's inline intrinsic headers win over MSVC's extern declarations
# (using /I here breaks the SSE intrinsics in inline CRT functions).
set(_INC
    -imsvc${_SDK}/crt/include
    -imsvc${_SDK}/sdk/include/${_VER}/ucrt
    -imsvc${_SDK}/sdk/include/${_VER}/um
    -imsvc${_SDK}/sdk/include/${_VER}/shared
    -imsvc${_SDK}/sdk/include/${_VER}/winrt
)
string(JOIN " " _INC_STR ${_INC})

set(_LIBS
    /libpath:${_SDK}/crt/lib/x86_64
    /libpath:${_SDK}/sdk/lib/ucrt/x86_64
    /libpath:${_SDK}/sdk/lib/um/x86_64
)
string(JOIN " " _LIB_STR ${_LIBS})

set(CMAKE_C_FLAGS_INIT             "${_BASE} ${_INC_STR}")
set(CMAKE_CXX_FLAGS_INIT           "${_BASE} ${_INC_STR}")
set(CMAKE_EXE_LINKER_FLAGS_INIT    "/machine:x64 ${_LIB_STR}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "/machine:x64 ${_LIB_STR}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "/machine:x64 ${_LIB_STR}")

set(CMAKE_RC_FLAGS
    "-I ${_SDK}/sdk/include/${_VER}/um -I ${_SDK}/sdk/include/${_VER}/shared -I ${_SDK}/crt/include -I ${_SDK}/sdk/include/${_VER}/ucrt -I ${_SDK}/sdk/include/um -I ${_SDK}/sdk/include/shared"
    CACHE STRING "" FORCE)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
