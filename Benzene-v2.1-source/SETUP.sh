#!/usr/bin/env bash
# ============================================================
#  Benzene — one-time toolchain + Windows SDK setup
#
#  Installs everything needed to cross-compile the Windows VST3
#  from Linux (tested on Ubuntu 24.04):
#    - clang-18 / lld-18 / llvm-18, cmake, ninja, wine
#    - JUCE 8 (cloned to $JUCE_DIR if missing)
#    - the Windows SDK + MSVC CRT, fetched and laid out under $XWIN
#    - the clang-cl wrapper + tool symlinks
#    - all the case-fold / debug-CRT / STL-version fixups
#
#  Run this ONCE. Afterwards use build.sh to (re)build.
#
#  NOTE ON SDK FETCH: this script fetches the Windows SDK packages
#  directly with curl (parsing the VS manifest) rather than via
#  `xwin splat`. xwin's bundled TLS roots reject some corporate /
#  proxy CA chains; curl uses the system trust store and works in
#  more environments. If you prefer the stock tool, install xwin
#  (cargo install xwin) and run its splat into $XWIN instead — the
#  rest of this script (symlinks, stubs) still applies.
#
#  Environment overrides:
#     JUCE_DIR   default $HOME/JUCE
#     XWIN       default /opt/xwin-sdk
# ============================================================
set -euo pipefail

JUCE_DIR="${JUCE_DIR:-$HOME/JUCE}"
XWIN="${XWIN:-/opt/xwin-sdk}"
SDKVER="10.0.26100"
CRTVER="14.44.17.14"          # MSVC CRT package version in the VS manifest
WORK="$(mktemp -d)"
SUDO=""
[ "$(id -u)" -ne 0 ] && SUDO="sudo"

echo ">> Benzene setup"
echo "   JUCE_DIR=$JUCE_DIR   XWIN=$XWIN"

# ------------------------------------------------------------
# 1. APT packages
# ------------------------------------------------------------
echo ">> installing apt packages"
$SUDO apt-get update -qq
$SUDO DEBIAN_FRONTEND=noninteractive apt-get install -y -qq \
    cmake ninja-build clang-18 lld-18 llvm-18 \
    wine64 msitools ca-certificates curl git python3 \
    libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev \
    libfreetype-dev libfontconfig-dev libasound2-dev \
    libcurl4-openssl-dev libwebkit2gtk-4.1-dev
$SUDO update-ca-certificates || true

# ------------------------------------------------------------
# 2. JUCE 8
# ------------------------------------------------------------
if [ ! -d "$JUCE_DIR/modules" ]; then
    echo ">> cloning JUCE 8 to $JUCE_DIR"
    git clone --depth 1 https://github.com/juce-framework/JUCE.git "$JUCE_DIR"
else
    echo ">> JUCE already present at $JUCE_DIR"
fi

# ------------------------------------------------------------
# 3. clang-cl wrapper + tool symlinks
# ------------------------------------------------------------
echo ">> installing clang-cl wrapper and tool symlinks"
$SUDO ln -sf /usr/lib/llvm-18/bin/clang /usr/lib/llvm-18/bin/clang-cl
$SUDO tee /usr/local/bin/benzene-clang-cl >/dev/null << 'WRAP'
#!/bin/bash
# clang-cl needs --target as its first arg, and CMake sometimes passes a bare
# "--" separator that clang-cl rejects; strip it.
args=()
for arg in "$@"; do [[ "$arg" == "--" ]] && continue; args+=("$arg"); done
exec /usr/lib/llvm-18/bin/clang-cl --target=x86_64-pc-windows-msvc "${args[@]}"
WRAP
$SUDO chmod +x /usr/local/bin/benzene-clang-cl
$SUDO ln -sf /usr/lib/llvm-18/bin/clang-cl /usr/local/bin/clang-cl
$SUDO ln -sf /usr/bin/llvm-ar-18           /usr/local/bin/llvm-lib
$SUDO ln -sf /usr/bin/lld-link-18          /usr/local/bin/lld-link
$SUDO ln -sf /usr/lib/llvm-18/bin/lld-link /usr/bin/lld-link 2>/dev/null || true
$SUDO ln -sf /usr/bin/llvm-mt-18           /usr/local/bin/llvm-mt
$SUDO ln -sf /usr/bin/llvm-rc-18           /usr/local/bin/llvm-rc

