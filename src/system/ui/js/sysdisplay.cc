#include <cstring>
#include <emscripten.h>
#include <memory>

#include "configparser.h"
#include "sysjs.h"

JSSystemDisplay::JSSystemDisplay(
    const DisplayCharacteristics &chr, int redraw_ms)
    : SystemDisplay(chr, redraw_ms) {
    mClientChar = chr;
    int frameBufferSize =
        mClientChar.width * mClientChar.height * mClientChar.bytesPerPixel;
    gFrameBuffer = (byte*)malloc(frameBufferSize);
    memset(gFrameBuffer, 0, frameBufferSize);

    jsFrameBufferSize = mClientChar.width * mClientChar.height * 4;
    jsFrameBuffer.reset(new byte[jsFrameBufferSize]);

    damageFrameBufferAll();
    setMouseGrab(true);

    String machargs;
    gConfig->getConfigString("prom_env_machargs", machargs);
    vt100DisplayEnabled = machargs.findFirstString("-v") != -1;

    EM_ASM_({ workerApi.didOpenVideo($0, $1); }, mClientChar.width, mClientChar.height);
}

void JSSystemDisplay::blit() {
    // See comments in SDLSystemDisplay::displayShow for the need for the +3.
    if (gDamageAreaFirstAddr > gDamageAreaLastAddr+3) {
        EM_ASM({ workerApi.blit(0, 0); });
        return;
    }

    // TODO: take into account damage area and only blit that part.
    healFrameBuffer();

    byte *jsFrameBuffer = this->jsFrameBuffer.get();
    int frameBufferSize =
        mClientChar.width * mClientChar.height * mClientChar.bytesPerPixel;

    // Invert endianess and generate the RGBA that the JS side expects.
    if (mClientChar.bytesPerPixel == 2) {
        for (int src = 0, dst = 0; src < frameBufferSize; src += 2, dst += 4) {
            uint16 pixel555 = gFrameBuffer[src + 1] | (gFrameBuffer[src] << 8);
            uint8 red5 = (pixel555 & 0x7c00) >> 10;
            uint8 green5 = (pixel555 & 0x3e0) >> 5;
            uint8 blue5 = (pixel555 & 0x1f);

            uint8 red8 = (red5 * 255 + 15) / 31;
            uint8 green8 = (green5 * 255 + 15) / 31;
            uint8 blue8 = (blue5 * 255 + 15) / 31;

            jsFrameBuffer[dst + 0] = red8;
            jsFrameBuffer[dst + 1] = green8;
            jsFrameBuffer[dst + 2] = blue8;
            jsFrameBuffer[dst + 3] = 0xff;
        }
    } else {
        // Offset by 1 to go from ARGB to RGBA and ensure that the alpha is set,
        // the Mac defaults to it being 0, which the browser renders as
        // transparent.
        uint32 *jsFrameBuffer32 = reinterpret_cast<uint32 *>(jsFrameBuffer);
        byte *jsFrameBufferAlpha = jsFrameBuffer + 3;
        uint32 *frameBuffer32 = reinterpret_cast<uint32 *>(gFrameBuffer + 1);
        uint32 frameBufferSize32 = frameBufferSize / 4;
        for (int i = 0; i < frameBufferSize32; i++) {
            *jsFrameBuffer32++ = *frameBuffer32++;
            *jsFrameBufferAlpha = 0xFF;
            jsFrameBufferAlpha += 4;
        }
    }

    EM_ASM_({ workerApi.blit($0, $1); }, jsFrameBuffer, jsFrameBufferSize);
}

void JSSystemDisplay::finishMenu() {
}

void JSSystemDisplay::updateTitle() {
}

void JSSystemDisplay::displayShow() {
    blit();
}

void JSSystemDisplay::convertCharacteristicsToHost(DisplayCharacteristics &aHostChar, const DisplayCharacteristics &aClientChar) {
    aHostChar = aClientChar;
    aHostChar.bytesPerPixel = 4;
    aHostChar.scanLineLength = aHostChar.bytesPerPixel * aHostChar.width;
}

bool JSSystemDisplay::changeResolution(const DisplayCharacteristics &aCharacteristics) {
    mClientChar = aCharacteristics;
    gFrameBuffer = (byte*)realloc(
        gFrameBuffer,
        mClientChar.width * mClientChar.height * mClientChar.bytesPerPixel);

    jsFrameBufferSize = mClientChar.width * mClientChar.height * 4;
    jsFrameBuffer.reset(new byte[jsFrameBufferSize]);

    EM_ASM_({ workerApi.didOpenVideo($0, $1); }, mClientChar.width, mClientChar.height);
    damageFrameBufferAll();
    return true;
}

void JSSystemDisplay::getHostCharacteristics(Container &modes) {
    DisplayCharacteristics *hostChar = new DisplayCharacteristics();
    convertCharacteristicsToHost(*hostChar, mClientChar);
    modes.insert(hostChar);
}

void JSSystemDisplay::setMouseGrab(bool enable) {
    // Mouse is always grabbed as far as PearPC is concerned, we just don't sent
    // events if it's not.
    SystemDisplay::setMouseGrab(true);
}

void JSSystemDisplay::drawChar(int x, int y, vcp color, byte chr) {
    if (vt100DisplayEnabled) {
        SystemDisplay::drawChar(x, y, color, chr);
    }
}

void JSSystemDisplay::fillVT(int x, int y, int w, int h, vcp color, byte chr) {
    if (vt100DisplayEnabled) {
        SystemDisplay::fillVT(x, y, w, h, color, chr);
    }
}

void JSSystemDisplay::fillAllVT(vcp color, byte chr) {
    if (vt100DisplayEnabled) {
        SystemDisplay::fillAllVT(color, chr);
    }
}

SystemDisplay *allocSystemDisplay(const char *title, const DisplayCharacteristics &chr, int redraw_ms) {
    return new JSSystemDisplay(chr, redraw_ms);
}
