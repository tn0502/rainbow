// Copyright 2011-13 Bifrost Entertainment. All rights reserved.

#include "Director.h"
#include "ConFuoco/Mixer.h"
#include "Graphics/TextureManager.h"
#include "Lua/lua_Platform.h"

namespace Rainbow
{
	void Director::init(const Data &main, const int width, const int height)
	{
		this->set_video(width, height);
		if (this->lua.init(main) != LUA_OK || this->lua.update(0) != LUA_OK)
			this->shutdown();
		else
			this->scenegraph.update(0);
	}

	void Director::set_video(const int w, const int h)
	{
		Lua::Platform::update(this->lua, w, h);
	}

	void Director::update(const unsigned long dt)
	{
		ConFuoco::Mixer::Instance->update();
		if (this->lua.update(dt))
			this->shutdown();
		else
			this->scenegraph.update(dt);
	}

	void Director::on_memory_warning()
	{
		lua_gc(this->lua, LUA_GCCOLLECT, 0);
		TextureManager::Instance().purge();
	}
}
