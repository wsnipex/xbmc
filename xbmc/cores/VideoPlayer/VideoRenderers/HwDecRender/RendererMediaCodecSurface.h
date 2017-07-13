/*
 *      Copyright (C) 2005-2017 Team Kodi
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
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#if defined(TARGET_ANDROID)

#include "system.h"
#include "cores/VideoPlayer/VideoRenderers/BaseRenderer.h"
#include <chrono>

class CRendererMediaCodecSurface : public CBaseRenderer
{
public:
  CRendererMediaCodecSurface();
  virtual ~CRendererMediaCodecSurface();

  virtual bool RenderCapture(CRenderCapture* capture);
  virtual void AddVideoPictureHW(VideoPicture &picture, int index);
  virtual void ReleaseBuffer(int idx);
  virtual bool Configure(unsigned int width, unsigned int height, unsigned int d_width, unsigned int d_height, float fps, unsigned flags, ERenderFormat format, void *hwPic, unsigned int orientation);
  virtual bool IsConfigured() { return m_bConfigured; };
  virtual CRenderInfo GetRenderInfo();
  virtual int GetImage(YV12Image *image, int source = -1, bool readonly = false);
  virtual void ReleaseImage(int source, bool preserve = false) {};
  virtual void FlipPage(int source);
  virtual void PreInit() {};
  virtual void UnInit() {};
  virtual void Reset();
  virtual void Update() {};
  virtual void RenderUpdate(bool clear, unsigned int flags = 0, unsigned int alpha = 255);
  virtual bool SupportsMultiPassRendering() { return false; };

  // Player functions
  virtual bool IsGuiLayer() { return false; };

  // Feature support
  virtual bool Supports(EINTERLACEMETHOD method) { return false; };
  virtual bool Supports(ESCALINGMETHOD method) { return false; };

  virtual bool Supports(ERENDERFEATURE feature);

  virtual EINTERLACEMETHOD AutoInterlaceMethod() { return VS_INTERLACEMETHOD_NONE; };
protected:
  virtual void ReorderDrawPoints() override;

private:

  int m_iRenderBuffer;
  static const int m_numRenderBuffers = 4;

  struct BUFFER
  {
    void *hwPic;
    int duration;
  } m_buffers[m_numRenderBuffers];

<<<<<<< HEAD
  std::chrono::time_point<std::chrono::system_clock> m_prevTime;
  bool m_bConfigured;
  unsigned int m_updateCount;
<<<<<<< HEAD
  CRect m_surfDestRect;
=======
=======
  // textures
  virtual bool UploadTexture(int index);
  virtual void DeleteTexture(int index);
  virtual bool CreateTexture(int index);
  
  // hooks for hw dec renderer
  virtual bool LoadShadersHook();
  virtual bool RenderHook(int index);  
  virtual int  GetImageHook(YuvImage *image, int source = AUTOSOURCE, bool readonly = false);
>>>>>>> 3548552... VideoPlayer: rename and move YuvImage
>>>>>>> VideoPlayer: rename and move YuvImage
};

#endif
