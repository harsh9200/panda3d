// Filename: crypto_utils.cxx
// Created by:  drose (07Nov00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

// This file is compiled only if we have crypto++ installed.


#include "hashVal.h"
#include "config_express.h"
#include "crypto_utils.h"

#include <md5.h>
#include <hex.h>
#include <files.h>

// md5 defines this, but we'd rather do without it, because it
// conflicts with a similar macro defined in NSPR.
#undef IS_LITTLE_ENDIAN

USING_NAMESPACE(CryptoPP);
USING_NAMESPACE(std);

static uint
read32(istream& is) {
  unsigned int ret = 0x0;
  for (int i=0; i<8; ++i) {
    char b;
    int n = 0;
    is >> b;
    switch (b) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      n = b - '0';
      break;
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
      n = b - 'A' + 10;
      break;
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
      n = b - 'a' + 10;
      break;
    default:
      cerr << "illegal hex nibble '" << b << "'" << endl;
    }
    ret = (ret << 4) | (n & 0x0f);
  }
  return ret;
}

void
md5_a_file(const Filename &name, HashVal &ret) {
  ostringstream os;
  MD5 md5;

  string fs = name.to_os_specific();

  try {
    FileSource f(fs.c_str(), true,
                 new HashFilter(md5, new HexEncoder(new FileSink(os))));
  } catch (...) {
    express_cat.warning()
      << "Unable to read " << name << " to compute md5 hash.\n";
    if (!name.exists()) {
      express_cat.warning()
        << "(file does not exist.)\n";
    } else {
      express_cat.warning()
        << "(file exists but cannot be read.)\n";
    }
    ret.hv[0] = 0;
    ret.hv[1] = 0;
    ret.hv[2] = 0;
    ret.hv[3] = 0;
    return;
  }

  istringstream is(os.str());

  ret.hv[0] = read32(is);
  ret.hv[1] = read32(is);
  ret.hv[2] = read32(is);
  ret.hv[3] = read32(is);
}

void
md5_a_buffer(const unsigned char* buf, unsigned long len, HashVal& ret) {
  MD5 md5;

  HashFilter hash(md5);
  hash.Put((byte*)buf, len);
  hash.Close();

  unsigned char* outb;
  unsigned long outl = hash.MaxRetrieveable();
  outb = new uchar[outl];
  hash.Get((byte*)outb, outl);
  ret.hv[0] = (outb[0] << 24) | (outb[1] << 16) | (outb[2] << 8) | outb[3];
  ret.hv[1] = (outb[4] << 24) | (outb[5] << 16) | (outb[6] << 8) | outb[7];
  ret.hv[2] = (outb[8] << 24) | (outb[9] << 16) | (outb[10] << 8) | outb[11];
  ret.hv[3] = (outb[12] << 24) | (outb[13] << 16) | (outb[14] << 8) | outb[15];
  delete[] outb;
}

