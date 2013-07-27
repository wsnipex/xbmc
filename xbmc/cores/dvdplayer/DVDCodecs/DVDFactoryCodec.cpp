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
#include "utils/log.h"

#include "DVDFactoryCodec.h"
#include "Video/DVDVideoCodec.h"
#include "Audio/DVDAudioCodec.h"
#include "Overlay/DVDOverlayCodec.h"

#if defined(TARGET_DARWIN_OSX)
#include "Video/DVDVideoCodecVDA.h"
#endif
#if defined(HAVE_VIDEOTOOLBOXDECODER)
#include "Video/DVDVideoCodecVideoToolBox.h"
#endif
#include "Video/DVDVideoCodecFFmpeg.h"
#include "Video/DVDVideoCodecOpenMax.h"
#include "Video/DVDVideoCodecLibMpeg2.h"
#if defined(HAVE_LIBCRYSTALHD)
#include "Video/DVDVideoCodecCrystalHD.h"
#endif
#if defined(HAS_LIBAMCODEC)
#include "Video/DVDVideoCodecAmlogic.h"
#endif
#include "Audio/DVDAudioCodecFFmpeg.h"
#include "Audio/DVDAudioCodecLibMad.h"
#include "Audio/DVDAudioCodecPcm.h"
#include "Audio/DVDAudioCodecLPcm.h"
#if defined(TARGET_DARWIN_OSX) || defined(TARGET_DARWIN_IOS)
#include "Audio/DVDAudioCodecPassthroughFFmpeg.h"
#endif
#include "Audio/DVDAudioCodecPassthrough.h"
#include "Overlay/DVDOverlayCodecSSA.h"
#include "Overlay/DVDOverlayCodecText.h"
#include "Overlay/DVDOverlayCodecTX3G.h"
#include "Overlay/DVDOverlayCodecFFmpeg.h"


#include "DVDStreamInfo.h"
#include "settings/Settings.h"
#include "utils/SystemInfo.h"

CDVDVideoCodec* CDVDFactoryCodec::OpenCodec(CDVDVideoCodec* pCodec, CDVDStreamInfo &hints, CDVDCodecOptions &options )
{
  try
  {
    CLog::Log(LOGDEBUG, "FactoryCodec - Video: %s - Opening", pCodec->GetName());
    if( pCodec->Open( hints, options ) )
    {
      CLog::Log(LOGDEBUG, "FactoryCodec - Video: %s - Opened", pCodec->GetName());
      return pCodec;
    }

    CLog::Log(LOGDEBUG, "FactoryCodec - Video: %s - Failed", pCodec->GetName());
    pCodec->Dispose();
    delete pCodec;
  }
  catch(...)
  {
    CLog::Log(LOGERROR, "FactoryCodec - Video: Failed with exception");
  }
  return NULL;
}

CDVDAudioCodec* CDVDFactoryCodec::OpenCodec(CDVDAudioCodec* pCodec, CDVDStreamInfo &hints, CDVDCodecOptions &options )
{
  try
  {
    CLog::Log(LOGDEBUG, "FactoryCodec - Audio: %s - Opening", pCodec->GetName());
    if( pCodec->Open( hints, options ) )
    {
      CLog::Log(LOGDEBUG, "FactoryCodec - Audio: %s - Opened", pCodec->GetName());
      return pCodec;
    }

    CLog::Log(LOGDEBUG, "FactoryCodec - Audio: %s - Failed", pCodec->GetName());
    pCodec->Dispose();
    delete pCodec;
  }
  catch(...)
  {
    CLog::Log(LOGERROR, "FactoryCodec - Audio: Failed with exception");
  }
  return NULL;
}

CDVDOverlayCodec* CDVDFactoryCodec::OpenCodec(CDVDOverlayCodec* pCodec, CDVDStreamInfo &hints, CDVDCodecOptions &options )
{
  try
  {
    CLog::Log(LOGDEBUG, "FactoryCodec - Overlay: %s - Opening", pCodec->GetName());
    if( pCodec->Open( hints, options ) )
    {
      CLog::Log(LOGDEBUG, "FactoryCodec - Overlay: %s - Opened", pCodec->GetName());
      return pCodec;
    }

    CLog::Log(LOGDEBUG, "FactoryCodec - Overlay: %s - Failed", pCodec->GetName());
    pCodec->Dispose();
    delete pCodec;
  }
  catch(...)
  {
    CLog::Log(LOGERROR, "FactoryCodec - Audio: Failed with exception");
  }
  return NULL;
}


