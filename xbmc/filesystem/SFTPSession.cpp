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

#include "SFTPSession.h"
#ifdef HAS_FILESYSTEM_SFTP
#include "FileItem.h"
#include "threads/SingleLock.h"
#include "threads/SystemClock.h"
#include "utils/log.h"
#include "utils/TimeUtils.h"
#include "utils/Variant.h"
#include "Util.h"
#include "URL.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sstream>

#ifdef TARGET_WINDOWS
#pragma comment(lib, "ssh.lib")
#endif

#ifndef S_ISDIR
#define S_ISDIR(m) ((m & _S_IFDIR) != 0)
#endif
#ifndef S_ISREG
#define S_ISREG(m) ((m & _S_IFREG) != 0)
#endif
#ifndef O_RDONLY
#define O_RDONLY _O_RDONLY
#endif


using namespace std;

namespace
{

CStdString CorrectPath(const CStdString path)
{
  if (path == "~")
    return "./";
  else if (path.substr(0, 2) == "~/")
    return "./" + path.substr(2);
  else
    return "/" + path;
}

const char* Libssh2ErrorString(int err)
{
  switch(err)
  {
  default:
    return "Unknown error code";
  case LIBSSH2_ERROR_SOCKET_NONE:
    return "The socket is invalid";
  case LIBSSH2_ERROR_BANNER_RECV:
    return "LIBSSH2_ERROR_BANNER_RECV";
  case LIBSSH2_ERROR_BANNER_SEND:
    return "Unable to send banner to remote host";
  case LIBSSH2_ERROR_INVALID_MAC:
    return "LIBSSH2_ERROR_INVALID_MAC";
  case LIBSSH2_ERROR_KEX_FAILURE:
    return "Encryption key exchange with the remote host failed";
  case LIBSSH2_ERROR_ALLOC:
    return "LIBSSH2_ERROR_ALLOC";
  case LIBSSH2_ERROR_SOCKET_SEND:
    return "Unable to send data on socket";
  case LIBSSH2_ERROR_KEY_EXCHANGE_FAILURE:
    return "LIBSSH2_ERROR_KEY_EXCHANGE_FAILURE";
  case LIBSSH2_ERROR_TIMEOUT:
    return "Connection timed out";
  case LIBSSH2_ERROR_HOSTKEY_INIT:
    return "LIBSSH2_ERROR_HOSTKEY_INIT";
  case LIBSSH2_ERROR_HOSTKEY_SIGN:
    return "LIBSSH2_ERROR_HOSTKEY_SIGN";
  case LIBSSH2_ERROR_DECRYPT:
    return "LIBSSH2_ERROR_DECRYPT";
  case LIBSSH2_ERROR_SOCKET_DISCONNECT:
    return "The socket was disconnected";
  case LIBSSH2_ERROR_PROTO:
    return "An invalid SSH protocol response was received on the socket";
  case LIBSSH2_ERROR_PASSWORD_EXPIRED:
    return "LIBSSH2_ERROR_PASSWORD_EXPIRED";
  case LIBSSH2_ERROR_FILE:
    return "LIBSSH2_ERROR_FILE";
  case LIBSSH2_ERROR_METHOD_NONE:
    return "LIBSSH2_ERROR_METHOD_NONE";
  case LIBSSH2_ERROR_AUTHENTICATION_FAILED:
    return "LIBSSH2_ERROR_AUTHENTICATION_FAILED";
  case LIBSSH2_ERROR_PUBLICKEY_UNVERIFIED:
    return "LIBSSH2_ERROR_PUBLICKEY_UNVERIFIED";
  case LIBSSH2_ERROR_CHANNEL_OUTOFORDER:
    return "LIBSSH2_ERROR_CHANNEL_OUTOFORDER";
  case LIBSSH2_ERROR_CHANNEL_FAILURE:
    return "LIBSSH2_ERROR_CHANNEL_FAILURE";
  case LIBSSH2_ERROR_CHANNEL_REQUEST_DENIED:
    return "LIBSSH2_ERROR_CHANNEL_REQUEST_DENIED";
  case LIBSSH2_ERROR_CHANNEL_UNKNOWN:
    return "LIBSSH2_ERROR_CHANNEL_UNKNOWN";
  case LIBSSH2_ERROR_CHANNEL_WINDOW_EXCEEDED:
    return "LIBSSH2_ERROR_CHANNEL_WINDOW_EXCEEDED";
  case LIBSSH2_ERROR_CHANNEL_PACKET_EXCEEDED:
    return "LIBSSH2_ERROR_CHANNEL_PACKET_EXCEEDED";
  case LIBSSH2_ERROR_CHANNEL_CLOSED:
    return "LIBSSH2_ERROR_CHANNEL_CLOSED";
  case LIBSSH2_ERROR_CHANNEL_EOF_SENT:
    return "LIBSSH2_ERROR_CHANNEL_EOF_SENT";
  case LIBSSH2_ERROR_SCP_PROTOCOL:
    return "LIBSSH2_ERROR_SCP_PROTOCOL";
  case LIBSSH2_ERROR_ZLIB:
    return "LIBSSH2_ERROR_ZLIB";
  case LIBSSH2_ERROR_SOCKET_TIMEOUT:
    return "LIBSSH2_ERROR_SOCKET_TIMEOUT";
  case LIBSSH2_ERROR_SFTP_PROTOCOL:
    return "LIBSSH2_ERROR_SFTP_PROTOCOL";
  case LIBSSH2_ERROR_REQUEST_DENIED:
    return "LIBSSH2_ERROR_REQUEST_DENIED";
  case LIBSSH2_ERROR_METHOD_NOT_SUPPORTED:
    return "LIBSSH2_ERROR_METHOD_NOT_SUPPORTED";
  case LIBSSH2_ERROR_INVAL:
    return "LIBSSH2_ERROR_INVAL";
  case LIBSSH2_ERROR_INVALID_POLL_TYPE:
    return "LIBSSH2_ERROR_INVALID_POLL_TYPE";
  case LIBSSH2_ERROR_PUBLICKEY_PROTOCOL:
    return "LIBSSH2_ERROR_PUBLICKEY_PROTOCOL";
  case LIBSSH2_ERROR_EAGAIN:
    return "Marked for non-blocking I/O but the call would block";
  case LIBSSH2_ERROR_BUFFER_TOO_SMALL:
    return "LIBSSH2_ERROR_BUFFER_TOO_SMALL";
  case LIBSSH2_ERROR_BAD_USE:
    return "LIBSSH2_ERROR_BAD_USE";
  case LIBSSH2_ERROR_COMPRESS:
    return "LIBSSH2_ERROR_COMPRESS";
  case LIBSSH2_ERROR_OUT_OF_BOUNDARY:
    return "LIBSSH2_ERROR_OUT_OF_BOUNDARY";
  case LIBSSH2_ERROR_AGENT_PROTOCOL:
    return "LIBSSH2_ERROR_AGENT_PROTOCOL";
  case LIBSSH2_ERROR_SOCKET_RECV:
    return "LIBSSH2_ERROR_SOCKET_RECV";
  case LIBSSH2_ERROR_ENCRYPT:
    return "LIBSSH2_ERROR_ENCRYPT";
  case LIBSSH2_ERROR_BAD_SOCKET:
    return "LIBSSH2_ERROR_BAD_SOCKET";
  case LIBSSH2_ERROR_KNOWN_HOSTS:
    return "LIBSSH2_ERROR_KNOWN_HOSTS";
  }
}

// description taken from latest draft found here:
// http://tools.ietf.org/html/draft-ietf-secsh-filexfer-13
const char* SftpErrorString(int err)
{
  switch (err)
  {
  default:
    return "SFTP protocol error: Unknown error code";
  case LIBSSH2_FX_OK:
    return "SFTP protocol error: Indicates successful completion of the operation.";
  case LIBSSH2_FX_EOF:
    return "SFTP protocol error: An attempt to read past the end-of-file was made; or, there are no more directory entries to return.";
  case LIBSSH2_FX_NO_SUCH_FILE:
    return "SFTP protocol error: A reference was made to a file which does not exist.";
  case LIBSSH2_FX_PERMISSION_DENIED:
    return "SFTP protocol error: The user does not have sufficient permissions to perform the operation.";
  case LIBSSH2_FX_FAILURE:
    return "SFTP protocol error: An error occurred, but no specific error code exists to describe the failure.";
  case LIBSSH2_FX_BAD_MESSAGE:
    return "SFTP protocol error: A badly formatted packet or other SFTP protocol incompatibility was detected.";
  case LIBSSH2_FX_NO_CONNECTION:
    return "SFTP protocol error: There is no connection to the server.";
  case LIBSSH2_FX_CONNECTION_LOST:
    return "SFTP protocol error: The connection to the server was lost.  This error MAY be used locally, but MUST NOT be return by a server.";
  case LIBSSH2_FX_OP_UNSUPPORTED:
    return "SFTP protocol error: An attempted operation could not be completed by the server because the server does not support the operation.";
  case LIBSSH2_FX_INVALID_HANDLE:
    return "SFTP protocol error: The handle value was invalid.";
  case LIBSSH2_FX_NO_SUCH_PATH:
    return "SFTP protocol error: The file path does not exist or is invalid.";
  case LIBSSH2_FX_FILE_ALREADY_EXISTS:
    return "SFTP protocol error: The file already exists.";
  case LIBSSH2_FX_WRITE_PROTECT:
    return "SFTP protocol error: The file is on read-only media, or the media is write protected.";
  case LIBSSH2_FX_NO_MEDIA:
    return "SFTP protocol error: The requested operation cannot be completed because there is no media available in the drive.";
  case LIBSSH2_FX_NO_SPACE_ON_FILESYSTEM:
    return "SFTP protocol error: The requested operation cannot be completed because there is insufficient free space on the filesystem.";
  case LIBSSH2_FX_QUOTA_EXCEEDED:
    return "SFTP protocol error: The operation cannot be completed because it would exceed the user's storage quota.";
  case LIBSSH2_FX_UNKNOWN_PRINCIPAL:
    return "SFTP protocol error: A principal referenced by the request (either the 'owner', 'group', or 'who' field of an ACL), was unknown.";
  case LIBSSH2_FX_LOCK_CONFLICT:
    return "SFTP protocol error: The file could not be opened because it is locked by another process.";
  case LIBSSH2_FX_DIR_NOT_EMPTY:
    return "SFTP protocol error: The directory is not empty.";
  case LIBSSH2_FX_NOT_A_DIRECTORY:
    return "SFTP protocol error: The specified file is not a directory.";
  case LIBSSH2_FX_INVALID_FILENAME:
    return "SFTP protocol error: The filename is not valid.";
  case LIBSSH2_FX_LINK_LOOP:
    return "SFTP protocol error: Too many symbolic links encountered or, an SSH_FXF_NOFOLLOW open encountered a symbolic link as the final component";
  }
}

} // anonymous namespace

