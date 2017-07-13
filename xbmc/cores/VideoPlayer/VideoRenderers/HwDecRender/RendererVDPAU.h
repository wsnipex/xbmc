/*
 *      Copyright (C) 2007-2015 Team XBMC
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

#pragma once

#include "system.h"

#include "cores/VideoPlayer/VideoRenderers/LinuxRendererGL.h"
#include "VdpauGL.h"

class CRendererVDPAU : public CLinuxRendererGL
{
public:
  CRendererVDPAU();
  virtual ~CRendererVDPAU();

  static CBaseRenderer* Create(CVideoBuffer *buffer);
  static bool Register();

  virtual bool Configure(const VideoPicture &picture, float fps, unsigned flags, unsigned int orientation) override;

  // Player functions
  virtual void ReleaseBuffer(int idx) override;
  virtual bool ConfigChanged(const VideoPicture &picture) override;
  bool NeedBuffer(int idx) override;

  // Feature support
  virtual bool Supports(ERENDERFEATURE feature);
  virtual bool Supports(ESCALINGMETHOD method);

protected:
  virtual bool LoadShadersHook();
  virtual bool RenderHook(int idx);
  virtual void AfterRenderHook(int idx) override;

  // textures
  virtual bool UploadTexture(int index) override;
  virtual void DeleteTexture(int index);
  virtual bool CreateTexture(int index);

  bool CreateVDPAUTexture(int index);
  void DeleteVDPAUTexture(int index);
  bool UploadVDPAUTexture(int index);

  bool CreateVDPAUTexture420(int index);
  void DeleteVDPAUTexture420(int index);
  bool UploadVDPAUTexture420(int index);

  virtual EShaderFormat GetShaderFormat() override;

  bool m_isYuv = false;

  VDPAU::CInteropState m_interopState;
  VDPAU::CVdpauTexture m_vdpauTextures[NUM_BUFFERS];
  GLsync m_fences[NUM_BUFFERS];
};


