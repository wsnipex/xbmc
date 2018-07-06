 /*
 *      Copyright (C) 2012-2017 Team Kodi
 *      http://kodi.tv
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
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "SavestateDatabase.h"
#include "SavestateFlatBuffer.h"
#include "SavestateUtils.h"
#include "filesystem/File.h"
#include "utils/log.h"
#include "URL.h"

using namespace KODI;
using namespace RETRO;

CSavestateDatabase::CSavestateDatabase() = default;

std::unique_ptr<ISavestate> CSavestateDatabase::CreateSavestate()
{
  std::unique_ptr<ISavestate> savestate;

  savestate.reset(new CSavestateFlatBuffer);

  return savestate;
}

bool CSavestateDatabase::AddSavestate(const std::string &gamePath, const ISavestate& save)
{
  bool bSuccess = false;

  const std::string savestatePath = CSavestateUtils::MakePath(gamePath);

  CLog::Log(LOGDEBUG, "Saving savestate to %s", CURL::GetRedacted(savestatePath).c_str());

  const uint8_t *data = nullptr;
  size_t size = 0;
  if (save.Serialize(data, size))
  {
    XFILE::CFile file;
    if (file.OpenForWrite(savestatePath))
    {
      const ssize_t written = file.Write(data, size);
      if (written == static_cast<ssize_t>(size))
      {
        CLog::Log(LOGDEBUG, "Wrote savestate of %u bytes", size);
        bSuccess = true;
      }
    }
    else
      CLog::Log(LOGERROR, "Failed to open savestate for writing");
  }

  return bSuccess;
}

bool CSavestateDatabase::GetSavestate(const std::string& gamePath, ISavestate& save)
{
  bool bSuccess = false;

  const std::string savestatePath = CSavestateUtils::MakePath(gamePath);

  CLog::Log(LOGDEBUG, "Loading savestate from %s", CURL::GetRedacted(savestatePath).c_str());

  std::vector<uint8_t> savestateData;

  XFILE::CFile savestateFile;
  if (savestateFile.Open(savestatePath, XFILE::READ_TRUNCATED))
  {
    int64_t size = savestateFile.GetLength();
    if (size > 0)
    {
      savestateData.resize(static_cast<size_t>(size));

      const ssize_t readLength = savestateFile.Read(savestateData.data(), savestateData.size());
      if (readLength != static_cast<ssize_t>(savestateData.size()))
      {
        CLog::Log(LOGERROR, "Failed to read savestate %s of size %d bytes",
          CURL::GetRedacted(savestatePath).c_str(),
          size);
        savestateData.clear();
      }
    }
    else
      CLog::Log(LOGERROR, "Failed to get savestate length: %s", CURL::GetRedacted(savestatePath).c_str());
  }
  else
    CLog::Log(LOGERROR, "Failed to open savestate file %s", CURL::GetRedacted(savestatePath).c_str());

  if (!savestateData.empty())
    bSuccess = save.Deserialize(std::move(savestateData));

  return bSuccess;
}

bool CSavestateDatabase::GetSavestatesNav(CFileItemList& items, const std::string& gamePath, const std::string& gameClient /* = "" */)
{
  //! @todo
  return false;
}

bool CSavestateDatabase::RenameSavestate(const std::string& path, const std::string& label)
{
  //! @todo
  return false;
}

bool CSavestateDatabase::DeleteSavestate(const std::string& path)
{
  //! @todo
  return false;
}

bool CSavestateDatabase::ClearSavestatesOfGame(const std::string& gamePath, const std::string& gameClient /* = "" */)
{
  //! @todo
  return false;
}
