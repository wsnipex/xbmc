/*
 *      Copyright (C) 2016 Team Kodi
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <memory>
#include <queue>
#include "DVDCodecs/Video/DVDVideoCodecFFmpeg.h"
#include "libavcodec/avcodec.h"
#include "MMALCodec.h"

class CMMALRenderer;
class CMMALPool;
struct MMAL_BUFFER_HEADER_T;
class CGPUMEM;

namespace MMAL {

class CDecoder;
class CGPUPool;

// a mmal video frame
class CMMALYUVBuffer : public CMMALBuffer
{
public:
  CMMALYUVBuffer(CDecoder *dec, std::shared_ptr<CMMALPool> pool, uint32_t mmal_encoding, uint32_t width, uint32_t height, uint32_t aligned_width, uint32_t aligned_height, uint32_t size);
  virtual ~CMMALYUVBuffer();

  CGPUMEM *gmem;
  CDecoder *m_omv;
private:
};

class CDecoder
  : public IHardwareDecoder
{
public:
  CDecoder(CProcessInfo& processInfo, CDVDStreamInfo &hints);
  virtual ~CDecoder();
  virtual bool Open(AVCodecContext* avctx, AVCodecContext* mainctx, const enum AVPixelFormat) override;
  virtual CDVDVideoCodec::VCReturn Decode(AVCodecContext* avctx, AVFrame* frame) override;
  virtual bool GetPicture(AVCodecContext* avctx, VideoPicture* picture) override;
  virtual CDVDVideoCodec::VCReturn Check(AVCodecContext* avctx) override;
  virtual const std::string Name() override { return "mmal"; }
  virtual unsigned GetAllowedReferences() override;
  virtual long Release() override;

  static void FFReleaseBuffer(void *opaque, uint8_t *data);
  static int FFGetBuffer(AVCodecContext *avctx, AVFrame *pic, int flags);

protected:
  AVCodecContext *m_avctx;
  CProcessInfo &m_processInfo;
  CCriticalSection m_section;
  std::shared_ptr<CMMALPool> m_pool;
  enum AVPixelFormat m_fmt;
  CDVDStreamInfo m_hints;
  CGPUMEM *m_gmem;
};

};
