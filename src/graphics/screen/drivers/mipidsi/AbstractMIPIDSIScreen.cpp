/*
 * This file is part of MINTGGGameEngine.
 *
 * Copyright 2025-2026 David Lerch
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "AbstractMIPIDSIScreen.h"

#include <cassert>

#include "../../../../util/Log.h"
#include "../../../../util/Util.h"

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
#   include <esp_cache.h>
#   include <esp_err.h>
#   include <esp_lcd_panel_ops.h>
#endif

LOG_USE_TAG("AbstractMIPIDSIScreen")



namespace MINTGGGameEngine
{

AbstractMIPIDSIScreen::AbstractMIPIDSIScreen(const Config& cfg)
    : BufferedScreen (
          cfg.pixelFormat,
          cfg.byteOrder,
          nullptr,
          cfg.width, cfg.height,
          {}
          ),
      cfg(cfg), additionalFramebuf(nullptr)
{
    for (size_t i = 0 ; i < numFramebufs ; i++) {
        framebufs[i].buf = nullptr;
    }

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_ESP_PPA_SUPPORTED
    ppaSRMClient = nullptr;
#endif
}

AbstractMIPIDSIScreen::~AbstractMIPIDSIScreen()
{
}

bool AbstractMIPIDSIScreen::init()
{
    const char* pformatStr = PixelFormatToString(cfg.pixelFormat);
    const char* borderStr;

    switch (cfg.byteOrder) {
    case ByteOrder::LittleEndian:   borderStr = "le";   break;
    case ByteOrder::BigEndian:      borderStr = "be";      break;
    default:                        borderStr = "[invalid]";
    }

    LogInfo (
        "Initializing MIPI DSI screen (%ux%u, pFormat: %s, bOrder: %s, mirrorX: %c, mirrorY: %c, swapXY: %c, fpass: %s)",
        cfg.width, cfg.height,
        pformatStr, borderStr,
        cfg.mirrorX ? '1' : '0', cfg.mirrorY ? '1' : '0', cfg.swapXY ? '1' : '0',
#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_ESP_PPA_SUPPORTED
        useFinalTransformPass() ? "ppa" : "no"
#else
        useFinalTransformPass() ? "sw" : "no"
#endif
        );

    // IMPORTANT: There is a not-so-obvious but important detail in the call to initPanel() below. We have to make sure
    //  it executes on the same CPU core that eventually runs commit(), because we must ensure that the ISR which calls
    //  our espOnFrameBufComplete() method below always runs on the same core that the critical section in commit() runs
    //  on, otherwise these two aren't fully mutually exclusive.
    if (!initPanel(numFramebufs)) {
        shutdownPanel();
        return false;
    }

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    rendererCoreID = xTaskGetCoreID(xTaskGetCurrentTaskHandle());
    if (rendererCoreID == tskNO_AFFINITY) {
        LogWarning("Renderer task not pinned to a core. This may break AbstractMIPIDSIScreen!");
    }

    // Will get updated by the actual ISR code later
    isrCoreID = rendererCoreID;
#endif

    if (!initFinalTransformPass()) {
        return false;
    }

    if (!initTripleBuffering()) {
        return false;
    }

    if (!BufferedScreen::init()) {
        shutdownPanel();
        return false;
    }

    return true;
}

void AbstractMIPIDSIScreen::shutdown()
{
    BufferedScreen::shutdown();

    shutdownTripleBuffering();

    shutdownFinalTransformPass();

    shutdownPanel();
}

void AbstractMIPIDSIScreen::commit()
{
    BufferedScreen::commit();

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    esp_err_t err;

    if (rendererCoreID != isrCoreID) {
        LogWarning("Framebuf done ISR executed on different core than renderer. This may break AbstractMIPIDSIScreen!");
    }

    const uint8_t bytesPerPixel = CalcPixelFormatBytesPerPixel(cfg.pixelFormat);
    const size_t frameBufSize = static_cast<size_t>(cfg.width) * static_cast<size_t>(cfg.height) * bytesPerPixel;

    const bool finalTransformPass = useFinalTransformPass();

    if (finalTransformPass  &&  mightUseHWForFinalTransformPass()) {
        // If hardware does the final transform pass, we need to sync the additional framebuffer in C2M mode now, so the
        // hardware doesn't read stale data.
        err = esp_cache_msync(additionalFramebuf, frameBufSize,
            ESP_CACHE_MSYNC_FLAG_DIR_C2M);
        if (err != ESP_OK) {
            LogError("Error syncing additional framebuffer cache: %s", esp_err_to_name(err));
            return;
        }
    }

    bool finalTransformOutputTouchedByCPU = false;
    if (finalTransformPass) {
        performFinalTransformPass(framebufs[renderingFramebufIdx].buf, &finalTransformOutputTouchedByCPU);
    }

    /*if (finalTransformOutputTouchedByCPU) {
        // If the CPU wrote anything into the rendering buffer during the final transform pass, we need to sync the
        // cache in C2M mode now, and invalidate it. Otherwise the DSI peripheral might see stale data, or the CPU might
        // write back stale data to it.
        err = esp_cache_msync(additionalFramebuf, frameBufSize,
            ESP_CACHE_MSYNC_FLAG_DIR_C2M | ESP_CACHE_MSYNC_FLAG_INVALIDATE);
        if (err != ESP_OK) {
            LogError("Error syncing additional framebuffer cache: %s", esp_err_to_name(err));
            return;
        }
    }*/

    auto& plfData = getPlatformData();
    auto panel = plfData.panelHandle;

    int drawBmpXEnd = cfg.width;
    int drawBmpYEnd = cfg.height;

    const uint8_t oldRenderingFramebufIdx = renderingFramebufIdx;

    // We need to do a C2M cache sync on the rendering framebuf so the DSI peripheral doesn't read stale data, UNLESS
    // we just did a final transform pass completely in hardware. In the latter case, the CPU actually never even
    // touched the rendering framebuf in the first place, so we can skip this step.
    if (!finalTransformPass  ||  finalTransformOutputTouchedByCPU) {
        // esp_lcd_panel_draw_bitmap() below does this as well, but we'll do it here first before entering the critical
        // section below to hopefully make the critical section as quick as possible.
        err = esp_cache_msync(framebufs[oldRenderingFramebufIdx].buf, frameBufSize, ESP_CACHE_MSYNC_FLAG_DIR_C2M);
        if (err != ESP_OK) {
            LogError("Error syncing DSI framebuffer cache: %s", esp_err_to_name(err));
            return;
        }
    }

    portENTER_CRITICAL(&framebufsSpinlock);

    // Submit rendering framebuf (that we just finished rendering) to the display. It will eventually become the new
    // committed framebuf further down in this critical section.
    // Do this first so we can recover more easily in case of error.
    err = esp_lcd_panel_draw_bitmap(panel, 0, 0, drawBmpXEnd, drawBmpYEnd, framebufs[oldRenderingFramebufIdx].buf);
    if (err != ESP_OK) {
        // Welp. Just keep trying to render into the same framebuffer and hope the problem fixes itself I guess?
        portEXIT_CRITICAL(&framebufsSpinlock);
        LogError("Error updating display framebuffer: %s", esp_err_to_name(err));
        return;
    }

    // Choose the next rendering framebuf
    if (displayingFramebufIdx == committedFramebufIdx) {
        // Display only holds one framebuf -> there is a free framebuf that we can choose as next rendering framebuf

        assert(freeFramebufIdx != UINT8_MAX); // Display holds one framebuf and we hold one, sothere must be a free one
        renderingFramebufIdx = freeFramebufIdx;
        freeFramebufIdx = UINT8_MAX;
    } else {
        // Display holds two framebufs (one for displaying, one committed for displaying next) -> all buffers are full.
        // We have to take the currently committed framebuf back from the display and use it as next rendering
        // framebuf instead.

        // The display must have stayed off the buffer we just rendered into
        assert(oldRenderingFramebufIdx != displayingFramebufIdx);
        assert(oldRenderingFramebufIdx != committedFramebufIdx);

        renderingFramebufIdx = committedFramebufIdx;
    }

    // Rendering framebuf (just finished rendering) becomes committed framebuf
    committedFramebufIdx = oldRenderingFramebufIdx;

    // Postcondition: We should have exclusive access to the framebuf we just chose for rendering the next frame
    assert(renderingFramebufIdx != displayingFramebufIdx);
    assert(renderingFramebufIdx != committedFramebufIdx);

    portEXIT_CRITICAL(&framebufsSpinlock);

    // If the final transform pass is done, the engine will keep rendering into the same buffer at all times.
    if (!finalTransformPass) {
        setBuffer(framebufs[renderingFramebufIdx].buf, {});
    }
