#include "Common/Data.h"
#include "Lua/lua_Font.h"

namespace Rainbow
{
	namespace Lua
	{
		const char Font::class_name[] = "font";
		const LuaMachine::Method<Font> Font::methods[] = {
			{ 0, 0 }
		};

		Font::Font(lua_State *L) : font(new FontAtlas(lua_tonumber(L, 2)))
		{
			const char *const path = Data::get_path(lua_tolstring(L, 1, nullptr));
			Data font(path);
			Data::free(path);
			if (!font)
				luaL_error(L, "rainbow.font: Failed to load font");
			if (!this->font->load(font))
				luaL_error(L, "rainbow.font: Failed to create font texture");
		}
	}
}