CSFTPSession::CSFTPSession(const CStdString &host, const CStdString &port, const CStdString &username, const CStdString &password)
{
  CLog::Log(LOGINFO, "SFTPSession: Creating new session on host '%s:%s' with user '%s'", host.c_str(), port.c_str(), username.c_str());
  CSingleLock lock(m_critSect);
  if (!Connect(host, port, username, password))
    Disconnect();

  m_LastActive = XbmcThreads::SystemClockMillis();
}

CSFTPSession::~CSFTPSession()
{
  CSingleLock lock(m_critSect);
  Disconnect();
}

sftp_file CSFTPSession::CreateFileHande(const CStdString &file)
{
  return CreateSFTPHandle(file, LIBSSH2_SFTP_OPENFILE);
}

void CSFTPSession::CloseFileHandle(sftp_file handle)
{
  CSingleLock lock(m_critSect);
  int rc;
  rc = libssh2_sftp_close_handle(handle);
  if (rc)
    CLog::Log(LOGERROR, "CSFTPSession::CloseFileHandle failed: %s",
              GetErrorString(rc));
}

bool CSFTPSession::GetDirectory(const CStdString &base, const CStdString &folder, CFileItemList &items)
{
  if (m_connected)
  {
    sftp_dir dir = CreateSFTPHandle(CorrectPath(folder), LIBSSH2_SFTP_OPENDIR);

    if (dir)
    {
      while (true)
      {
        char mem[512];
        char longentry[512];
        LIBSSH2_SFTP_ATTRIBUTES attrs;

        int rc = libssh2_sftp_readdir_ex(dir, mem, sizeof(mem), longentry,
                                         sizeof(longentry), &attrs);

        if (rc == 0)
          break;

        if (rc < 0 && rc != LIBSSH2_ERROR_BUFFER_TOO_SMALL)
        {
          CLog::Log(LOGERROR, "SFTPSession::GetDirectory readdir failed: %s",
                    GetErrorString(rc));
          break;
        }

        if ((strcmp(mem, "..") == 0) || (strcmp(mem, ".") == 0))
          continue;

        CStdString localPath = folder + mem;

        // if file is a symlink perform stat to gain target information
        if (attrs.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS)
          if (LIBSSH2_SFTP_S_ISLNK(attrs.permissions))
          {
            const CStdString tpath = CorrectPath(localPath);
            CSingleLock lock(m_critSect);
            if (libssh2_sftp_stat_ex(m_sftp_session, tpath.c_str(),
                                     tpath.size(), LIBSSH2_SFTP_STAT,
                                     &attrs))
              CLog::Log(LOGERROR, "SFTPSession: Failed to stat target of symlink %s",
                        localPath.c_str());
          }

        CFileItemPtr pItem(new CFileItem);
        pItem->SetLabel(mem);

        if (mem[0] == '.')
          pItem->SetProperty("file:hidden", true);

        if (attrs.flags & LIBSSH2_SFTP_ATTR_ACMODTIME)
          pItem->m_dateTime = attrs.mtime;

        if (attrs.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS)
        {
          if (LIBSSH2_SFTP_S_ISDIR(attrs.permissions))
          {
            localPath.append("/");
            pItem->m_bIsFolder = true;
            pItem->m_dwSize = 0;
          }
          else
            if (attrs.flags & LIBSSH2_SFTP_ATTR_SIZE)
              pItem->m_dwSize = attrs.filesize;
        }

        pItem->SetPath(base + localPath);
        items.Add(pItem);
      }

      libssh2_sftp_close_handle(dir);

      return true;
    }
  }
  else
    CLog::Log(LOGERROR, "SFTPSession: Not connected, can't list directory '%s'", folder.c_str());

  return false;
}

