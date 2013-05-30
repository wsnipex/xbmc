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

#include "system.h"
#ifdef HAVE_LIBVA

#include "VAAPI_VPP.h"
#if VA_CHECK_VERSION(0,34,0)
# define VPP_AVAIL 1
#endif

#ifdef VPP_AVAIL
#include <va/va_vpp.h>
#endif

#include "utils/log.h"
#include "threads/SingleLock.h"

#include <algorithm>

#ifndef VA_SURFACE_ATTRIB_SETTABLE
#define vaCreateSurfaces(d, f, w, h, s, ns, a, na) \
    vaCreateSurfaces(d, w, h, f, ns, s)
#endif

using namespace VAAPI;

bool CVPP::isSupported = true;

CVPP::CVPP()
    :m_width(0)
    ,m_height(0)
    ,m_configId(VA_INVALID_ID)
    ,m_vppReady(-1)
    ,m_deintBobReady(-1)
    ,m_bobContext(VA_INVALID_ID)
    ,m_bobTargetCount(0)
    ,m_bobFilter(VA_INVALID_ID)
{}

CVPP::CVPP(CDisplayPtr& display, int width, int height)
    :m_display(display)
    ,m_width(width)
    ,m_height(height)
    ,m_configId(VA_INVALID_ID)
    ,m_vppReady(0)
    ,m_deintBobReady(0)
    ,m_bobContext(VA_INVALID_ID)
    ,m_bobTargetCount(0)
    ,m_bobFilter(VA_INVALID_ID)
{
    assert(display.get() && display->get());
    assert(width > 0 && height > 0);
}

CVPP::~CVPP()
{
    deinit();
}

bool CVPP::Supported()
{
#ifdef VPP_AVAIL
    return isSupported;
#else
    return false;
#endif
}

void CVPP::deinit()
{
#ifdef VPP_AVAIL
    CSingleLock lock(m_bob_lock);

    CLog::Log(LOGDEBUG, "VAAPI_VPP - Deinitializing");

    if(DeintBobReady() || m_deintBobReady == -100)
    {
        vaDestroyBuffer(m_display->get(), m_bobFilter);
        vaDestroyContext(m_display->get(), m_bobContext);
        delete[] m_bobTargets;
        m_bobTargets = 0;
        m_deintBobReady = -1;

        CLog::Log(LOGDEBUG, "VAAPI_VPP - Deinitialized bob");
    }

    if(VppReady())
    {
        vaDestroyConfig(m_display->get(), m_configId);
        m_vppReady = -1;

        CLog::Log(LOGDEBUG, "VAAPI_VPP - Deinitialized vpp");
    }
#endif
}

bool CVPP::InitVpp()
{
#ifdef VPP_AVAIL
    CSingleLock lock(m_bob_lock);

    if(VppFailed())
        return false;

    int numEntrypoints = vaMaxNumEntrypoints(m_display->get());
    VAEntrypoint *entrypoints = new VAEntrypoint[numEntrypoints];

    if(vaQueryConfigEntrypoints(m_display->get(), VAProfileNone, entrypoints, &numEntrypoints) != VA_STATUS_SUCCESS)
    {
        delete[] entrypoints;

        CLog::Log(LOGERROR, "VAAPI_VPP - failed querying entrypoints");

        m_vppReady = -1;
        return false;
    }

    int i;
    for(i = 0; i < numEntrypoints; ++i)
        if(entrypoints[i] == VAEntrypointVideoProc)
            break;
    delete[] entrypoints;

    if(i >= numEntrypoints)
    {
        CLog::Log(LOGDEBUG, "VAAPI_VPP - Entrypoint VideoProc not supported");
        isSupported = false;
        m_vppReady = -1;
        return false;
    }

    if(vaCreateConfig(m_display->get(), VAProfileNone, VAEntrypointVideoProc, NULL, 0, &m_configId) != VA_STATUS_SUCCESS)
    {
        CLog::Log(LOGERROR, "VAAPI_VPP - failed creating va config");
        m_vppReady = -1;
        return false;
    }

    CLog::Log(LOGDEBUG, "VAAPI_VPP - Successfully initialized VPP");

    m_vppReady = 1;
    return true;
#else
    m_vppReady = -1;
    return false;
#endif
}

