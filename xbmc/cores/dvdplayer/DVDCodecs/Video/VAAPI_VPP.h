/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://www.xbmc.org
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#pragma once

#include "VAAPI.h"
#include "threads/CriticalSection.h"

#include <map>
#include <boost/shared_ptr.hpp>

namespace VAAPI
{
    class CVPP
    {
        friend class VPPSurfaceDeleter;
        CVPP();

        public:
        CVPP(CDisplayPtr& display, int width, int height);
        ~CVPP();

        static bool Supported();

        void deinit();

        bool InitVpp();
        bool InitDeintBob(int num_surfaces);

        inline bool VppReady() { return m_vppReady > 0; }
        inline bool VppFailed() { return m_vppReady < 0; }
        inline bool DeintBobReady() { return m_deintBobReady > 0; }
        inline bool DeintBobFailed() { return m_deintBobReady < 0; }

        CSurfacePtr DeintBob(const CSurfacePtr& input, bool topField);

        private:
        CSurfacePtr getFreeSurface();


        static bool isSupported;


        CDisplayPtr m_display;
        int m_width;
        int m_height;

        VAConfigID m_configId;

        int m_vppReady;
        int m_deintBobReady;

        //VPP Bob Deinterlacing
        CCriticalSection m_bob_lock;
        VAContextID m_bobContext;
        int m_bobTargetCount;
        CSurfacePtr *m_bobTargets;
        VABufferID m_bobFilter;
    };

}

