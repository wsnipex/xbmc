#pragma once
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

#include "system.h"
#ifdef HAS_FILESYSTEM_SFTP
#include "SFTP.h"
#include "utils/StdString.h"
#include "threads/CriticalSection.h"

class CFileItemList;

class CSFTPSession
{
public:
  CSFTPSession(const CStdString &host, unsigned int port, const CStdString &username, const CStdString &password);
  virtual ~CSFTPSession();

  sftp_file CreateFileHande(const CStdString &file);
  void CloseFileHandle(sftp_file handle);
  bool GetDirectory(const CStdString &base, const CStdString &folder, CFileItemList &items);
  bool DirectoryExists(const char *path);
  bool FileExists(const char *path);
  int Stat(const char *path, struct __stat64* buffer);
  int Seek(sftp_file handle, uint64_t position);
  int Read(sftp_file handle, void *buffer, size_t length);
  int64_t GetPosition(sftp_file handle);
  bool IsIdle();
private:
  bool VerifyKnownHost(ssh_session session);
  bool Connect(const CStdString &host, unsigned int port, const CStdString &username, const CStdString &password);
  void Disconnect();
  bool GetItemPermissions(const char *path, uint32_t &permissions);
  CCriticalSection m_critSect;

  bool m_connected;
  int m_sock;
  ssh_session  m_session;
  sftp_session m_sftp_session;
  int m_LastActive;
};

#endif
