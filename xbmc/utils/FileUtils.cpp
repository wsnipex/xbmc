/*
 *  Copyright (C) 2010-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */
#include <vector>

#include "FileUtils.h"
#include "ServiceBroker.h"
#include "guilib/GUIKeyboardFactory.h"
#include "utils/log.h"
#include "guilib/LocalizeStrings.h"
#include "JobManager.h"
#include "FileOperationJob.h"
#include "URIUtils.h"
#include "filesystem/MultiPathDirectory.h"
#include "filesystem/SpecialProtocol.h"
#include "filesystem/StackDirectory.h"
#include "settings/MediaSourceSettings.h"
#include "Util.h"
#include "StringUtils.h"
#include "URL.h"
#include "settings/Settings.h"
#include "utils/Variant.h"

#if defined(TARGET_WINDOWS)
#include "platform/win32/WIN32Util.h"
#include "utils/CharsetConverter.h"
#endif

using namespace XFILE;

bool CFileUtils::DeleteItem(const std::string &strPath)
{
  CFileItemPtr item(new CFileItem(strPath));
  item->SetPath(strPath);
  item->m_bIsFolder = URIUtils::HasSlashAtEnd(strPath);
  item->Select(true);
  return DeleteItem(item);
}

bool CFileUtils::DeleteItem(const CFileItemPtr &item)
{
  if (!item || item->IsParentFolder())
    return false;

  // Create a temporary item list containing the file/folder for deletion
  CFileItemPtr pItemTemp(new CFileItem(*item));
  pItemTemp->Select(true);
  CFileItemList items;
  items.Add(pItemTemp);

  // grab the real filemanager window, set up the progress bar,
  // and process the delete action
  CFileOperationJob op(CFileOperationJob::ActionDelete, items, "");

  return op.DoWork();
}

bool CFileUtils::RenameFile(const std::string &strFile)
{
  std::string strFileAndPath(strFile);
  URIUtils::RemoveSlashAtEnd(strFileAndPath);
  std::string strFileName = URIUtils::GetFileName(strFileAndPath);
  std::string strPath = URIUtils::GetDirectory(strFileAndPath);
  if (CGUIKeyboardFactory::ShowAndGetInput(strFileName, CVariant{g_localizeStrings.Get(16013)}, false))
  {
    strPath = URIUtils::AddFileToFolder(strPath, strFileName);
    CLog::Log(LOGINFO,"FileUtils: rename %s->%s\n", strFileAndPath.c_str(), strPath.c_str());
    if (URIUtils::IsMultiPath(strFileAndPath))
    { // special case for multipath renames - rename all the paths.
      std::vector<std::string> paths;
      CMultiPathDirectory::GetPaths(strFileAndPath, paths);
      bool success = false;
      for (unsigned int i = 0; i < paths.size(); ++i)
      {
        std::string filePath(paths[i]);
        URIUtils::RemoveSlashAtEnd(filePath);
        filePath = URIUtils::GetDirectory(filePath);
        filePath = URIUtils::AddFileToFolder(filePath, strFileName);
        if (CFile::Rename(paths[i], filePath))
          success = true;
      }
      return success;
    }
    return CFile::Rename(strFileAndPath, strPath);
  }
  return false;
}

bool CFileUtils::RemoteAccessAllowed(const std::string &strPath)
{
  std::string SourceNames[] = { "programs", "files", "video", "music", "pictures" };

  std::string realPath = URIUtils::GetRealPath(strPath);
  // for rar:// and zip:// paths we need to extract the path to the archive
  // instead of using the VFS path
  while (URIUtils::IsInArchive(realPath))
    realPath = CURL(realPath).GetHostName();

  if (StringUtils::StartsWithNoCase(realPath, "virtualpath://upnproot/"))
    return true;
  else if (StringUtils::StartsWithNoCase(realPath, "musicdb://"))
    return true;
  else if (StringUtils::StartsWithNoCase(realPath, "videodb://"))
    return true;
  else if (StringUtils::StartsWithNoCase(realPath, "library://video"))
    return true;
  else if (StringUtils::StartsWithNoCase(realPath, "library://music"))
    return true;
  else if (StringUtils::StartsWithNoCase(realPath, "sources://video"))
    return true;
  else if (StringUtils::StartsWithNoCase(realPath, "special://musicplaylists"))
    return true;
  else if (StringUtils::StartsWithNoCase(realPath, "special://profile/playlists"))
    return true;
  else if (StringUtils::StartsWithNoCase(realPath, "special://videoplaylists"))
    return true;
  else if (StringUtils::StartsWithNoCase(realPath, "special://skin"))
    return true;
  else if (StringUtils::StartsWithNoCase(realPath, "special://profile/addon_data"))
    return true;
  else if (StringUtils::StartsWithNoCase(realPath, "addons://sources"))
    return true;
  else if (StringUtils::StartsWithNoCase(realPath, "upnp://"))
    return true;
  else if (StringUtils::StartsWithNoCase(realPath, "plugin://"))
    return true;
  else
  {
    std::string strPlaylistsPath = CServiceBroker::GetSettings()->GetString(CSettings::SETTING_SYSTEM_PLAYLISTSPATH);
    URIUtils::RemoveSlashAtEnd(strPlaylistsPath);
    if (StringUtils::StartsWithNoCase(realPath, strPlaylistsPath))
      return true;
  }
  bool isSource;
  for (const std::string& sourceName : SourceNames)
  {
    VECSOURCES* sources = CMediaSourceSettings::GetInstance().GetSources(sourceName);
    int sourceIndex = CUtil::GetMatchingSource(realPath, *sources, isSource);
    if (sourceIndex >= 0 && sourceIndex < (int)sources->size() && sources->at(sourceIndex).m_iHasLock != 2 && sources->at(sourceIndex).m_allowSharing)
      return true;
  }
  return false;
}