CDVDVideoCodec* CDVDFactoryCodec::CreateVideoCodec(CDVDStreamInfo &hint, unsigned int surfaces, const std::vector<ERenderFormat>& formats)
{
  CDVDVideoCodec* pCodec = NULL;
  CDVDCodecOptions options;

  if(formats.empty())
    options.m_formats.push_back(RENDER_FMT_YUV420P);
  else
    options.m_formats = formats;

  //when support for a hardware decoder is not compiled in
  //only print it if it's actually available on the platform
  CStdString hwSupport;
#if defined(TARGET_DARWIN_OSX)
  hwSupport += "VDADecoder:yes ";
#endif
#if defined(HAVE_VIDEOTOOLBOXDECODER) && defined(TARGET_DARWIN)
  hwSupport += "VideoToolBoxDecoder:yes ";
#elif defined(TARGET_DARWIN)
  hwSupport += "VideoToolBoxDecoder:no ";
#endif
#ifdef HAVE_LIBCRYSTALHD
  hwSupport += "CrystalHD:yes ";
#else
  hwSupport += "CrystalHD:no ";
#endif
#if defined(HAS_LIBAMCODEC)
  hwSupport += "AMCodec:yes ";
#else
  hwSupport += "AMCodec:no ";
#endif
#if defined(HAVE_LIBOPENMAX) && defined(TARGET_POSIX)
  hwSupport += "OpenMax:yes ";
#elif defined(TARGET_POSIX)
  hwSupport += "OpenMax:no ";
#endif
#if defined(HAVE_LIBVDPAU) && defined(TARGET_POSIX)
  hwSupport += "VDPAU:yes ";
#elif defined(TARGET_POSIX) && !defined(TARGET_DARWIN)
  hwSupport += "VDPAU:no ";
#endif
#if defined(TARGET_WINDOWS) && defined(HAS_DX)
  hwSupport += "DXVA:yes ";
#elif defined(TARGET_WINDOWS)
  hwSupport += "DXVA:no ";
#endif
#if defined(HAVE_LIBVA) && defined(TARGET_POSIX)
  hwSupport += "VAAPI:yes ";
#elif defined(TARGET_POSIX) && !defined(TARGET_DARWIN)
  hwSupport += "VAAPI:no ";
#endif

  CLog::Log(LOGDEBUG, "CDVDFactoryCodec: compiled in hardware support: %s", hwSupport.c_str());
#if !defined(HAS_LIBAMCODEC)
  // dvd's have weird still-frames in it, which is not fully supported in ffmpeg
  if(hint.stills && (hint.codec == AV_CODEC_ID_MPEG2VIDEO || hint.codec == AV_CODEC_ID_MPEG1VIDEO))
  {
    if( (pCodec = OpenCodec(new CDVDVideoCodecLibMpeg2(), hint, options)) ) return pCodec;
  }
#endif

#if defined(TARGET_DARWIN_OSX)
  if (!hint.software && CSettings::Get().GetBool("videoplayer.usevda"))
  {
    if (hint.codec == AV_CODEC_ID_H264 && !hint.ptsinvalid)
    {
      if ( (pCodec = OpenCodec(new CDVDVideoCodecVDA(), hint, options)) ) return pCodec;
    }
  }
#endif

#if defined(HAVE_VIDEOTOOLBOXDECODER)
  if (!hint.software && CSettings::Get().GetBool("videoplayer.usevideotoolbox"))
  {
    if (g_sysinfo.HasVideoToolBoxDecoder())
    {
      switch(hint.codec)
      {
        case AV_CODEC_ID_H264:
          if (hint.codec == AV_CODEC_ID_H264 && hint.ptsinvalid)
            break;
          if ( (pCodec = OpenCodec(new CDVDVideoCodecVideoToolBox(), hint, options)) ) return pCodec;
        break;
        default:
        break;
      }
    }
  }
#endif

#if defined(HAVE_LIBCRYSTALHD)
  if (!hint.software && CSettings::Get().GetBool("videoplayer.usechd"))
  {
    if (CCrystalHD::GetInstance()->DevicePresent())
    {
      switch(hint.codec)
      {
        case AV_CODEC_ID_VC1:
        case AV_CODEC_ID_WMV3:
        case AV_CODEC_ID_H264:
        case AV_CODEC_ID_MPEG2VIDEO:
          if (hint.codec == AV_CODEC_ID_H264 && hint.ptsinvalid)
            break;
          if (hint.codec == AV_CODEC_ID_MPEG2VIDEO && hint.width <= 720)
            break;
          if ( (pCodec = OpenCodec(new CDVDVideoCodecCrystalHD(), hint, options)) ) return pCodec;
        break;
        default:
        break;
      }
    }
  }
#endif

#if defined(HAS_LIBAMCODEC)
  if (!hint.software)
  {
    CLog::Log(LOGINFO, "Amlogic Video Decoder...");
    if ( (pCodec = OpenCodec(new CDVDVideoCodecAmlogic(), hint, options)) ) return pCodec;
  }
#endif

#if defined(HAVE_LIBOPENMAX)
  if (CSettings::Get().GetBool("videoplayer.useomx") && !hint.software )
  {
      if (hint.codec == AV_CODEC_ID_H264 || hint.codec == AV_CODEC_ID_MPEG2VIDEO || hint.codec == AV_CODEC_ID_VC1)
    {
      if ( (pCodec = OpenCodec(new CDVDVideoCodecOpenMax(), hint, options)) ) return pCodec;
    }
  }
#endif

  // try to decide if we want to try halfres decoding
#if !defined(TARGET_POSIX) && !defined(TARGET_WINDOWS)
  float pixelrate = (float)hint.width*hint.height*hint.fpsrate/hint.fpsscale;
  if( pixelrate > 1400.0f*720.0f*30.0f )
  {
    CLog::Log(LOGINFO, "CDVDFactoryCodec - High video resolution detected %dx%d, trying half resolution decoding ", hint.width, hint.height);
    options.m_keys.push_back(CDVDCodecOption("lowres","1"));
  }
#endif

  CStdString value;
  value.Format("%d", surfaces);
  options.m_keys.push_back(CDVDCodecOption("surfaces", value));
  if( (pCodec = OpenCodec(new CDVDVideoCodecFFmpeg(), hint, options)) ) return pCodec;

  return NULL;
}

