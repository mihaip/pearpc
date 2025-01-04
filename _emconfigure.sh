#!/bin/bash
set -euo pipefail

em_flags=""
em_ldflags=""

em_defines=""
if [[ -z "${pearpcjs_conf_debug:-}" ]]; then
  em_flags+=" -O3 -gsource-map"
else
  echo "Debug build"
  em_flags+=" -O0 -gsource-map"
  em_ldflags+=" -s ASSERTIONS=2 "
  em_ldflags+=" -s DEMANGLE_SUPPORT=1"
  em_defines+=" -DDEBUG"
fi

export EMSCRIPTEN=1
export DEFINES=$em_defines
export CFLAGS="$em_flags -g"
export CPPFLAGS="$em_flags -g"
export LDFLAGS="$em_flags $em_ldflags"

emconfigure ./autogen.sh
emconfigure ./configure \
  --enable-emscripten=yes \
  --build="asmjs-unknown-linux-gnu" \
  --cache-file="/tmp/pearpc.config.cache.emscripten${pearpcjs_conf_debug:-}"