# ------------------------------------------------------------
# 4. Fetch the Windows SDK + MSVC CRT (curl-based, proxy-safe)
# ------------------------------------------------------------
if [ -d "$XWIN/sdk/include/$SDKVER" ] && [ -d "$XWIN/crt/include" ]; then
    echo ">> Windows SDK already present at $XWIN — skipping fetch"
else
    echo ">> fetching Windows SDK + MSVC CRT into $XWIN (this downloads ~hundreds of MB)"
    $SUDO mkdir -p "$XWIN/crt" "$XWIN/sdk"
    $SUDO chown -R "$(id -u):$(id -g)" "$XWIN"
    cd "$WORK"

    # 4a. VS channel manifest -> product manifest (.vsman)
    curl -sL "https://aka.ms/vs/17/release/channel" -o vs17channel.json
    VSMAN_URL=$(python3 - << 'PY'
import json
ch=json.load(open('vs17channel.json'))
for it in ch.get('channelItems',[]):
    if it.get('id')=='Microsoft.VisualStudio.Manifests.VisualStudio':
        print(it['payloads'][0]['url'])
PY
)
    curl -sL "$VSMAN_URL" -o vs.vsman

    # 4b. download CRT (headers + x64 libs, plus store/onecore for the import libs)
    mkdir -p dl
    python3 - "$CRTVER" << 'PY'
import json,sys,os,subprocess
crt=sys.argv[1]
m=json.load(open('vs.vsman'))
def curl(u,o): subprocess.run(['curl','-sL','--retry','3','--max-time','300',u,'-o',o])
want=[f'microsoft.vc.{crt}.crt.headers.base',
      f'microsoft.vc.{crt}.crt.x64.desktop.base',
      f'microsoft.vc.{crt}.crt.x64.store.base',
      f'microsoft.vc.{crt}.crt.x64.onecore.desktop.base']
for p in m['packages']:
    if p['id'].lower() in want:
        for pl in p.get('payloads',[]):
            fn=pl['fileName'].replace('\\','/').split('/')[-1]
            curl(pl['url'], f"dl/{p['id']}__{fn}")
            print("CRT", p['id'], fn)
PY

    # 4c. download Windows SDK MSIs (+ all cabs) for headers and x64 libs
    python3 - << 'PY'
import json,os,subprocess
m=json.load(open('vs.vsman'))
sdk=[p for p in m['packages'] if p['id'].lower()=='win11sdk_10.0.26100'][0]
def base(fn): return fn.replace('\\','/').split('/')[-1]
def curl(u,o): subprocess.run(['curl','-sL','--retry','3','--max-time','300',u,'-o',o])
os.makedirs('dl/sdk',exist_ok=True)
for pl in sdk['payloads']:
    fn=base(pl['fileName']); low=fn.lower()
    if low.endswith('.msi') or low.endswith('.cab'):
        curl(pl['url'], f'dl/sdk/{fn}')