#endif
}

bool AbstractMIPIDSIScreen::initTripleBuffering()
{
    PlatformData& plfData = getPlatformData();

    displayingFramebufIdx = 0;
    committedFramebufIdx = displayingFramebufIdx;
    renderingFramebufIdx = 1;
    freeFramebufIdx = 2;

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    portMUX_INITIALIZE(&framebufsSpinlock);

    assert(numFramebufs == 3);
    esp_err_t err = esp_lcd_dpi_panel_get_frame_buffer(plfData.panelHandle, 3,
        &framebufs[0].buf, &framebufs[1].buf, &framebufs[2].buf);
    if (err != ESP_OK) {
        LogError("Error getting DSI framebuffers: %s", esp_err_to_name(err));
        return false;
    }
#else
    (void) plfData;

    // Allocate a dummy rendering framebuffer. We won't use it ourselves, but e.g. the desktop port uses it to display
    // any type of screen in its main window.
    const size_t bytesPerPixel = CalcPixelFormatBytesPerPixel(cfg.pixelFormat);
    const size_t framebufSize = static_cast<size_t>(cfg.width) * static_cast<size_t>(cfg.height) * bytesPerPixel;
    framebufs[renderingFramebufIdx].buf = malloc(framebufSize);
#endif

    if (useFinalTransformPass()) {
        // Engine renders into the additional framebuf first. The final transform pass then transforms that additional
        // framebuf into the rendering framebuf
        setBuffer(additionalFramebuf, {});
    } else {
        // Engine renders directly into the rendering framebuf
        setBuffer(framebufs[renderingFramebufIdx].buf, {});
    }

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    esp_lcd_dpi_panel_event_callbacks_t evtCallbacks = {
        .on_frame_buf_complete = &AbstractMIPIDSIScreen::espOnFrameBufComplete
    };
    err = esp_lcd_dpi_panel_register_event_callbacks(plfData.panelHandle, &evtCallbacks, this);
#endif

    return true;
}

