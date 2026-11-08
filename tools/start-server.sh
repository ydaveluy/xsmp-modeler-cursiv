#!/bin/sh
# Starts a packaged language server and checks that it answers `initialize`.
# On a system without build tools, this shows that the server finds every
# library it needs.
#
#   tools/start-server.sh <server executable>
set -eu

body='{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"processId":null,"rootUri":null,"capabilities":{}}}'
request() { printf 'Content-Length: %d\r\n\r\n%s' "${#body}" "$body"; }
if command -v timeout > /dev/null; then
  request | timeout 60 "$1" | grep -q '"capabilities"'
else
  request | "$1" | grep -q '"capabilities"'
fi
