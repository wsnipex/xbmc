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
#include "utils/RegExp.h"
#include "utils/StringUtils.h"

using namespace std;

bool CHTTPSpecialHandler::CheckHTTPRequest(const HTTPRequest &request)
{
  return (request.url.find("/special/") == 0 || request.url.find("/log") == 0);
}


/*
first check if it's a "special://" path
if not return an error (as you already do)
then check if the file exists
then check if it's a .log or .xml. if not, you can just let libmicrohttpd do a file download
*/
int CHTTPSpecialHandler::HandleHTTPRequest(const HTTPRequest &request)
{
  CStdString ext;
  CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::HandleHTTPRequest: RequestUrl %s", request.url.c_str());

  if (request.url.substr(0, 19) == "/special/special://")
  {
    m_path = request.url.substr(9);
    ext = URIUtils::GetExtension(request.url);
    ext = ext.ToLower();
  }
  else if (request.url.substr(0, 4) == "/log" && request.url.size() <= 5)
  {
    m_path = "special://temp/xbmc.log";
    ext = ".log";
  }
  else
  {
    // we should not land here
    CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::HandleHTTPRequest: substring %s", request.url.substr(0, 19).c_str());
    m_response = "400 Bad Request";
    m_responseCode = MHD_HTTP_BAD_REQUEST;
    m_responseType = HTTPError;
    return MHD_YES;
  }

  //CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::HandleHTTPRequest: file is %s", m_path.c_str());

  XFILE::CFile *file = new XFILE::CFile();
  if (XFILE::CFile::Exists(m_path) && file->Open(m_path, READ_CACHED))
  //if (XFILE::CFile::Exists(m_path) && file->Open(m_path, READ_NO_CACHE))
  {
    if (ext == ".log" || ext == ".xml")
    {
      m_response = "";
      memset(m_buf,'\0',sizeof(m_buf));
      std::string line = "";
      std::string user;
      std::string pass;
      std::string replace;

      CRegExp regex1, regex2;
      regex1.RegComp("://(.*):(.*)@");
      regex2.RegComp("<[^<>]*pass[^<>]*>([^<]+)</.*pass.*>");

      while (file->ReadString(m_buf, 1023))
      {
        line = m_buf;
        if (regex1.RegFind(line))
        {
          user = regex1.GetReplaceString("\\1");
          pass = regex1.GetReplaceString("\\2");
          replace = string("://") + string(user) + ":" + string(pass) + "@";
          StringUtils::Replace(line, replace, "://xxx:xxx@");
        }
        if (regex2.RegFind(line))
        {
          replace = regex2.GetReplaceString("\\1");
          if (replace.length() > 0)
            StringUtils::Replace(line, replace, "xxx");
        }
        m_response += line;
        memset(m_buf,'\0',sizeof(m_buf));
      }

      const char *mime = request.webserver->CreateMimeTypeFromExtension(ext.c_str());
      if (mime)
        m_responseHeaderFields.insert(pair<string, string>("Content-Type", mime));

      file->Close();
      delete file;
      m_request.clear();

      //CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::HandleHTTPRequest: m_response %s", m_response.c_str());
      m_responseCode = MHD_HTTP_OK;
      m_responseType = HTTPMemoryDownloadNoFreeCopy;
    }
    else
    {
      m_responseCode = MHD_HTTP_OK;
      m_responseType = HTTPFileDownload;
    }
  }
  else
  {
      m_responseCode = MHD_HTTP_NOT_FOUND;
      m_responseType = HTTPError;
  }
  return MHD_YES;
}
