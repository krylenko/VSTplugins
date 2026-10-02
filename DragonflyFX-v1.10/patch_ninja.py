#!/usr/bin/env python3
"""Post-configure ninja patches for clang-cl cross builds (run after every cmake configure)."""
import os, re, sys
bdir = sys.argv[1]; tc = os.path.abspath(sys.argv[2]); x = "/opt/xwin-sdk"
r = os.path.join(bdir, "CMakeFiles/rules.ninja")
s = open(r).read(); n = s.count(" -c -- $in"); open(r, "w").write(s.replace(" -c -- $in", " -c $in")); print(f"rules.ninja: {n} '-c --' fixes")
b = os.path.join(bdir, "build.ninja"); s = open(b).read()
inc = f"-I{x}/sdk/include/um -I{x}/sdk/include/shared -I{x}/crt/include -I{x}/sdk/include/ucrt"
patches = [
  ("-DCMAKE_RC_COMPILER=/usr/bin/llvm-rc-18 && /usr/bin/cmake --build",
   f"-DCMAKE_RC_COMPILER=/usr/bin/llvm-rc-18 -DCMAKE_LINKER=/usr/local/bin/lld-link -DCMAKE_MT=/usr/local/bin/llvm-mt -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE={tc} && /usr/bin/cmake --build"),
  ("  OBJECT_DIR = CMakeFiles/DragonflyFX_rc_lib.dir", f"  INCLUDES = {inc}\n  OBJECT_DIR = CMakeFiles/DragonflyFX_rc_lib.dir"),
]
for old, new in patches:
    if new in s: print("already:", old[:40])
    elif old in s: s = s.replace(old, new); print("OK:", old[:40])
    else: sys.exit("MISS: " + old[:60])
# run the manifest helper under Wine
s2, k = re.subn(r"(&& )(\S*vst3_helper\.exe >)", r"\1DISPLAY= WINEDEBUG=-all wine \2", s) if "wine " not in s else (s, 0)
print(f"wine prefix: {k} site(s)"); open(b, "w").write(s2)