void AbstractMIPIDSIScreen::shutdownTripleBuffering()
{
#ifndef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    // Free the dummy rendering buffer
    free(framebufs[renderingFramebufIdx].buf);
    framebufs[renderingFramebufIdx].buf = nullptr;
#endif
}

bool AbstractMIPIDSIScreen::initFinalTransformPass()
{
    if (!useFinalTransformPass()) {
        return true;
    }

    const size_t bytesPerPixel = CalcPixelFormatBytesPerPixel(cfg.pixelFormat);
    const size_t framebufSize = static_cast<size_t>(cfg.width) * static_cast<size_t>(cfg.height) * bytesPerPixel;
    constexpr size_t framebufAlignment = 128;

    additionalFramebuf = AdvancedAlignedAlloc(framebufAlignment, framebufSize, ADVANCED_ALLOC_FLAG_EXTERNAL_RAM);
    if (!additionalFramebuf) {
        additionalFramebuf = AdvancedAlignedAlloc(framebufAlignment, framebufSize, ADVANCED_ALLOC_FLAG_INTERNAL_RAM);
    }
    if (!additionalFramebuf) {
        LogError("Error allocating final transform pass framebuffer");
        return false;
    }

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_ESP_PPA_SUPPORTED
    ppa_client_config_t ppaCfg = {
        .oper_type = PPA_OPERATION_SRM,
        .max_pending_trans_num = 1,
        .data_burst_length = PPA_DATA_BURST_LENGTH_128
    };
    esp_err_t err = ppa_register_client(&ppaCfg, &ppaSRMClient);
    if (err != ESP_OK) {
        LogError("Error registering PPA SRM client: %s", esp_err_to_name(err));
    }
#endif

    return true;
}

void AbstractMIPIDSIScreen::shutdownFinalTransformPass()
{
    if (!useFinalTransformPass()) {
        return;
    }

    AdvancedFree(additionalFramebuf);
    additionalFramebuf = nullptr;
}

