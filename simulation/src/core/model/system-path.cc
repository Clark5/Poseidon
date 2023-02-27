/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "system-path.h"
#include "fatal-error.h"
#include "assert.h"
#include "log.h"
#include "ns3/core-config.h"
#include <cstdlib>
#include <cerrno>
#include <cstring>
#ifndef WIN32
#if defined (HAVE_DIRENT_H) and defined (HAVE_SYS_TYPES_H)
#define HAVE_OPENDIR
#include <sys/types.h>
#include <dirent.h>
#endif
#if defined (HAVE_SYS_STAT_H) and defined (HAVE_SYS_TYPES_H)
#define HAVE_MKDIR_H
#include <sys/types.h>
#include <sys/stat.h>
#endif
#endif
#include <sstream>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif /* __APPLE__ */

#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#ifdef __linux__
#include <unistd.h>
#endif

#if defined (WIN32)
#define SYSTEM_PATH_SEP "\\"
#include <Windows.h>
#include <WinBase.h>
#define HAVE_FIND_FIRST_FILE
#else
#define SYSTEM_PATH_SEP "/"
#endif


NS_LOG_COMPONENT_DEFINE ("SystemPath");

namespace ns3 {

namespace SystemPath {

std::string Dirname (std::string path)
{
  NS_LOG_FUNCTION (path);
  std::list<std::string> elements = Split (path);
  std::list<std::string>::const_iterator last = elements.end();
  last--;
  return Join (elements.begin (), last);
}

std::string FindSelfDirectory (void)
{
  /** 
   * This function returns the path to the running $PREFIX.
   * Mac OS X: _NSGetExecutablePath() (man 3 dyld)
   * Linux: readlink /proc/self/exe
   * Solaris: getexecname()
   * FreeBSD: sysctl CTL_KERN KERN_PROC KERN_PROC_PATHNAME -1
   * BSD with procfs: readlink /proc/curproc/file
   * Windows: GetModuleFileName() with hModule = NULL
   */
  NS_LOG_FUNCTION_NOARGS ();
  std::string filename;
#if defined(__linux__)
  {
    ssize_t size = 1024;
    char *buffer = (char*)malloc (size);
    memset (buffer, 0, size);
    int status;
    while (true)
      {
        status = readlink("/proc/self/exe", buffer, size);
        if (status != 1 || (status == -1 && errno != ENAMETOOLONG))
          {
            break;
          }
        size *= 2;
        free (buffer);
        buffer = (char*)malloc (size);
	memset (buffer, 0, size);
      }
    if (status == -1)
      {
        NS_FATAL_ERROR ("Oops, could not find self directory.");
      }
    filename = buffer;
    free (buffer);
  }
#elif defined (WIN32)
  {
    /// \todo untested. it should work if code is compiled with
    /// LPTSTR = char *
    DWORD size = 1024;
    LPTSTR lpFilename = (LPTSTR) malloc (sizeof(TCHAR) * size);
    DWORD status = GetModuleFileName (0, lpFilename, size);
    
	while (status == size)
      {
	size = size * 2;
	free (lpFilename);
	lpFilename = (LPTSTR) malloc (sizeof(TCHAR) * size);
	status = GetModuleFileName (0, lpFilename, size);
      }
    NS_ASSERT (status != 0);
    filename = lpFilename;
    free (lpFilename);
  }
#elif defined (__APPLE__)
  {
    uint32_t bufsize = 1024;
    char *buffer = (char *) malloc (bufsize);
    NS_ASSERT (buffer != 0);
    int status = _NSGetExecutablePath (buffer, &bufsize);
    if (status == -1)
      {
	free (buffer);
	buffer = (char *) malloc (bufsize);
	status = _NSGetExecutablePath (buffer, &bufsize);
      }
    NS_ASSERT (status == 0);
    filename = buffer;
    free (buffer);
  } 
#elif defined (__FreeBSD__)
  {
    int     mib[4];
    size_t  bufSize = 1024;
    char   *buf = (char *) malloc(bufSize);

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PATHNAME;
    mib[3] = -1;

    sysctl(mib, 4, buf, &bufSize, NULL, 0);
    filename = buf;
  }
#endif
    return Dirname (filename);
}
  
std::string Append (std::string left, std::string right)
{
  // removing trailing separators from 'left'
  NS_LOG_FUNCTION (left << right);
  while (true)
    {
      std::string::size_type lastSep = left.rfind (SYSTEM_PATH_SEP);
      if (lastSep != left.size () - 1)
	{
	  break;
	}
      left = left.substr (0, left.size () - 1);
    } 
  std::string retval = left + SYSTEM_PATH_SEP + right;
  return retval;
}

std::list<std::string> Split (std::string path)
{
  NS_LOG_FUNCTION (path);
  std::list<std::string> retval;
  std::string::size_type current = 0, next = 0;
  next = path.find (SYSTEM_PATH_SEP, current);
  while (next != std::string::npos)
    {
      std::string item = path.substr (current, next - current);
      retval.push_back (item);
      current = next + 1;
      next = path.find (SYSTEM_PATH_SEP, current);
    }
  std::string item = path.substr (current, next - current);
  retval.push_back (item);
  return retval;
}

std::string Join (std::list<std::string>::const_iterator begin,
		  std::list<std::string>::const_iterator end)
{
  NS_LOG_FUNCTION (&begin << &end);
  std::string retval = "";
  for (std::list<std::string>::const_iterator i = begin; i != end; i++)
    {
      if (i == begin)
	{
	  retval = *i;
	}
      else
	{
	  retval = retval + SYSTEM_PATH_SEP + *i;
	}
    }
  return retval;
}
  
std::list<std::string> ReadFiles (std::string path)
{
  NS_LOG_FUNCTION (path);
  std::list<std::string> files;
#if defined HAVE_OPENDIR
  DIR *dp = opendir (path.c_str ());
  if (dp == NULL)
    {
      NS_FATAL_ERROR ("Could not open directory=" << path);
    }
  struct dirent *de = readdir (dp);
  while (de != 0)
    {
      files.push_back (de->d_name);
      de = readdir (dp);
    }
  closedir (dp);
#elif defined (HAVE_FIND_FIRST_FILE)
  HANDLE hFind;
  WIN32_FIND_DATA fileData;
 
  std::stringstream ss;
  ss << path << "\\*";
  
  hFind = FindFirstFile (ss.str().c_str(), &fileData);
  std::string buffer;
  if(hFind != INVALID_HANDLE_VALUE)
  {
	  buffer = fileData.cFileName;
	  files.push_back(fileData.cFileName);
	  while(FindNextFile(hFind, &fileData))	 
	  {
		buffer = fileData.cFileName;
		files.push_back(fileData.cFileName);
	  }
  }
  else
  {
	NS_FATAL_ERROR ("Could not open directory=" << path);
  }
/*
  if (hFind == INVALID_HANDLE_VALUE)
    {
      
    }
  do
    {
      files.push_back (fileData.cFileName);
    } while (FindNextFile (hFind, &fileData));
*/
  FindClose(hFind);
#else
#error "No support for reading a directory on this platform"
#endif
  return files;
}

std::string 
MakeTemporaryDirectoryName (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  char *path = NULL;

  path = getenv ("TMP");
  if (path == NULL)
    {
      path = getenv ("TEMP");
      if (path == NULL)
	{
	  path = const_cast<char *> ("/tmp");
	}
    }

  //
  // Just in case the user wants to go back and find the output, we give
  // a hint as to which dir we created by including a time hint.
  //
#ifndef WIN32
  time_t now = time (NULL);
  struct tm *tm_now = localtime (&now);
#endif
  //
  // But we also randomize the name in case there are multiple users doing
  // this at the same time
  //
#ifndef WIN32
  srand (time (0));
  long int n = rand ();
#endif
  //
  // The final path to the directory is going to look something like
  // 
  //   /tmp/ns3-14.30.29.32767
  //
  // The first segment comes from one of the temporary directory env 
  // variables or /tmp if not found.  The directory name starts with an
  // identifier telling folks who is making all of the temp directories
  // and then the local time (in this case 14.30.29 -- which is 2:30 and
  // 29 seconds PM).
  //
  std::ostringstream oss;
#ifndef WIN32
  oss << path << SYSTEM_PATH_SEP << "ns-3." << tm_now->tm_hour << "."
      << tm_now->tm_min << "." << tm_now->tm_sec << "." << n;
#else
  //This may need to be fixed.  Used?
  oss << path << SYSTEM_PATH_SEP << "ns-3";
#endif
  return oss.str ();
}

void 
MakeDirectories (std::string path)
{
  NS_LOG_FUNCTION (path);
  std::list<std::string> elements = Split (path);
  for (std::list<std::string>::const_iterator i = elements.begin (); i != elements.end (); ++i)
    {
      std::string tmp = Join (elements.begin (), i);
#ifdef WIN32
	  if(!CreateDirectory(tmp.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS && tmp.size() != 0)
	   {
		   NS_LOG_ERROR ("failed creating directory " << tmp);
	   }
#elif defined(HAVE_MKDIR_H)
      if (mkdir (tmp.c_str (), S_IRWXU))
        {
          NS_LOG_ERROR ("failed creating directory " << tmp);
        }
#endif
    }
#ifdef WIN32
	   if(!CreateDirectory(path.c_str (), NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
	   {
		   NS_LOG_ERROR ("failed creating directory " << path);
	   }
#elif defined(HAVE_MKDIR_H)
      if (mkdir (path.c_str (), S_IRWXU))
        {
          NS_LOG_ERROR ("failed creating directory " << path);
        }
#endif

}

} // namespace SystemPath

} // namespace ns3
