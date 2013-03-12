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
  return (request.url.find("/special") == 0 || request.url.find("/log") == 0);
}

int CHTTPSpecialHandler::HandleHTTPRequest(const HTTPRequest &request)
{
  CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::HandleHTTPRequest: RequestUrl %s", request.url.c_str());
  if (request.url.size() > 9)
  {
    m_path = request.url.substr(9);
    CStdString ext = URIUtils::GetExtension(request.url);
    ext = ext.ToLower();
    XFILE::CFile *file = new XFILE::CFile();
    CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::HandleHTTPRequest: RequestUrlSize %d", request.url.size());
    if (XFILE::CFile::Exists(m_path) && file->Open(m_path, READ_NO_CACHE))
    {
      if (m_path.substr(0, 10) == "special://")
      {
    	CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::HandleHTTPRequest: File is readable and path = special://");
    	if (ext == ".log" || ext == ".xml")
    	{
    	  CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::HandleHTTPRequest: extension is %s", ext.c_str());
    	  CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::HandleHTTPRequest: file length %ld", file->GetLength());

    	  m_response = "";
    	  /*
          char* buf = new char[file->GetLength()+1];
          unsigned res = file->Read(buf, file->GetLength());

    	  CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::HandleHTTPRequest: readfile res %d", res);
    	  CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::HandleHTTPRequest: buf %s", buf);
    	  if(res == 0)
    	    return -1;

    	  CRegExp regex;
    	  regex.RegComp("://(.*):(.*)@");
    	  regex.RegFind(buf);
    	  std::string user = regex.GetReplaceString ("\\1");
    	  std::string pass = regex.GetReplaceString ("\\2");
    	  std::string replace = string("://") + string(user) + ":" + string(pass) + "@";
    	  m_response = buf;
    	  StringUtils::Replace(m_response, replace, "://xxx:xxx@");
    	  CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::HandleHTTPRequest: m_response %s", m_response.c_str());
    	  CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::HandleHTTPRequest: after m_response");
    	  //strcpy(m_response, x);
    	  CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::HandleHTTPRequest: buf[-1] %s", buf[-1]);
    	  m_responseCode = MHD_HTTP_OK;
          m_responseType = HTTPMemoryDownloadNoFreeNoCopy;
    	  delete[] buf;
    	  buf = NULL;
    	  CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::HandleHTTPRequest: response size %ld", m_response.size());
    	  */
    	  char buf[1024];
    	  while (file->ReadString(buf, 1023))
    	  {
    	    m_response += buf;
    	  }
    	  //CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::HandleHTTPRequest: m_response before regex %s", m_response.c_str());
    	  CRegExp regex;
    	  regex.RegComp("://(.*):(.*)@");
    	  regex.RegFind(m_response);
    	  std::string user = regex.GetReplaceString ("\\1");
    	  std::string pass = regex.GetReplaceString ("\\2");
    	  std::string replace = string("://") + string(user) + ":" + string(pass) + "@";
    	  StringUtils::Replace(m_response, replace, "://xxx:xxx@");
    	  //CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::HandleHTTPRequest: m_response %s", m_response.c_str());

    	  m_responseCode = MHD_HTTP_OK;
          m_responseType = HTTPMemoryDownloadNoFreeNoCopy;

    	}
    	else
    	{
    	  CLog::Log(LOGDEBUG, "CHTTPSpecialHandler::HandleHTTPRequest: extension is %s", ext.c_str());
          m_responseCode = MHD_HTTP_OK;
          m_responseType = HTTPFileDownload;
    	}
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