void AbstractMIPIDSIScreen::performFinalTransformPass(void* outBuf, bool* outputTouchedByCPU)
{
    // NOTE: All cache sync is handled by commit(), as long as we set outputTouchedByCPU correctly.

    Rotation rotation;
    FlipDir mirror;

    if (cfg.swapXY) {
        if (cfg.mirrorY) {
            rotation = Rotation::Rot270;
            mirror = cfg.mirrorX ? FlipDir::Horizontal : FlipDir::None;
        } else {
            rotation = Rotation::Rot90;
            mirror = cfg.mirrorX ? FlipDir::None : FlipDir::Horizontal;
        }
    } else {
        rotation = Rotation::Rot0;
        if (cfg.mirrorX) {
            mirror = cfg.mirrorY ? FlipDir::Both : FlipDir::Horizontal;
        } else {
            mirror = cfg.mirrorY ? FlipDir::Vertical : FlipDir::None;
        }
    }

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_ESP_PPA_SUPPORTED
    if (performFinalTransformPassESPPPA(outBuf, rotation, mirror)) {
        *outputTouchedByCPU = false;
        return;
    }
#endif

    BitmapRenderer::Transfer transfer = {
        .src = {
            .buffer = additionalFramebuf,
            .alpha = nullptr,
            .width = cfg.width,
            .height = cfg.height,
            .blockOffsetX = 0,
            .blockOffsetY = 0,
            .blockWidth = cfg.width,
            .blockHeight = cfg.height,
            .pixelFormat = getNativePixelFormat(),
            .alphaStorage = AlphaStorage::Bit1, // Ignored
            .byteOrder = getNativeByteOrder()
        },

        .dest = {
            .buffer = outBuf,
            .alpha = nullptr,
            .width = cfg.swapXY ? cfg.height : cfg.width,
            .height = cfg.swapXY ? cfg.width : cfg.height,
            .blockOffsetX = 0,
            .blockOffsetY = 0,
            .pixelFormat = getNativePixelFormat(),
            .alphaStorage = AlphaStorage::Bit1, // Ignored
            .byteOrder = getNativeByteOrder()
        },

        .blendMode = BitmapRenderer::BlendMode::Opaque, // Ignored
        .alphaMode = BitmapRenderer::AlphaMode::KeepDestAlpha // Ignored
    };

    BitmapRenderer::TransformSimple transform = {
        .scaleX = 1,
        .scaleY = 1,

        .rotation = rotation,

        .mirror = mirror
    };

    BitmapRenderer renderer;

    if (!renderer.checkValid(transfer, transform)) {
        LogError("Error in final transform pass: %s", renderer.getErrorMessage());
        return;
    }

    *outputTouchedByCPU = true;

    renderer.render(transfer, transform);
}


#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_ESP_PPA_SUPPORTED

