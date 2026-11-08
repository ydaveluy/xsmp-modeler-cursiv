#!/usr/bin/env bash
# Checks a native build once tools/package-native.sh has run: the example
# workspace validates, and a TAS-MDK project generates through the handler of
# the TAS-MDK package, loaded as the editor would load it.
#
#   tools/check-native.sh <build directory>
set -euo pipefail

cli=$(cd "$1" && pwd)/src/xsmp_cli
root=$(cd "$(dirname "$0")/.." && pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

"$cli" validate "$root/example/06-mission-system" -w "$root/example"

mkdir -p "$work/TasMdk/smdl" "$work/Demo/smdl"
cat > "$work/TasMdk/xsmp.project" <<'EOF'
project 'TasMdk' using 'ECSS_SMP_2025'
source 'smdl'
EOF
cat > "$work/TasMdk/smdl/TasMdk.xsmpcat" <<'EOF'
catalogue TasMdk
namespace TasMdk {
  /** @uuid 11111111-1111-1111-1111-111111111111 */
  public model Model {}
}
EOF
cat > "$work/Demo/xsmp.project" <<'EOF'
project 'Demo' using 'ECSS_SMP_2025'
profile 'tas-mdk'
dependency 'TasMdk'
source 'smdl'
EOF
cat > "$work/Demo/smdl/Demo.xsmpcat" <<'EOF'
catalogue Demo
namespace demo {
  /** @uuid 01234567-89ab-cdef-0123-456789abcdef */
  public model Root extends TasMdk.Model {}
}
EOF
"$cli" generate "$work/Demo" -w "$work" --extension "$root/extensions/tas-mdk"
test -n "$(find "$work/Demo" -path '*-gen/*' -type f -print -quit)"
