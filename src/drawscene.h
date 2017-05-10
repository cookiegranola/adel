/*
Minetest
Copyright (C) 2010-2014 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef DRAWSCENE_H_
#define DRAWSCENE_H_

#include "camera.h"
#include "hud.h"
#include "minimap.h"
#include "irrlichttypes_extrabloated.h"

void init_postprocess(video::IVideoDriver *driver, const v2u32 &screensize, Client &client);
void clean_postprocess(video::IVideoDriver *driver);
void begin_postprocess(video::IVideoDriver *driver, video::SColor color = video::SColor(0, 0, 0, 0));
void apply_effect(video::IVideoDriver *driver, const char* name);

void draw_load_screen(const std::wstring &text, IrrlichtDevice *device,
		gui::IGUIEnvironment *guienv, ITextureSource *tsrc, float dtime = 0,
		int percent = 0, bool clouds = true);

void draw_scene(video::IVideoDriver *driver, scene::ISceneManager *smgr,
		Camera &camera, Client &client, LocalPlayer *player,
		Hud &hud, Minimap &mapper, gui::IGUIEnvironment *guienv,
		const v2u32 &screensize, const video::SColor &skycolor,
		bool show_hud, bool show_minimap);

#endif /* DRAWSCENE_H_ */