bool CSFTPSession::DirectoryExists(const char *path)
{
  bool exists = false;
  uint32_t permissions = 0;
  exists = GetItemPermissions(path, permissions);
  return exists && S_ISDIR(permissions);
}

bool CSFTPSession::FileExists(const char *path)
{
  bool exists = false;
  uint32_t permissions = 0;
  exists = GetItemPermissions(path, permissions);
  return exists && S_ISREG(permissions);
}

int CSFTPSession::Stat(const char *path, struct __stat64* buffer)
{
  if(m_connected)
  {
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    int rc = 0;
    const CStdString p = CorrectPath(path);

    m_LastActive = XbmcThreads::SystemClockMillis();

    {
      CSingleLock lock(m_critSect);
      rc = libssh2_sftp_stat_ex(m_sftp_session, p.c_str(), p.size(),
                                        LIBSSH2_SFTP_STAT, &attrs);
    }

    if (rc)
    {
      CLog::Log(LOGERROR, "SFTPSession::Stat - Failed to get attributes for '%s': %s",
                path, GetErrorString(rc));
      return -1;
    }

    memset(buffer, 0, sizeof(struct __stat64));
    if (attrs.flags & LIBSSH2_SFTP_ATTR_SIZE)
      buffer->st_size = attrs.filesize;

    if (attrs.flags & LIBSSH2_SFTP_ATTR_ACMODTIME)
    {
      buffer->st_mtime = attrs.mtime;
      buffer->st_atime = attrs.atime;
    }

    if (attrs.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS)
    {
      if LIBSSH2_SFTP_S_ISDIR(attrs.permissions)
        buffer->st_mode = _S_IFDIR;
      else if LIBSSH2_SFTP_S_ISREG(attrs.permissions)
        buffer->st_mode = _S_IFREG;
    }

    return 0;
  }
  else
  {
    CLog::Log(LOGERROR, "SFTPSession::Stat - Failed because not connected for '%s'", path);
    return -1;
  }
}