CDVDAudioCodec* CDVDFactoryCodec::CreateAudioCodec( CDVDStreamInfo &hint, bool passthrough /* = true */)
{
  CDVDAudioCodec* pCodec = NULL;
  CDVDCodecOptions options;

  if (passthrough)
  {
#if defined(TARGET_DARWIN_OSX) || defined(TARGET_DARWIN_IOS)
    switch(hint.codec)
    {
      case AV_CODEC_ID_AC3:
      case AV_CODEC_ID_DTS:
        pCodec = OpenCodec( new CDVDAudioCodecPassthroughFFmpeg(), hint, options );
        if( pCodec ) return pCodec;
        break;
      default:
        break;      
    }
#endif
    pCodec = OpenCodec( new CDVDAudioCodecPassthrough(), hint, options );
    if( pCodec ) return pCodec;
  }

  switch (hint.codec)
  {
  case AV_CODEC_ID_MP2:
  case AV_CODEC_ID_MP3:
    {
      pCodec = OpenCodec( new CDVDAudioCodecLibMad(), hint, options );
      if( pCodec ) return pCodec;
      break;
    }
  case AV_CODEC_ID_PCM_S32LE:
  case AV_CODEC_ID_PCM_S32BE:
  case AV_CODEC_ID_PCM_U32LE:
  case AV_CODEC_ID_PCM_U32BE:
  case AV_CODEC_ID_PCM_S24LE:
  case AV_CODEC_ID_PCM_S24BE:
  case AV_CODEC_ID_PCM_U24LE:
  case AV_CODEC_ID_PCM_U24BE:
  case AV_CODEC_ID_PCM_S24DAUD:
  case AV_CODEC_ID_PCM_S16LE:
  case AV_CODEC_ID_PCM_S16BE:
  case AV_CODEC_ID_PCM_U16LE:
  case AV_CODEC_ID_PCM_U16BE:
  case AV_CODEC_ID_PCM_S8:
  case AV_CODEC_ID_PCM_U8:
  case AV_CODEC_ID_PCM_ALAW:
  case AV_CODEC_ID_PCM_MULAW:
    {
      pCodec = OpenCodec( new CDVDAudioCodecPcm(), hint, options );
      if( pCodec ) return pCodec;
      break;
    }
#if 0
  //case AV_CODEC_ID_LPCM_S16BE:
  //case AV_CODEC_ID_LPCM_S20BE:
  case AV_CODEC_ID_LPCM_S24BE:
    {
      pCodec = OpenCodec( new CDVDAudioCodecLPcm(), hint, options );
      if( pCodec ) return pCodec;
      break;
    }
#endif
  default:
    {
      pCodec = NULL;
      break;
    }
  }

  pCodec = OpenCodec( new CDVDAudioCodecFFmpeg(), hint, options );
  if( pCodec ) return pCodec;

  return NULL;
}

CDVDOverlayCodec* CDVDFactoryCodec::CreateOverlayCodec( CDVDStreamInfo &hint )
{
  CDVDOverlayCodec* pCodec = NULL;
  CDVDCodecOptions options;

  switch (hint.codec)
  {
    case AV_CODEC_ID_TEXT:
#if defined(LIBAVCODEC_FROM_FFMPEG) && LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54,53,100)
    // API changed in:
    // ffmpeg: commit 2626cc4580bfd560c6983338d77b2c11c16af94f (11 Aug 2012)
    //         release 1.0 (28 Sept 2012)
    case AV_CODEC_ID_SUBRIP:
#endif
      pCodec = OpenCodec(new CDVDOverlayCodecText(), hint, options);
      if( pCodec ) return pCodec;
      break;

    case AV_CODEC_ID_SSA:
      pCodec = OpenCodec(new CDVDOverlayCodecSSA(), hint, options);
      if( pCodec ) return pCodec;

      pCodec = OpenCodec(new CDVDOverlayCodecText(), hint, options);
      if( pCodec ) return pCodec;
      break;

    case AV_CODEC_ID_MOV_TEXT:
      pCodec = OpenCodec(new CDVDOverlayCodecTX3G(), hint, options);
      if( pCodec ) return pCodec;
      break;

    default:
      pCodec = OpenCodec(new CDVDOverlayCodecFFmpeg(), hint, options);
      if( pCodec ) return pCodec;
      break;
  }

  return NULL;
}
