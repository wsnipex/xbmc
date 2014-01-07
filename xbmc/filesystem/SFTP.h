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
#include <libssh2.h>
#include <libssh2_sftp.h>
#include <boost/shared_ptr.hpp>

typedef LIBSSH2_SESSION* ssh_session;
typedef LIBSSH2_SFTP* sftp_session;
typedef LIBSSH2_SFTP_ATTRIBUTES* sftp_attributes;
typedef LIBSSH2_SFTP_HANDLE* sftp_file;
typedef LIBSSH2_SFTP_HANDLE* sftp_dir;

//five secs timeout for SFTP (in milliseconds)
#define SFTP_TIMEOUT 5000

class CSFTPSession;
class CSFTPSessionManager;
typedef boost::shared_ptr<CSFTPSession> CSFTPSessionPtr;

#endif