bool CVPP::InitDeintBob(int num_surfaces)
{
#ifdef VPP_AVAIL
    CSingleLock lock(m_bob_lock);

    assert(VppReady());

    if(DeintBobFailed())
        return false;

    VASurfaceID *bobTargetsInternal = new VASurfaceID[num_surfaces];
    m_bobTargetCount = num_surfaces;
    if(vaCreateSurfaces(m_display->get(), VA_RT_FORMAT_YUV420, m_width, m_height, bobTargetsInternal, m_bobTargetCount, NULL, 0) != VA_STATUS_SUCCESS)
    {
        CLog::Log(LOGERROR, "VAAPI_VPP - failed creating bob target surface");
        m_deintBobReady = -1;
        return false;
    }

    if(vaCreateContext(m_display->get(), m_configId, m_width, m_height, VA_PROGRESSIVE, bobTargetsInternal, m_bobTargetCount, &m_bobContext) != VA_STATUS_SUCCESS)
    {
        CLog::Log(LOGERROR, "VAAPI_VPP - failed creating bob deint context");
        vaDestroySurfaces(m_display->get(), bobTargetsInternal, m_bobTargetCount);
        m_deintBobReady = -1;
        return false;
    }

    VAProcFilterType filters[VAProcFilterCount];
    unsigned int numFilters = VAProcFilterCount;
    VAProcFilterCapDeinterlacing deinterlacingCaps[VAProcDeinterlacingCount];
    unsigned int numDeinterlacingCaps = VAProcDeinterlacingCount;

    if(
        vaQueryVideoProcFilters(m_display->get(), m_bobContext, filters, &numFilters) != VA_STATUS_SUCCESS
     || vaQueryVideoProcFilterCaps(m_display->get(), m_bobContext, VAProcFilterDeinterlacing, deinterlacingCaps, &numDeinterlacingCaps) != VA_STATUS_SUCCESS)
    {
        vaDestroyContext(m_display->get(), m_bobContext);
        vaDestroySurfaces(m_display->get(), bobTargetsInternal, m_bobTargetCount);
        CLog::Log(LOGERROR, "VAAPI_VPP - failed querying filter caps");
        m_deintBobReady = -1;
        return false;
    }

    bool bobSupported = false;
    for(unsigned int fi = 0; fi < numFilters; ++fi)
        if(filters[fi] == VAProcFilterDeinterlacing)
            for(unsigned int ci = 0; ci < numDeinterlacingCaps; ++ci)
            {
                if(deinterlacingCaps[ci].type != VAProcDeinterlacingBob)
                    continue;
                bobSupported = true;
                break;
            }

    if(!bobSupported)
    {
        vaDestroyContext(m_display->get(), m_bobContext);
        vaDestroySurfaces(m_display->get(), bobTargetsInternal, m_bobTargetCount);
        CLog::Log(LOGDEBUG, "VAAPI_VPP - bob deinterlacing filter is not supported");
        isSupported = false;
        m_deintBobReady = -1;
        return false;
    }

    VAProcFilterParameterBufferDeinterlacing deint;
    deint.type = VAProcFilterDeinterlacing;
    deint.algorithm = VAProcDeinterlacingBob;
    deint.flags = 0;

    if(vaCreateBuffer(m_display->get(), m_bobContext, VAProcFilterParameterBufferType, sizeof(deint), 1, &deint, &m_bobFilter) != VA_STATUS_SUCCESS)
    {
        vaDestroyContext(m_display->get(), m_bobContext);
        vaDestroySurfaces(m_display->get(), bobTargetsInternal, m_bobTargetCount);
        CLog::Log(LOGERROR, "VAAPI_VPP - creating ProcFilterParameterBuffer failed");
        m_deintBobReady = -1;
        return false;
    }

    CLog::Log(LOGDEBUG, "VAAPI_VPP - Successfully initialized bob");

    m_bobTargets = new CSurfacePtr[m_bobTargetCount];
    for(int i = 0; i < m_bobTargetCount; ++i)
        m_bobTargets[i] = CSurfacePtr(new CSurface(bobTargetsInternal[i], m_display));

    m_deintBobReady = 1;
    return true;
#else
    m_deintBobReady = -1;
    return false;
#endif
}

