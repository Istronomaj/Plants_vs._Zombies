#!/usr/bin/env bash
#
# Runs a command with the MSVC toolchain on PATH.
#
# The CMake and Ninja this project uses ship inside Visual Studio Build Tools
# and are not on the default PATH, and the compiler needs the environment that
# VsDevCmd.bat sets up. Rather than adding all of that to your shell profile,
# this wraps a single command.
#
# Usage:
#   ./scripts/devshell.sh cmake --preset dev-release
#   ./scripts/devshell.sh cmake --build --preset dev-release
#   ./scripts/devshell.sh ctest --preset dev-release
#
# On Linux and macOS the toolchain is already on PATH, so the command runs
# unchanged and this script is a no-op wrapper.

set -euo pipefail

if [[ $# -eq 0 ]]; then
    echo "usage: $0 <command> [args...]" >&2
    exit 2
fi

# Not Windows: nothing to set up.
if [[ "$(uname -s)" != MINGW* && "$(uname -s)" != MSYS* && "$(uname -s)" != CYGWIN* ]]; then
    exec "$@"
fi

find_vsdevcmd() {
    local vswhere="/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe"
    if [[ -x "$vswhere" ]]; then
        local install
        install=$("$vswhere" -latest -products '*' \
            -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 \
            -property installationPath 2>/dev/null | tr -d '\r')
        if [[ -n "$install" ]]; then
            # vswhere prints a Windows path; convert it for bash.
            local unix_path
            unix_path=$(cygpath -u "$install" 2>/dev/null || echo "$install")
            if [[ -f "$unix_path/Common7/Tools/VsDevCmd.bat" ]]; then
                cygpath -w "$unix_path/Common7/Tools/VsDevCmd.bat"
                return 0
            fi
        fi
    fi
    return 1
}

if ! vsdevcmd=$(find_vsdevcmd); then
    echo "error: could not find Visual Studio Build Tools with the C++ workload." >&2
    echo "Install it from https://visualstudio.microsoft.com/downloads/ (Build Tools" >&2
    echo "for Visual Studio -> Desktop development with C++)." >&2
    exit 1
fi

# Quote each argument so paths containing spaces survive the trip through cmd.
quoted=""
for arg in "$@"; do
    quoted+=" \"$arg\""
done

project_dir=$(cygpath -w "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")

# Written to a temporary .bat rather than passed via `cmd /c`: the command line
# needs nested quotes around three paths that all contain spaces, and getting
# those through bash, MSYS path mangling and cmd's own parser intact is not
# worth the fight.
batch=$(mktemp --suffix=.bat)
trap 'rm -f "$batch"' EXIT

# cd before calling VsDevCmd: it resolves vswhere.exe relative to the current
# directory, and inheriting Git Bash's cwd makes that lookup fail noisily.
cat > "$batch" <<EOF
@echo off
cd /d "$project_dir"
call "$vsdevcmd" -arch=x64 -host_arch=x64 >nul 2>&1
cd /d "$project_dir"
$quoted
EOF

batch_win=$(cygpath -w "$batch")

# Scoped to this one call: MSYS rewrites anything that looks like a Unix path,
# which would turn "/c" into a drive path and mangle the batch file's location.
MSYS_NO_PATHCONV=1 MSYS2_ARG_CONV_EXCL='*' cmd.exe /c "$batch_win"
