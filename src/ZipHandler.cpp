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

// ZipHandler.cpp -- handles the extracting from zip-archives

#include <tchar.h>
#include <string>
#include "zzip/zzip.h"

#include "ZipHandler.h"

using namespace std;

bool ZipHandler::openDir(string dirName)
{	
  ZZIP_DIR* dir;
  dir = zzip_dir_open(_T(dirName.c_str()),0);	

  if (dir)
  {
    dir_t newDir;
    newDir.dirName = dirName;
    newDir.dir = dir;
    dirs.push_back(newDir);			
    return true;
  }
  else
  {
    return false;
  }	
}

bool ZipHandler::extractFile(string fileName, string outputName, bool ignoreSuffix )
{
  bool found = false;
  bool written = false;

  vector<dir_t>::iterator dirIterator;
  for (dirIterator = dirs.begin(); dirIterator != dirs.end(); dirIterator++)
  {
    dir_t dir_entry = *dirIterator;
    ZZIP_DIR* dir = dir_entry.dir;

    if (dir)
    {
      ZZIP_DIRENT dirent;
      while (zzip_dir_read(dir,&dirent) != 0 ) 
      {
        int k = 0;

        char *text = new char[strlen(dirent.d_name) + 1];			   
        strcpy_s(text, strlen(dirent.d_name) + 1, dirent.d_name);

        if (ignoreSuffix)
        {
          while (text[k] != '.' && text[k] != '\0')
            k++;

          text[k] = '\0';
        }

        if (!strcmp(text, fileName.c_str()))
        {							 	   
          delete text;
          found = true;
          break;     
        }
        delete text;
      }

      if (found)
      {
        ZZIP_FILE* fp = zzip_file_open(dir,dirent.d_name,0);

        if (fp) 
        {		 
          unsigned char *buf;
          buf = new unsigned char[dirent.st_size];

          zzip_ssize_t len = zzip_file_read(fp, buf, dirent.st_size);		

          if (len)
          {			   
            FILE *textureFile;
            fopen_s(&textureFile, outputName.c_str(), "wb");
            fwrite(buf, sizeof(buf[0]), len, textureFile);
            fclose(textureFile);    	
            written = true;
          }
          delete buf;
          zzip_file_close(fp);
        }
      }

      if (written)
      {
        break;
      }
    }
  }

  return written;
}

bool ZipHandler::closeDir(string dirName)
{
  vector<dir_t>::iterator dirIterator;
  for (dirIterator = dirs.begin(); dirIterator != dirs.end(); dirIterator++)
  {
    dir_t dir_entry = *dirIterator;
    string name = dir_entry.dirName;

    if (name.compare(dirName) == 0)
    {
      zzip_dir_close(dir_entry.dir);
      dirs.erase(dirIterator);
      return true;
    }
  }	

  return false;
}