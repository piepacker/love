/**
 * Copyright (c) 2006-2023 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

// LOVE
#include "common/config.h"
#include "graphics/Graphics.h"
#include "Window.h"

namespace love
{
namespace window
{
namespace libretro
{

Window::Window()
    : open(false)
    , window(nullptr)
{
}

Window::~Window()
{
}

void Window::setGraphics(graphics::Graphics *graphics)
{
	this->graphics.set(graphics);
}

bool Window::setWindow(int width, int height, WindowSettings *settings)
{
	if (!graphics.get())
		graphics.set(Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS));

	if (graphics.get() && graphics->isCanvasActive())
		throw love::Exception("love.window.setMode cannot be called while a Canvas is active in love.graphics.");

	WindowSettings f;

	if (settings)
		f = *settings;

    // TODO create window + context ?
    open = true;

    return true;
}

void Window::getWindow(int &width, int &height, WindowSettings &newsettings)
{
	// The window might have been modified (moved, resized, etc.) by the user.
	if (window)
		updateSettings(settings, true);

	width = windowWidth;
	height = windowHeight;
	newsettings = settings;
}


void Window::close()
{
	close(true);
}

void Window::close(bool allowExceptions)
{
	if (graphics.get())
	{
		if (allowExceptions && graphics->isCanvasActive())
			throw love::Exception("love.window.close cannot be called while a Canvas is active in love.graphics.");

		graphics->unSetMode();
	}

    // TODO destroy window/context ? 
	open = false;
}

bool Window::setFullscreen(bool fullscreen)
{
	return setFullscreen(fullscreen, settings.fstype);
}

bool Window::setFullscreen(bool fullscreen, Window::FullscreenType fstype)
{
	if (!window)
		return false;

	if (graphics.get() && graphics->isCanvasActive())
		throw love::Exception("love.window.setFullscreen cannot be called while a Canvas is active in love.graphics.");

    return true;
}

int Window::getDisplayCount() const
{
	return 1;
}

void Window::updateSettings(const WindowSettings &newsettings, bool updateGraphicsViewport)
{
    // TODO
}

bool Window::onSizeChanged(int width, int height)
{
	if (!window)
		return false;

	windowWidth = width;
	windowHeight = height;

	//SDL_GL_GetDrawableSize(window, &pixelWidth, &pixelHeight);

	if (graphics.get())
	{
		double scaledw, scaledh;
		fromPixels((double) pixelWidth, (double) pixelHeight, scaledw, scaledh);
		graphics->setViewportSize((int) scaledw, (int) scaledh, pixelWidth, pixelHeight);
	}

	return true;
}

bool Window::hasFocus() const
{
	//return (window && SDL_GetKeyboardFocus() == window);
    return true;
}

bool Window::hasMouseFocus() const
{
	//return (window && SDL_GetMouseFocus() == window);
    return false;
}

bool Window::isVisible() const
{
	//return window && (SDL_GetWindowFlags(window) & SDL_WINDOW_SHOWN) != 0;
    return true;
}

void Window::setMouseGrab(bool grab)
{
	//mouseGrabbed = grab;
	//if (window)
	//	SDL_SetWindowGrab(window, (SDL_bool) grab);
}

bool Window::isMouseGrabbed() const
{
	//if (window)
	//	return SDL_GetWindowGrab(window) != SDL_FALSE;
	//else
	//	return mouseGrabbed;
    return false;
}

int Window::getWidth() const
{
	return windowWidth;
}

int Window::getHeight() const
{
	return windowHeight;
}

int Window::getPixelWidth() const
{
	return pixelWidth;
}

int Window::getPixelHeight() const
{
	return pixelHeight;
}


void Window::windowToPixelCoords(double *x, double *y) const
{
	if (x != nullptr)
		*x = (*x) * ((double) pixelWidth / (double) windowWidth);
	if (y != nullptr)
		*y = (*y) * ((double) pixelHeight / (double) windowHeight);
}

void Window::pixelToWindowCoords(double *x, double *y) const
{
	if (x != nullptr)
		*x = (*x) * ((double) windowWidth / (double) pixelWidth);
	if (y != nullptr)
		*y = (*y) * ((double) windowHeight / (double) pixelHeight);
}

void Window::windowToDPICoords(double *x, double *y) const
{
	double px = x != nullptr ? *x : 0.0;
	double py = y != nullptr ? *y : 0.0;

	windowToPixelCoords(&px, &py);

	double dpix = 0.0;
	double dpiy = 0.0;

	fromPixels(px, py, dpix, dpiy);

	if (x != nullptr)
		*x = dpix;
	if (y != nullptr)
		*y = dpiy;
}

void Window::DPIToWindowCoords(double *x, double *y) const
{
	double dpix = x != nullptr ? *x : 0.0;
	double dpiy = y != nullptr ? *y : 0.0;

	double px = 0.0;
	double py = 0.0;

	toPixels(dpix, dpiy, px, py);
	pixelToWindowCoords(&px, &py);

	if (x != nullptr)
		*x = px;
	if (y != nullptr)
		*y = py;
}

double Window::getDPIScale() const
{
	return settings.usedpiscale ? getNativeDPIScale() : 1.0;
}

double Window::getNativeDPIScale() const
{
#ifdef LOVE_ANDROID
	return love::android::getScreenScale();
#else
	return (double) pixelHeight / (double) windowHeight;
#endif
}

double Window::toPixels(double x) const
{
	return x * getDPIScale();
}

void Window::toPixels(double wx, double wy, double &px, double &py) const
{
	double scale = getDPIScale();
	px = wx * scale;
	py = wy * scale;
}

double Window::fromPixels(double x) const
{
	return x / getDPIScale();
}

void Window::fromPixels(double px, double py, double &wx, double &wy) const
{
	double scale = getDPIScale();
	wx = px / scale;
	wy = py / scale;
}

const void *Window::getHandle() const
{
	return window;
}

void Window::minimize()
{
//	if (window != nullptr)
//		SDL_MinimizeWindow(window);
}

void Window::maximize()
{
	if (window != nullptr)
	{
		//SDL_MaximizeWindow(window);
		updateSettings(settings, true);
	}
}

void Window::restore()
{
	if (window != nullptr)
	{
		//SDL_RestoreWindow(window);
		updateSettings(settings, true);
	}
}

bool Window::isMaximized() const
{
	//return window != nullptr && (SDL_GetWindowFlags(window) & SDL_WINDOW_MAXIMIZED);
    return false;
}

bool Window::isMinimized() const
{
	//return window != nullptr && (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED);
    return false;
}

void Window::setDisplaySleepEnabled(bool enable)
{
	//if (enable)
	//	SDL_EnableScreenSaver();
	//else
	//	SDL_DisableScreenSaver();
}

bool Window::isDisplaySleepEnabled() const
{
	//return SDL_IsScreenSaverEnabled() != SDL_FALSE;
    return false;
}

void Window::setWindowTitle(const std::string &title)
{
	this->title = title;

	//if (window)
	//	SDL_SetWindowTitle(window, title.c_str());
}


bool Window::showMessageBox(const std::string &title, const std::string &message, MessageBoxType type, bool attachtowindow)
{
    return false;
}

int Window::showMessageBox(const MessageBoxData &data)
{
    return 0; // OR maybe -2
}

int Window::getVSync() const
{
	//return context != nullptr ? SDL_GL_GetSwapInterval() : 0;
    return 0;
}

std::vector<Window::WindowSize> Window::getFullscreenSizes(int displayindex) const
{
	std::vector<WindowSize> sizes;
    WindowSize w = {windowWidth, windowHeight};
    sizes.push_back(w);

	//for (int i = 0; i < SDL_GetNumDisplayModes(displayindex); i++)
	//{
	//	SDL_DisplayMode mode = {};
	//	SDL_GetDisplayMode(displayindex, i, &mode);

	//	WindowSize w = {mode.w, mode.h};

	//	// SDL2's display mode list has multiple entries for modes of the same
	//	// size with different bits per pixel, so we need to filter those out.
	//	if (std::find(sizes.begin(), sizes.end(), w) == sizes.end())
	//		sizes.push_back(w);
	//}

	return sizes;
}

void Window::getDesktopDimensions(int displayindex, int &width, int &height) const
{
	if (displayindex >= 0 && displayindex < getDisplayCount())
	{
		//SDL_DisplayMode mode = {};
		//SDL_GetDesktopDisplayMode(displayindex, &mode);
		//width = mode.w;
		//height = mode.h;
		width = windowWidth; // RANDOM VALUE
		height = windowHeight;
	}
	else
	{
		width = 0;
		height = 0;
	}
}

love::image::ImageData *Window::getIcon()
{
	//return icon.get();
    return nullptr;
}

bool Window::setIcon(love::image::ImageData *imgd)
{
	if (!imgd)
		return false;

	if (imgd->getFormat() != PIXELFORMAT_RGBA8)
		throw love::Exception("setIcon only accepts 32-bit RGBA images.");

#if 0
	icon.set(imgd);

	if (!window)
		return false;

	uint32 rmask, gmask, bmask, amask;
#ifdef LOVE_BIG_ENDIAN
	rmask = 0xFF000000;
	gmask = 0x00FF0000;
	bmask = 0x0000FF00;
	amask = 0x000000FF;
#else
	rmask = 0x000000FF;
	gmask = 0x0000FF00;
	bmask = 0x00FF0000;
	amask = 0xFF000000;
#endif

	int w = imgd->getWidth();
	int h = imgd->getHeight();
	int bytesperpixel = (int) getPixelFormatSize(imgd->getFormat());
	int pitch = w * bytesperpixel;

	SDL_Surface *sdlicon = nullptr;

	{
		// We don't want another thread modifying the ImageData mid-copy.
		love::thread::Lock lock(imgd->getMutex());
		sdlicon = SDL_CreateRGBSurfaceFrom(imgd->getData(), w, h, bytesperpixel * 8, pitch, rmask, gmask, bmask, amask);
	}

	if (!sdlicon)
		return false;

	SDL_SetWindowIcon(window, sdlicon);
	SDL_FreeSurface(sdlicon);

	return true;
#else
    return false;
#endif
}

Window::DisplayOrientation Window::getDisplayOrientation(int displayindex) const
{
#if 0
	// TODO: We can expose this everywhere, we just need to watch out for the
	// SDL binary being older than the headers on Linux.
#if SDL_VERSION_ATLEAST(2, 0, 9) && (defined(LOVE_ANDROID) || !defined(LOVE_LINUX))
	switch (SDL_GetDisplayOrientation(displayindex))
	{
		case SDL_ORIENTATION_UNKNOWN: return ORIENTATION_UNKNOWN;
		case SDL_ORIENTATION_LANDSCAPE: return ORIENTATION_LANDSCAPE;
		case SDL_ORIENTATION_LANDSCAPE_FLIPPED: return ORIENTATION_LANDSCAPE_FLIPPED;
		case SDL_ORIENTATION_PORTRAIT: return ORIENTATION_PORTRAIT;
		case SDL_ORIENTATION_PORTRAIT_FLIPPED: return ORIENTATION_PORTRAIT_FLIPPED;
	}
#else
	LOVE_UNUSED(displayindex);
#endif
#endif

	return ORIENTATION_UNKNOWN;
}

Rect Window::getSafeArea() const
{
#if defined(LOVE_IOS)
	if (window != nullptr)
		return love::ios::getSafeArea(window);
#elif defined(LOVE_ANDROID)
	if (window != nullptr)
	{
		int top, left, bottom, right;

		if (love::android::getSafeArea(top, left, bottom, right))
		{
			// DisplayCutout API returns safe area in pixels
			// and is affected by display orientation.
			double safeLeft, safeTop, safeWidth, safeHeight;
			fromPixels(left, top, safeLeft, safeTop);
			fromPixels(pixelWidth - left - right, pixelHeight - top - bottom, safeWidth, safeHeight);
			return {(int) safeLeft, (int) safeTop, (int) safeWidth, (int) safeHeight};
		}
	}
#endif

	double dw, dh;
	fromPixels(pixelWidth, pixelHeight, dw, dh);
	return {0, 0, (int) dw, (int) dh};
}

bool Window::isOpen() const
{
	return open;
}

void Window::requestAttention(bool continuous)
{
	LOVE_UNUSED(continuous);
}

const std::string &Window::getWindowTitle() const
{
	return title;
}

const char *Window::getDisplayName(int displayindex) const
{
    return "dispaly name";
}

void Window::getPosition(int &x, int &y, int &displayindex)
{
    x = y =  0;
    displayindex = 0;
}

const char *Window::getName() const
{
	return "love.window.libretro";
}

void Window::setPosition(int x, int y, int displayindex)
{
}

void Window::setVSync(int vsync)
{
}

void Window::swapBuffers()
{
}

} // namespace libretro
} // namespace window
} // namespace love
