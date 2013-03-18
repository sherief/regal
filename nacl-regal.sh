#!/bin/bash
# Copyright (c) 2012 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Any arguments passed to this script will be passed onto make.  It is
# recommend to pass -j<N> to speed up the build.

# NOTE - Pepper 25 is needed for the ARM build.

set -e

echo "Building x86_64"
make -f Makefile SYSTEM=nacl-x86_64 $*
echo "Building i686"
make -f Makefile SYSTEM=nacl-i686 $*

unset NACL_GLIBC
echo "Building ARM"
make -f Makefile SYSTEM=nacl-arm $*
