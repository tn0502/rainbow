-- Copyright (c) 2010-14 Bifrost Entertainment AS and Tommy Nguyen
-- Distributed under the MIT License.
-- (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

local format = string.format
local pairs = pairs
local tonumber = tonumber
local type = type

local rainbow = rainbow
local audio = rainbow.audio
local scenegraph = rainbow.scenegraph

local FMOD = FMOD

local SoundFinalizer = {
	__gc = function(self) audio.delete_sound(self) end
}

local function endswith(str, ending)
	return ending == '' or str:sub(-ending:len()) == ending
end

local function extend(data)
	return setmetatable(
		{ __userdata = data },
		{ __index = function(self, key)
			local field = self.__userdata[key]
			if not field then
				return nil
			end
			self[key] = function(self, ...)
				return field(self.__userdata, ...)
			end
			return self[key]
		end })
end

local function insert(t, k, v)
	t[k] = t[k] and error(format("'%s' is defined twice", k)) or v
end

local function merge(t1, t2)
	for k,v in pairs(t2) do
		insert(t1, k, v)
	end
end

local function setproperties(obj, def)
	if def.alignment then
		obj:set_alignment(def.alignment)
	end
	if def.animations then
		obj.animations = {}
		for k,v in pairs(def.animations) do
			obj.animations[k] = rainbow.animation(obj, v, v.fps, v.delay or 0)
			obj.animations[k]:stop()
		end
	end
	if def.color then
		local r = tonumber(def.color:sub(1, 2), 16)
		local g = tonumber(def.color:sub(3, 4), 16)
		local b = tonumber(def.color:sub(5, 6), 16)
		local a = tonumber(def.color:sub(7, 8), 16)
		obj:set_color(r, g, b, a)
	end
	if def.pivot then
		local x, y = 1, 2
		obj:set_pivot(def.pivot[x], def.pivot[y])
	end
	if def.position then
		local x, y = 1, 2
		obj:set_position(def.position[x], def.position[y])
	end
	if def.rotation then
		obj:set_rotation(def.rotation)
	end
	if def.scale then
		obj:set_scale(def.scale)
	end
	if def.texture then
		obj:set_texture(def.texture)
	end
end

local function createbatch(node, resources)
	local width, height = 1, 2
	local batch = extend(rainbow.spritebatch(#node.sprites))
	batch:set_texture(resources[node.texture])
	local sprites = {}
	for i = 1, #node.sprites do
		local def = node.sprites[i]
		local sprite =
		    extend(batch:create_sprite(def.size[width], def.size[height]))
		setproperties(sprite, def)
		insert(sprites, def.name, sprite)
	end
	return batch, sprites
end

local function createfont(def)
	local path, size = 1, 2
	return rainbow.font(def[path], def[size])
end

local function createlabel(def, resources)
	local label = extend(def.text and rainbow.label(def.text)
	                               or rainbow.label())
	label:set_font(resources[def.font])
	setproperties(label, def)
	return label
end

local createsound =
    FMOD and function(snd)
                 return FMOD.createSound(snd)
             end
          or function(snd)
                 return setmetatable(audio.create_sound(snd), SoundFinalizer)
             end

local function createtexture(def)
	local path = 1
	local x, y, width, height = 1, 2, 3, 4
	local texture = rainbow.texture(def[path])
	for i = 2, #def do
		texture:create(def[i][x], def[i][y], def[i][width], def[i][height])
	end
	return texture
end

local function createresources(resources)
	local t = {}
	for name,resource in pairs(resources) do
		local path = type(resource) ~= "table" and resource or resource[1]
		if endswith(path, ".png") or endswith(path, ".pvr") then
			insert(t, name, createtexture(resource))
		elseif endswith(path, ".otf") or endswith(path, ".ttf") then
			insert(t, name, createfont(resource))
		elseif endswith(path, ".ogg") or endswith(path, ".mp3") then
			insert(t, name, createsound(resource))
		else
			print("Unknown resource: " .. path)
		end
	end
	return t
end

local function createnodes(parent, resources, nodes)
	local t = {}
	for i = 1, #nodes do
		local node = nodes[i]
		if node.sprites then
			local batch, sprites = createbatch(node, resources)
			merge(t, sprites)
			batch.node = scenegraph:add_batch(parent, batch)
			for _,sprite in pairs(sprites) do
				for _,animation in pairs(sprite.animations or {}) do
					scenegraph:add_animation(batch.node, animation)
				end
			end
			insert(t, node.name, batch)
		elseif node.font then
			local label = createlabel(node, resources)
			label.node = scenegraph:add_label(parent, label)
			insert(t, node.name, label)
		elseif node.nodes then
			local group = scenegraph:add_node(parent)
			merge(t, createnodes(group, resources, node.nodes))
			insert(t, node.name, group)
		else
			print("Unknown element: " .. node.name)
		end
	end
	return t
end

return {
	createresources = createresources,
	createnodes = createnodes,
	version = 1.1
}