CDateTime CFileUtils::GetModificationDate(const std::string& strFileNameAndPath, const bool& bUseLatestDate)
{
  CDateTime dateAdded;
  if (strFileNameAndPath.empty())
  {
    CLog::Log(LOGDEBUG, "%s empty strFileNameAndPath variable", __FUNCTION__);
    return dateAdded;
  }

  try
  {
    std::string file = strFileNameAndPath;
    if (URIUtils::IsStack(strFileNameAndPath))
      file = CStackDirectory::GetFirstStackedFile(strFileNameAndPath);

    if (URIUtils::IsInArchive(file))
      file = CURL(file).GetHostName();

    // Let's try to get the modification datetime
    struct __stat64 buffer;
    if (CFile::Stat(file, &buffer) == 0 && (buffer.st_mtime != 0 || buffer.st_ctime != 0))
    {
      time_t now = time(NULL);
      time_t addedTime;
      // Prefer the modification time if it's valid
      if (!bUseLatestDate)
      {
        if (buffer.st_mtime != 0 && (time_t)buffer.st_mtime <= now)
          addedTime = (time_t)buffer.st_mtime;
        else
          addedTime = (time_t)buffer.st_ctime;
      }
      // Use the newer of the creation and modification time
      else
      {
        addedTime = std::max((time_t)buffer.st_ctime, (time_t)buffer.st_mtime);
        // if the newer of the two dates is in the future, we try it with the older one
        if (addedTime > now)
          addedTime = std::min((time_t)buffer.st_ctime, (time_t)buffer.st_mtime);
      }

      // make sure the datetime does is not in the future
      if (addedTime <= now)
      {
        struct tm *time;
#ifdef HAVE_LOCALTIME_R
        struct tm result = {};
        time = localtime_r(&addedTime, &result);
#else
        time = localtime(&addedTime);
#endif
        if (time)
          dateAdded = *time;
      }
    }
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s unable to extract modification date for file (%s)", __FUNCTION__, strFileNameAndPath.c_str());
  }
  return dateAdded;
}

bool CFileUtils::CheckFileAccessAllowed(const std::string &filePath)
{
  // DENY access to paths matching
  const std::vector<std::string> blacklist = {
    "passwords.xml",
    "sources.xml",
    "guisettings.xml",
    "advancedsettings.xml",
    "/.ssh/",
  };
  // ALLOW kodi paths
  // TODO: most likely many more needed
  const std::vector<std::string> whitelist = {
    CSpecialProtocol::TranslatePath("special://home"),
    CSpecialProtocol::TranslatePath("special://xbmc")
  };

  // image urls come in the form of image://... sometimes with a / appended at the end
  // strip this off to get the real file path
  bool isImage = false;
  std::string decodePath = CURL::Decode(filePath);
  size_t pos = decodePath.find("image://");
  if (pos != std::string::npos)
  {
    isImage = true;
    decodePath.erase(pos, 8);
    URIUtils::RemoveSlashAtEnd(decodePath);
  }

  // check blacklist
  for (const auto &b : blacklist)
  {
    if (decodePath.find(b) != std::string::npos)
    {
      CLog::Log(LOGERROR,"%s denied access to %s",  __FUNCTION__, decodePath.c_str());
      return false;
    }
  }

#if defined(TARGET_POSIX)
  std::string whiteEntry;
  char *fullpath = realpath(decodePath.c_str(), nullptr);

  // if this is a locally existing file, check access permissions
  if (fullpath)
  {
    const std::string realPath = fullpath;
    free(fullpath);

    CLog::Log(LOGDEBUG,"%s filePath: %s decodePath: %s realpath: %s",  __FUNCTION__, filePath.c_str(), decodePath.c_str(), realPath.c_str()); // TODO: remove

    // check whitelist
    for (const auto &w : whitelist)
    {
      char *realtemp = realpath(w.c_str(), nullptr);
      if (realtemp)
      {
        whiteEntry = realtemp;
        free(realtemp);
      }
      if (StringUtils::StartsWith(realPath, whiteEntry))
        return true;
    }
    // check sources with realPath
    return CFileUtils::RemoteAccessAllowed(realPath);
  }
#elif defined(TARGET_WINDOWS)
  CURL url(decodePath);
  if (url.GetProtocol().empty())
  {
    std::wstring decodePathW;
    g_charsetConverter.utf8ToW(decodePath, decodePathW, false);
    CWIN32Util::AddExtraLongPathPrefix(decodePathW);
    DWORD bufSize = GetFullPathNameW(decodePathW.c_str(), 0, nullptr, nullptr);
    if (bufSize > 0)
    {
      std::wstring fullpathW;
      fullpathW.resize(bufSize);
      if (GetFullPathNameW(decodePathW.c_str(), bufSize, const_cast<wchar_t*>(fullpathW.c_str()), nullptr) <= bufSize - 1)
      {
        CWIN32Util::RemoveExtraLongPathPrefix(fullpathW);
        std::string fullpath;
        g_charsetConverter.wToUTF8(fullpathW, fullpath, false);
        for (const std::string& whiteEntry : whitelist)
        {
          if (StringUtils::StartsWith(fullpath, whiteEntry))
            return true;
        }
        return CFileUtils::RemoteAccessAllowed(fullpath);
      }
    }
  }
#endif
  // if it isn't a local file, it must be a vfs entry
  CLog::Log(LOGDEBUG,"%s filePath: %s decodePath: %s",  __FUNCTION__, filePath.c_str(), decodePath.c_str()); // TODO: remove
  if (! isImage)
    return CFileUtils::RemoteAccessAllowed(decodePath);
  return true;
}