int CSFTPSession::Seek(sftp_file handle, uint64_t position)
{
  CSingleLock lock(m_critSect);
  m_LastActive = XbmcThreads::SystemClockMillis();
  libssh2_sftp_seek64(handle, position);
  return 0;
}

int CSFTPSession::Read(sftp_file handle, char *buffer, size_t length)
{
  CSingleLock lock(m_critSect);
  m_LastActive = XbmcThreads::SystemClockMillis();
  return libssh2_sftp_read(handle, buffer, length);
}

int64_t CSFTPSession::GetPosition(sftp_file handle)
{
  CSingleLock lock(m_critSect);
  m_LastActive = XbmcThreads::SystemClockMillis();
  return libssh2_sftp_tell64(handle);
}

bool CSFTPSession::IsIdle()
{
  return (XbmcThreads::SystemClockMillis() - m_LastActive) > 90000;
}

bool CSFTPSession::VerifyKnownHost(ssh_session session)
{
  // disabled for now, needs changes in UI to be done correctly
  return true;
  /*
  switch (ssh_is_server_known(session))
  {
    case SSH_SERVER_KNOWN_OK:
      return true;
    case SSH_SERVER_KNOWN_CHANGED:
      CLog::Log(LOGERROR, "SFTPSession: Server that was known has changed");
      return false;
    case SSH_SERVER_FOUND_OTHER:
      CLog::Log(LOGERROR, "SFTPSession: The host key for this server was not found but an other type of key exists. An attacker might change the default server key to confuse your client into thinking the key does not exist");
      return false;
    case SSH_SERVER_FILE_NOT_FOUND:
      CLog::Log(LOGINFO, "SFTPSession: Server file was not found, creating a new one");
    case SSH_SERVER_NOT_KNOWN:
      CLog::Log(LOGINFO, "SFTPSession: Server unkown, we trust it for now");
      if (ssh_write_knownhost(session) < 0)
      {
        CLog::Log(LOGERROR, "CSFTPSession: Failed to save host '%s'", strerror(errno));
        return false;
      }

      return true;
    case SSH_SERVER_ERROR:
      CLog::Log(LOGERROR, "SFTPSession: Failed to verify host '%s'", ssh_get_error(session));
      return false;
  }

  return false;
  */
}

