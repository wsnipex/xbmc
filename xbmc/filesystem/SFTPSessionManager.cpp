/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "SFTPSessionManager.h"
#ifdef HAS_FILESYSTEM_SFTP
#include "SFTPSession.h"
#include "threads/SingleLock.h"
#include "utils/Variant.h"
#include "Util.h"
#include "URL.h"
#include <fcntl.h>
#include <sstream>

#ifdef TARGET_WINDOWS
#pragma comment(lib, "ssh.lib")
#endif

using namespace std;

CCriticalSection CSFTPSessionManager::m_critSect;
map<CStdString, CSFTPSessionPtr> CSFTPSessionManager::sessions;

CSFTPSessionPtr CSFTPSessionManager::CreateSession(const CURL &url)
{
  string username = url.GetUserName().c_str();
  string password = url.GetPassWord().c_str();
  string hostname = url.GetHostName().c_str();
  unsigned int port = url.HasPort() ? url.GetPort() : 22;

  return CSFTPSessionManager::CreateSession(hostname, port, username, password);
}

CSFTPSessionPtr CSFTPSessionManager::CreateSession(const CStdString &host, unsigned int port, const CStdString &username, const CStdString &password)
{
  // Convert port number to string
  stringstream itoa;
  itoa << port;
  CStdString portstr = itoa.str();

  CSingleLock lock(m_critSect);
  CStdString key = username + ":" + password + "@" + host + ":" + portstr;
  CSFTPSessionPtr ptr = sessions[key];
  if (ptr == NULL)
  {
    ptr = CSFTPSessionPtr(new CSFTPSession(host, port, username, password));
    sessions[key] = ptr;
  }

  return ptr;
}

void CSFTPSessionManager::ClearOutIdleSessions()
{
  CSingleLock lock(m_critSect);
  for(map<CStdString, CSFTPSessionPtr>::iterator iter = sessions.begin(); iter != sessions.end();)
  {
    if (iter->second->IsIdle())
      sessions.erase(iter++);
    else
      iter++;
  }
}

void CSFTPSessionManager::DisconnectAllSessions()
{
  CSingleLock lock(m_critSect);
  sessions.clear();
}

#endif