print("SDK payloads downloaded:", len(os.listdir('dl/sdk')))
PY

    # 4d. extract CRT vsix (zip) -> $XWIN/crt
    mkdir -p crt_x
    for v in dl/*headers*.vsix dl/*x64.desktop.base*.vsix dl/*Store*.vsix dl/*OneCore*.vsix; do
        [ -f "$v" ] && unzip -oq "$v" -d crt_x || true
    done
    MSVC=$(find crt_x/Contents/VC/Tools/MSVC -maxdepth 1 -mindepth 1 -type d | head -1)
    mkdir -p "$XWIN/crt/include" "$XWIN/crt/lib/x86_64"
    cp -r "$MSVC/include/." "$XWIN/crt/include/"
    cp "$MSVC/lib/x64/"*.lib "$XWIN/crt/lib/x86_64/" 2>/dev/null || true
    # dynamic import libs live in the store/onecore variants
    find crt_x -path '*lib/x64/msvcprt.lib' -exec cp {} "$XWIN/crt/lib/x86_64/" \; 2>/dev/null || true
    find crt_x -path '*lib/onecore/x64/oldnames.lib' -exec cp {} "$XWIN/crt/lib/x86_64/" \; 2>/dev/null || true
    find crt_x -name 'comsuppw.lib' -exec cp {} "$XWIN/crt/lib/x86_64/" \; 2>/dev/null || true

    # 4e. extract SDK MSIs -> $XWIN/sdk
    mkdir -p sdk_x
    for msi in dl/sdk/*.msi; do msiextract -C sdk_x "$msi" >/dev/null 2>&1 || true; done
    KIT="sdk_x/Program Files/Windows Kits/10"
    mkdir -p "$XWIN/sdk/include/$SDKVER" "$XWIN/sdk/lib/um/x86_64" "$XWIN/sdk/lib/ucrt/x86_64"
    cp -r "$KIT/Include/$SDKVER.0/." "$XWIN/sdk/include/$SDKVER/"
    cp "$KIT/Lib/$SDKVER.0/um/x64/"*.[lL]ib   "$XWIN/sdk/lib/um/x86_64/"   2>/dev/null || true
    cp "$KIT/Lib/$SDKVER.0/ucrt/x64/"*.[lL]ib "$XWIN/sdk/lib/ucrt/x86_64/" 2>/dev/null || true
fi

# ------------------------------------------------------------
# 5. Fixups: case-fold symlinks, debug-CRT stubs, STL version guard
# ------------------------------------------------------------
echo ">> applying case-fold symlinks and stubs"

# 5a. lowercase symlinks for all libs (Linux is case-sensitive; the SDK isn't)
python3 - "$XWIN" << 'PY'
import os,sys
xwin=sys.argv[1]
for d in [f'{xwin}/sdk/lib/um/x86_64', f'{xwin}/crt/lib/x86_64', f'{xwin}/sdk/lib/ucrt/x86_64']:
    if not os.path.isdir(d): continue
    for f in os.listdir(d):
        lo=f.lower()
        if lo!=f and not os.path.exists(os.path.join(d,lo)):
            try: os.symlink(os.path.join(d,f), os.path.join(d,lo))
            except OSError: pass
PY

# 5b. exact-case lib aliases the linker asks for by specific names
( cd "$XWIN/sdk/lib/um/x86_64/" && \
  for p in "dwrite.lib Dwrite.lib" "d2d1.lib D2d1.lib" "dxgi.lib DXGI.lib" \
           "d3d11.lib D3D11.lib" "dbghelp.lib DbgHelp.lib"; do
      set -- $p; [ -f "$1" ] && ln -sf "$1" "$2" 2>/dev/null || true
  done )

# 5c. bidirectional header case aliases — many SDK headers #include each other
#     with mixed case that doesn't match the on-disk (often lowercased) names.
python3 - "$XWIN" "$SDKVER" << 'PY'
import os,re,sys
xwin,sdkver=sys.argv[1],sys.argv[2]
root=f'{xwin}/sdk/include/{sdkver}'
dirs=[f'{xwin}/crt/include']+[f'{root}/{s}' for s in ('um','shared','ucrt','winrt')]
index={}
for d in dirs:
    if not os.path.isdir(d): continue
    for f in os.listdir(d):
        p=os.path.join(d,f)
        if os.path.isfile(p) or os.path.islink(p):
            index.setdefault(f.lower(),[]).append((d,f))
inc=re.compile(r'#\s*include\s*[<"]([^>"]+)[>"]')
refs=set()
for d in dirs:
    if not os.path.isdir(d): continue
    for f in os.listdir(d):
        p=os.path.join(d,f)
        if not (os.path.isfile(p) and not os.path.islink(p)): continue
        try:
            for line in open(p,errors='ignore'):
                if '#include' in line:
                    m=inc.search(line)
                    if m: refs.add(m.group(1).replace('\\','/').split('/')[-1])
        except OSError: pass
n=0
for ref in refs:
    lo=ref.lower()
    if lo not in index: continue
    for d,real in index[lo]:
        tgt=os.path.join(d,ref)
        if not os.path.exists(tgt):
            try: os.symlink(real,tgt); n+=1
            except OSError: pass
print(f"   header case aliases: {n}")
PY

# 5d. debug-CRT stubs (CMake's compiler check links against *d.lib variants)
for f in msvcrt msvcprt vcruntime libcmt libvcruntime; do
    ln -sf "$XWIN/crt/lib/x86_64/${f}.lib" "$XWIN/crt/lib/x86_64/${f}d.lib" 2>/dev/null || true
done
ln -sf "$XWIN/sdk/lib/ucrt/x86_64/ucrt.lib" "$XWIN/sdk/lib/ucrt/x86_64/ucrtd.lib" 2>/dev/null || true

# 5e. VS2022 STL requires Clang 19+; we use Clang 18 (fully compatible). Relax the guard.
YV="$XWIN/crt/include/yvals_core.h"
[ -f "$YV" ] && sed -i 's/__clang_major__ < 19/__clang_major__ < 18/' "$YV" || true

rm -rf "$WORK"
echo ""
echo ">> setup complete. Now run:  ./build.sh"