namespace
{

int init_socket(const char* host, const char* port)
{
  int m_socket;
  struct addrinfo hints;
  struct addrinfo *result;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  int s = getaddrinfo(host, port, &hints, &result);
  if (s != 0)
  {
    CLog::Log(LOGERROR, "SFTPSession::Connect: getaddrinfo failed: %s",
              gai_strerror(s));
    return -1;
  }

  struct addrinfo *rp = result;
  for (; rp != NULL; rp = rp->ai_next)
  {
    m_socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (m_socket == -1)
      continue;

    if (connect(m_socket, rp->ai_addr, rp->ai_addrlen) != -1)
      break; // successfull connection

    // connection failure, try next
    close(m_socket);
  }

  freeaddrinfo(result);

  if (rp == NULL)
  {
    CLog::Log(LOGERROR, "SFTPSession::Connect: Found no host to connect");
    return -1;
  }

  return m_socket;
}

ssh_session
init_ssh_session()
{
  ssh_session session = libssh2_session_init();

  if (!session)
  {
    CLog::Log(LOGERROR, "SFTPSession::Connect: Failed to create SSH session");
    return NULL;
  }

  // we work in blocking operation
  libssh2_session_set_blocking(session, 1);
  libssh2_session_set_timeout(session, SFTP_TIMEOUT);

  return session;
}

} // anonymous namespace

