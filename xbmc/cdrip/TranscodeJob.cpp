/*
 *      Copyright (C) 2012-2013 Team XBMC
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

#include "TranscodeJob.h"
#include "Encoder.h"
#include "EncoderFFmpeg.h"
#include "FileItem.h"
#include "utils/log.h"
#include "Util.h"
#include "dialogs/GUIDialogExtendedProgressBar.h"
#include "filesystem/File.h"
#include "filesystem/SpecialProtocol.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/LocalizeStrings.h"
#include "settings/Settings.h"
#include "settings/AdvancedSettings.h"
#include "utils/StringUtils.h"
#include "addons/AddonManager.h"
#include "addons/AudioEncoder.h"
#include "cores/paplayer/AudioDecoder.h"
#include "cores/AudioEngine/Utils/AEUtil.h"

using namespace ADDON;
using namespace MUSIC_INFO;
using namespace XFILE;

CTranscodeJob::CTranscodeJob(const CFileItem& input,
                             const std::string& output,
                             int encoder) :
  m_input(input), m_output(CUtil::MakeLegalPath(output)), m_encoder(encoder)
{
}

CTranscodeJob::~CTranscodeJob()
{
}

bool CTranscodeJob::DoWork()
{
  CLog::Log(LOGINFO, "Start transcoding track %s to %s", m_input.GetPath().c_str(),
                                                         m_output.c_str());

  // if we are ripping to a samba share, rip it to hd first and then copy it it the share
  std::string outPath(m_output);
  if (URIUtils::IsRemote(outPath))
    outPath = SetupTempFile();

  if (outPath.empty())
  {
    CLog::Log(LOGERROR, "Transcoder: Error opening file");
    return false;
  }

  // initialise our audio decoder
  CFile reader;
  CAudioDecoder decoder;

  if (!decoder.Create(m_input, (m_input.m_lStartOffset * 1000) / 75))
  {
    CLog::Log(LOGERROR, "Transcoder: unable to setup decoder");
    return false;
  }

  // read the tag info
  m_input.LoadMusicTag();
  if (!m_input.HasMusicInfoTag())
  {
    CLog::Log(LOGERROR, "Transcoder: no music tag");
    return false;
  }

  // start decoding so we're ready to go
  decoder.Start();
  while (!decoder.GetDataSize())
  {
    int status = decoder.GetStatus();
    if (status == STATUS_ENDED   ||
        status == STATUS_NO_FILE ||
        decoder.ReadSamples(PACKET_SIZE) == RET_ERROR)
    {
      CLog::Log(LOGINFO, "Transcoder: Error reading samples");
      return false;
    }
  }

  // read the file info
  CAEChannelInfo channels;
  AEDataFormat   format;
  unsigned int   samplerate;
  decoder.GetDataFormat(&channels, &samplerate, NULL, &format);
  unsigned int   bps = CAEUtil::DataFormatToBits(format);

  // initialize the encoder
  CEncoder* encoder = SetupEncoder(outPath, m_input.GetMusicInfoTag(), samplerate, channels.Count(), bps);
  if (!encoder)
  {
    CLog::Log(LOGERROR, "Error: unable to setup encoder");
    return false;
  }

  // setup the progress dialog
  CGUIDialogExtendedProgressBar* pDlgProgress = 
      (CGUIDialogExtendedProgressBar*)g_windowManager.GetWindow(WINDOW_DIALOG_EXT_PROGRESS);
  CGUIDialogProgressBarHandle* handle = pDlgProgress->GetHandle(g_localizeStrings.Get(605));

  std::string strLine = StringUtils::Format("%02i. %s - %s", m_input.GetMusicInfoTag()->GetTrackNumber(),
                                            StringUtils::Join(m_input.GetMusicInfoTag()->GetArtist(), g_advancedSettings.m_musicItemSeparator).c_str(),
                                            m_input.GetMusicInfoTag()->GetTitle().c_str());
  handle->SetText(strLine);

  // progress tracking
  m_currentSamples = 0;
  m_totalSamples = decoder.TotalTime() * samplerate / 1000 * channels.Count();

  // start transcoding
  int percent=0;
  int oldpercent=0;
  bool cancelled(false);
  int result;
  while (!cancelled && (result=TranscodeChunk(decoder, encoder, percent)) == 0)
  {
//    cancelled = ShouldCancel(percent,100);
    if (percent > oldpercent)
    {
      oldpercent = percent;
      handle->SetPercentage(static_cast<float>(percent));
    }
  }

  // close encoder
  encoder->CloseEncode();
  delete encoder;

  // close the decoder
  reader.Close();

  m_output = URIUtils::ReplaceExtension(m_output, URIUtils::GetExtension(outPath));
  if (outPath != m_output && !cancelled && result == 2)
  {
    // copy the ripped track to the share
    if (!CFile::Copy(outPath, m_output))
    {
      CLog::Log(LOGERROR, "Audiotranscoder: Error copying file from %s to %s",
                outPath.c_str(), m_output.c_str());
      CFile::Delete(outPath);
      return false;
    }
    // delete cached file
    CFile::Delete(outPath);
  }

  if (cancelled)
  {
    CLog::Log(LOGWARNING, "User Cancelled Transcoding");
    CFile::Delete(m_output);
  }
  else if (result == 1)
    CLog::Log(LOGERROR, "Transcoder: Error decoding %s", m_input.GetPath().c_str());
  else if (result < 0)
    CLog::Log(LOGERROR, "Transcoder: Error encoding %s", m_input.GetPath().c_str());
  else
    CLog::Log(LOGINFO, "Finished transcoding %s", m_input.GetPath().c_str());

  handle->MarkFinished();

  return !cancelled && result == 2;
}

int CTranscodeJob::TranscodeChunk(CAudioDecoder& decoder, CEncoder* encoder, int& percent)
{
  percent = 0;

  const unsigned int packet_samples = 1024;

  // read from the decoder (measured in samples)
  while (decoder.GetDataSize() < packet_samples)
  {
    int status = decoder.GetStatus();
    if (status == STATUS_ENDED)
      break;
    if (status == STATUS_NO_FILE ||
        decoder.ReadSamples(PACKET_SIZE) == RET_ERROR)
    {
      CLog::Log(LOGINFO, "Transcoder: Error reading samples");
      return 1;
    }
  }
  unsigned int read_samples = decoder.GetDataSize();
  if (!read_samples) // finished
    return 2;

  // encode data
  AEDataFormat format;
  decoder.GetDataFormat(NULL, NULL, NULL, &format);
  unsigned int bytes = read_samples * (CAEUtil::DataFormatToBits(format) >> 3);

  int encres = encoder->Encode(bytes, (uint8_t *)decoder.GetData(read_samples));

  // Get progress indication
  m_currentSamples += read_samples;
  percent = static_cast<int>(m_currentSamples * 100.0f / m_totalSamples);

  return -(1-encres);
}

CEncoder* CTranscodeJob::SetupEncoder(std::string &outFile, CMusicInfoTag *tag, unsigned int samplerate, unsigned int channels, unsigned int bps)
{
  CEncoder* encoder = NULL;
  std::string extension;
  if (CSettings::Get().GetString("audiocds.encoder") == "audioencoder.xbmc.builtin.aac")
  {
    boost::shared_ptr<IEncoder> enc(new CEncoderFFmpeg());
    encoder = new CEncoder(enc);
    extension = ".aac";
  }
  else if (CSettings::Get().GetString("audiocds.encoder") == "audioencoder.xbmc.builtin.wma")
  {
    boost::shared_ptr<IEncoder> enc(new CEncoderFFmpeg());
    encoder = new CEncoder(enc);
    extension = ".wma";
  }
  else
  {
    AddonPtr addon;
    CAddonMgr::Get().GetAddon(CSettings::Get().GetString("audiocds.encoder"), addon);
    if (addon)
    {
      boost::shared_ptr<CAudioEncoder> aud =  boost::static_pointer_cast<CAudioEncoder>(addon);
      aud->Create();
      boost::shared_ptr<IEncoder> enc =  boost::static_pointer_cast<IEncoder>(aud);
      encoder = new CEncoder(enc);
      extension = aud->extension;
    }
  }
  if (!encoder)
    return NULL;

  outFile = URIUtils::ReplaceExtension(outFile, extension);

  // we have to set the tags before we init the Encoder
  encoder->SetComment("Transcoded with XBMC");
  if (tag)
  {
    encoder->SetArtist(StringUtils::Join(tag->GetArtist(),
                                        g_advancedSettings.m_musicItemSeparator));
    encoder->SetTitle(tag->GetTitle());
    encoder->SetAlbum(tag->GetAlbum());
    encoder->SetAlbumArtist(StringUtils::Join(tag->GetAlbumArtist(),
                                        g_advancedSettings.m_musicItemSeparator));
    encoder->SetGenre(StringUtils::Join(tag->GetGenre(),
                                        g_advancedSettings.m_musicItemSeparator));
    encoder->SetTrack(StringUtils::Format("%i", tag->GetTrackNumber()));
    encoder->SetTrackLength(tag->GetDuration());
    encoder->SetYear(tag->GetYearString());
  }
  // init encoder
  if (!encoder->Init(outFile.c_str(), channels, samplerate, bps))
    delete encoder, encoder = NULL;

  return encoder;
}

std::string CTranscodeJob::SetupTempFile()
{
  char tmp[MAX_PATH];
#ifndef TARGET_POSIX
  GetTempFileName(CSpecialProtocol::TranslatePath("special://temp/"), "riptrack", 0, tmp);
#else
  int fd;
  strncpy(tmp, CSpecialProtocol::TranslatePath("special://temp/riptrackXXXXXX").c_str(), MAX_PATH);
  if ((fd = mkstemp(tmp)) == -1)
   tmp[0] = '\0'; 
  if (fd != -1)
    close(fd);
#endif
  return tmp;
}

bool CTranscodeJob::operator==(const CJob* job) const
{
  if (strcmp(job->GetType(),GetType()) == 0)
  {
    const CTranscodeJob* rjob = dynamic_cast<const CTranscodeJob*>(job);
    if (rjob)
    {
      return m_input.GetPath() == rjob->m_input.GetPath() &&
             m_output == rjob->m_output;
    }
  }
  return false;
}
