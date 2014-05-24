#pragma once
/*
*      Copyright (C) 2014 Team XBMC
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

#include <string>
#include "utils/Job.h"
#include "FileItem.h"

class CAudioDecoder;
namespace MUSIC_INFO {
  class CMusicInfoTag;
};
class CEncoder;

class CTranscodeJob : public CJob
{
public:
  //! \brief Construct a transcode job
  //! \param input The input file url
  //! \param output The output file url
  //! \param encoder The encoder to use. See Encoder.h
  CTranscodeJob(const CFileItem& input, const std::string& output, int encoder);

  virtual ~CTranscodeJob();

  virtual const char* GetType() const { return "audiotranscoder"; };
  virtual bool operator==(const CJob *job) const;
  virtual bool DoWork();
protected:
  //! \brief Setup the audio encoder
  CEncoder* SetupEncoder(std::string &outFile, MUSIC_INFO::CMusicInfoTag *tag, unsigned int samplerate, unsigned int channels, unsigned int bps);

  //! \brief Helper used if output is a remote url
  std::string SetupTempFile();

  //! \brief Transcode a chunk of audio
  //! \param decoder The input decoder
  //! \param encoder The audio encoder
  //! \param percent The percentage completed on return
  //! \return 0 if everything went okay, or
  //!         a positive error code from the decoder, or
  //!         -1 if the encoder failed
  //! \sa CEncoder::Encode
  int TranscodeChunk(CAudioDecoder& decoder, CEncoder* encoder, int& percent);

private:
  CFileItem m_input;       ///< The input fileitem
  std::string m_output;    ///< The output url
  int m_encoder;           ///< The audio encoder
  size_t m_totalSamples;   ///< Total samples (for progress)
  size_t m_currentSamples; ///< Current samples (for progress)
};