bool AbstractMIPIDSIScreen::performFinalTransformPassESPPPA(void* outBuf, Rotation rotation, FlipDir flipDir)
{
    ppa_srm_color_mode_t colorMode;
    ppa_srm_rotation_angle_t rotAngle;

    uint8_t bytesPerPixel;

    // Exact color mode doesn't matter, since we're not doing color conversion. It's all about the bytes per pixel.
    switch (getNativePixelFormat()) {
    case PixelFormat::RGB565:
    case PixelFormat::BGR565:
        colorMode = PPA_SRM_COLOR_MODE_RGB565;
        bytesPerPixel = 2;
        break;
    case PixelFormat::RGB888:
    case PixelFormat::BGR888:
        colorMode = PPA_SRM_COLOR_MODE_RGB888;
        bytesPerPixel = 3;
        break;
    case PixelFormat::RGBX8888:
    case PixelFormat::XBGR8888:
        colorMode = PPA_SRM_COLOR_MODE_ARGB8888;
        bytesPerPixel = 4;
        break;
    default:
        assert(false);
        return false;
    }

    switch (rotation) {
    case Rotation::Rot0:    rotAngle = PPA_SRM_ROTATION_ANGLE_0;    break;
    case Rotation::Rot90:   rotAngle = PPA_SRM_ROTATION_ANGLE_90;   break;
    case Rotation::Rot180:  rotAngle = PPA_SRM_ROTATION_ANGLE_180;  break;
    case Rotation::Rot270:  rotAngle = PPA_SRM_ROTATION_ANGLE_270;  break;
    default: assert(false); return false;
    }

    const size_t frameBufSize = static_cast<size_t>(cfg.width) * static_cast<size_t>(cfg.height) * bytesPerPixel;
    const bool swapXY = (rotation == Rotation::Rot90  ||  rotation == Rotation::Rot270);

    ppa_srm_oper_config_t operCfg = {
        .in = {
            .buffer = additionalFramebuf,
            .pic_w = cfg.width,
            .pic_h = cfg.height,
            .block_w = cfg.width,
            .block_h = cfg.height,
            .block_offset_x = 0,
            .block_offset_y = 0,
            .srm_cm = colorMode
        },

        .out = {
            .buffer = outBuf,
            .buffer_size = frameBufSize,
            .pic_w = swapXY ? cfg.height : cfg.width,
            .pic_h = swapXY ? cfg.width : cfg.height,
            .block_offset_x = 0,
            .block_offset_y = 0,
            .srm_cm = colorMode
        },

        .rotation_angle = rotAngle,
        .scale_x = 1,
        .scale_y = 1,
        .mirror_x = (flipDir & FlipDir::Horizontal) != 0,
        .mirror_y = (flipDir & FlipDir::Vertical) != 0,

        .rgb_swap = false,
        .byte_swap = false,
        .alpha_update_mode = PPA_ALPHA_NO_CHANGE,

        .mode = PPA_TRANS_MODE_BLOCKING
    };

    esp_err_t err = ppa_do_scale_rotate_mirror(ppaSRMClient, &operCfg);
    if (err != ESP_OK) {
        LogError("Error in final transform pass using PPA: %s", esp_err_to_name(err));
        return false;
    }

    return true;
}

#endif


#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED

bool AbstractMIPIDSIScreen::espOnFrameBufComplete(esp_lcd_panel_handle_t panel, esp_lcd_dpi_panel_event_data_t* edata, void* userCtx)
{
    // Called by ESP-IDF's DSI driver from an ISR.
    // It is important that the actual reconfiguration of the DSI peripheral to start reading from the committed
    // framebuf happens in the very same ISR that this code runs in, because we need the entire "reconfigure peripheral
    // to start displaying from committedFramebufIdx now, then update displayingFramebufIdx and freeFramebufIdx
    // accordingly" sequence to be conceptually atomic and mutually exclusive with the critical section in commit()
    // above. This is how it currently works in the ESP-IDF driver, and I hope we can rely on that remaining the case
    // in the future.

    auto screen = static_cast<AbstractMIPIDSIScreen*>(userCtx);

    // Record core ID that's running this ISR, for checking that is executes on the same core as the renderer.
    screen->isrCoreID = xPortGetCoreID();

    portENTER_CRITICAL_ISR(&screen->framebufsSpinlock);

    if (screen->displayingFramebufIdx != screen->committedFramebufIdx) {
        // We will now move to displaying a different framebuffer, so the one we just finished displaying becomes
        // available.

        assert(screen->committedFramebufIdx != screen->renderingFramebufIdx);   // At this point we're committed to
                                                                                // the next framebuf, so the renderer
                                                                                // mustn't touch it until we mark it
                                                                                // as free again.
        assert(screen->freeFramebufIdx == UINT8_MAX);   // We're holding two, and the renderer holds one. There
                                                        // shouldn't be a free one left.

        // Framebuf that just finished displaying is now free
        screen->freeFramebufIdx = screen->displayingFramebufIdx;

        // Committed framebuf is now displaying
        screen->displayingFramebufIdx = screen->committedFramebufIdx;
    } else {
        // We don't have a new framebuffer to move on to, so we have to keep displaying the one we just finished
        // displaying. There's nothing to do here.
        assert(screen->freeFramebufIdx != UINT8_MAX);   // We're holding one framebuf, and the renderer is holding one,
                                                        // so there should always be one that's free.
    }

    portEXIT_CRITICAL_ISR(&screen->framebufsSpinlock);

    return false;
}

#endif

} // MINTGGGameEngine