bool CSFTPSession::Connect(const CStdString &host, const CStdString &port, const CStdString &username, const CStdString &password)
{
  CSingleLock lock(m_critSect);

  int rc;
  m_socket        = 0;
  m_connected     = false;
  m_session       = NULL;
  m_sftp_session  = NULL;

  m_socket = init_socket(host.c_str(), port.c_str());
  if (m_socket < 0)
    return false;
  m_session = init_ssh_session();
  if (!m_session)
    return false;

  rc = libssh2_session_handshake(m_session, m_socket);
  if (rc)
  {
    CLog::Log(LOGERROR, "CSFTPSession::Connect: Session handshake failed: %s",
              GetErrorString(rc));
    libssh2_session_free(m_session);
    m_session = NULL;
    return false;
  }

  // TODO: Perform hostkey verification

  // TODO: Implement public key authentication via agent

  rc = libssh2_userauth_password_ex(m_session, username.c_str(),
                                    username.size(), password.c_str(),
                                    password.size(), NULL);
  if (rc)
  {
    if (rc == LIBSSH2_ERROR_AUTHENTICATION_FAILED)
      CLog::Log(LOGINFO, "CSFTPSession::Connect: Wrong username/password");
    else
      CLog::Log(LOGERROR, "CSFTPSession::Connect: Error during password authentication %s",
                GetErrorString(rc));

    return false;
  }

  m_sftp_session = libssh2_sftp_init(m_session);

  if (!m_sftp_session)
  {
    int err = libssh2_session_last_errno(m_session);
    CLog::Log(LOGERROR, "CSFTPSession::Connect: Failed to init SFTP session: %s",
              GetErrorString(err));
    return false;
  }

  m_connected = true;
  return true;
}

void CSFTPSession::Disconnect()
{
  CLog::Log(LOGINFO, "Disconnecting session");
  CSingleLock lock(m_critSect);
  if (m_sftp_session)
    libssh2_sftp_shutdown(m_sftp_session);
  m_sftp_session = NULL;

  if (m_session)
  {
    libssh2_session_disconnect(m_session, "Good bye");
    libssh2_session_free(m_session);
  }
  m_session = NULL;

  if (m_socket >= 0)
    close(m_socket);
  m_socket = -1;
}

/*!
 \brief Gets POSIX compatible permissions information about the specified file or directory.
 \param path Remote SSH path to the file or directory.
 \param permissions POSIX compatible permissions information for the file or directory (if it exists). i.e. can use macros S_ISDIR() etc.
 \return Returns \e true, if it was possible to get permissions for the file or directory, \e false otherwise.
 */
bool CSFTPSession::GetItemPermissions(const char *path, uint32_t &permissions)
{
  const CStdString cpath = CorrectPath(path);
  LIBSSH2_SFTP_ATTRIBUTES attrs;
  int rc;

  {
    CSingleLock lock(m_critSect);
    rc = libssh2_sftp_stat_ex(m_sftp_session, cpath.c_str(), cpath.size(),
                              LIBSSH2_SFTP_STAT, &attrs);
  }

  if (rc)
    return false;

  if (attrs.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS)
  {
    permissions = attrs.permissions;
    return true;
  }
  else
    return false;
}

LIBSSH2_SFTP_HANDLE*
CSFTPSession::CreateSFTPHandle(const CStdString& file, int open_type)
{
  if (m_connected)
  {
    const CStdString path = CorrectPath(file);
    m_LastActive = XbmcThreads::SystemClockMillis();

    sftp_file handle = NULL;
    CSingleLock lock(m_critSect);

    int err = 0;
    handle = libssh2_sftp_open_ex(m_sftp_session, path.c_str(),
                                  path.size(), LIBSSH2_FXF_READ,
                                  0, open_type);

    if (!handle)
      err = libssh2_session_last_errno(m_session);

    if (err)
      CLog::Log(LOGERROR, "SFTPSession:: Failed to create handle for file %s: %s",
                file.c_str(), GetErrorString(err));

    return handle;
  }
  else
    CLog::Log(LOGERROR, "SFTPSession: Not connected and can't create file handle for '%s'", file.c_str());

  return NULL;
}

const char*
CSFTPSession::GetErrorString(int rc)
{
  if (rc != LIBSSH2_ERROR_SFTP_PROTOCOL)
    return Libssh2ErrorString(rc);

  {
    CSingleLock lock(m_critSect);
    rc = libssh2_sftp_last_error(m_sftp_session);
  }

  return SftpErrorString(rc);
}

#endif
