/*
 *      Copyright (C) 2011-2013 Team XBMC
 *      http://www.xbmc.org
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

#include "HTTPSpecialHandler.h"
#include "MediaSource.h"
#include "URL.h"
#include "filesystem/File.h"
#include "network/WebServer.h"
#include "settings/Settings.h"
#include "utils/URIUtils.h"
#include "utils/log.h"

using namespace std;

bool CHTTPSpecialHandler::CheckHTTPRequest(const HTTPRequest &request)
{
  CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::CheckHTTPRequest: RequestUrl %s", request.url.c_str());
  return (request.url.find("/special") == 0 || request.url.find("/log") == 0);
}

int CHTTPSpecialHandler::HandleHTTPRequest(const HTTPRequest &request)
{
  CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::HandleHTTPRequest: RequestUrl %s", request.url.c_str());
  if (request.url.size() > 9)
  {
    m_path = request.url.substr(9);

    if (XFILE::CFile::Exists(m_path))
    {
      bool accessible = false;
      if (m_path.substr(0, 10) == "special://")
      {
    	  m_responseCode = MHD_HTTP_OK;
          m_responseType = HTTPSpecialDownload;
      }
      else
      {
        m_responseCode = MHD_HTTP_UNAUTHORIZED;
        m_responseType = HTTPError;
      }
    }
  }
  else
  {
    m_responseCode = MHD_HTTP_BAD_REQUEST;
    m_responseType = HTTPError;
  }

  return MHD_YES;
}
