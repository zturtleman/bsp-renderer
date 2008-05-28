/*
===========================================================================
Copyright (C) 2008 Daniel Örstadius

This file is part of bsp-renderer source code.

bsp-renderer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

bsp-renderer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with bsp-renderer.  If not, see <http://www.gnu.org/licenses/>.

*/

// ZipHandler.h -- handles the extracting from zip-archives

#ifndef _ZIPHANDLER_H
#define _ZIPHANDLER_H

#include <string>
#include <vector>
#include "zzip/zzip.h"

using namespace std;

typedef struct {
  string dirName;
  ZZIP_DIR* dir;
} dir_t;

class ZipHandler 
{
public:
  bool openDir(string dirName);
  bool extractFile(string fileName, string outputName, bool ignoreSuffix);
  bool closeDir(string dirName);

private:
  //ZZIP_DIR* dir;
  vector<dir_t> dirs;

};

#endif /* _ZIPHANDLER_H */