#define CHECK_VA(s, b) \
{\
    VAStatus res = s;\
    if(res != VA_STATUS_SUCCESS)\
    {\
        CLog::Log(LOGERROR, "VAAPI_VPP - failed executing "#s" at line %d with error %x:%s", __LINE__, res, vaErrorStr(res));\
        VABufferID buf = b;\
        if(buf != 0)\
            vaDestroyBuffer(m_display->get(), buf);\
        m_deintBobReady = -100;\
        return CSurfacePtr();\
    }\
}

CSurfacePtr CVPP::DeintBob(const CSurfacePtr& input, bool topField)
{
#ifdef VPP_AVAIL
    CSingleLock lock(m_bob_lock);

    if(!DeintBobReady())
        return CSurfacePtr();

    CSurfacePtr bobTarget = getFreeSurface();
    if(!bobTarget.get())
        return CSurfacePtr();

    VABufferID pipelineBuf;
    VAProcPipelineParameterBuffer *pipelineParam;
    VARectangle inputRegion;
    VARectangle outputRegion;

    CHECK_VA(vaBeginPicture(m_display->get(), m_bobContext, bobTarget->m_id), 0);

    CHECK_VA(vaCreateBuffer(m_display->get(), m_bobContext, VAProcPipelineParameterBufferType, sizeof(VAProcPipelineParameterBuffer), 1, NULL, &pipelineBuf), 0);
    CHECK_VA(vaMapBuffer(m_display->get(), pipelineBuf, (void**)&pipelineParam), pipelineBuf);

    // This input/output regions crop two the top and bottom pixel rows
    // It removes the flickering pixels caused by bob
    const int vcrop = 1;
    const int hcrop = 0;
    inputRegion.x = outputRegion.x = hcrop;
    inputRegion.y = outputRegion.y = vcrop;
    inputRegion.width = outputRegion.width = m_width - (2 * hcrop);
    inputRegion.height = outputRegion.height = m_height - (2 * vcrop);

    pipelineParam->surface = input->m_id;
    pipelineParam->output_region = &outputRegion;
    pipelineParam->surface_region = &inputRegion;
    pipelineParam->output_background_color = 0xff000000;

    pipelineParam->filter_flags = topField ? VA_TOP_FIELD : VA_BOTTOM_FIELD;
    pipelineParam->filters = &m_bobFilter;
    pipelineParam->num_filters = 1;

    pipelineParam->num_forward_references = 0;
    pipelineParam->forward_references = NULL;
    pipelineParam->num_backward_references = 0;
    pipelineParam->backward_references = NULL;

    CHECK_VA(vaUnmapBuffer(m_display->get(), pipelineBuf), pipelineBuf);

    CHECK_VA(vaRenderPicture(m_display->get(), m_bobContext, &pipelineBuf, 1), pipelineBuf);
    CHECK_VA(vaEndPicture(m_display->get(), m_bobContext), pipelineBuf);

    CHECK_VA(vaDestroyBuffer(m_display->get(), pipelineBuf), 0);

    CHECK_VA(vaSyncSurface(m_display->get(), bobTarget->m_id), 0);

    return bobTarget;
#else
    return CSurfacePtr();
#endif
}

CSurfacePtr CVPP::getFreeSurface()
{
#ifdef VPP_AVAIL
    for(int i = 0; i < m_bobTargetCount; ++i)
    {
        if(m_bobTargets[i].unique())
        {
            return m_bobTargets[i];
        }
    }

    CLog::Log(LOGWARNING, "VAAPI_VPP - Running out of surfaces");
#endif

    return CSurfacePtr();
}

#endif
