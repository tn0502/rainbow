// Copyright 2011-13 Bifrost Entertainment. All rights reserved.

#include "Algorithm.h"
#include "Graphics/Animation.h"
#include "Graphics/Label.h"
#include "Graphics/SceneGraph.h"
#include "Graphics/SpriteBatch.h"

using Rainbow::equalf;

namespace SceneGraph
{
	Node::~Node()
	{
		for (size_t i = 0; i < this->children.size(); ++i)
			delete this->children[i];
	}

	void Node::draw()
	{
		if (!this->enabled)
			return;

		switch (this->type)
		{
			case DrawableNode:
			case LabelNode:
				this->drawable->draw();
				break;
			case SpriteBatchNode:
				this->sprite_batch->draw();
				break;
			default:
				break;
		}
		for (size_t i = 0; i < this->children.size(); ++i)
			this->children[i]->draw();
	}

	void Node::move(const float x, const float y)
	{
		if (equalf(x, 0.0f) && equalf(y, 0.0f))
			return;

		switch (this->type)
		{
			case LabelNode:
				this->label->move(x, y);
				break;
			case SpriteNode:
				this->sprite->move(x, y);
				break;
			case SpriteBatchNode:
				{
					Vector<Sprite*> &sprites = this->sprite_batch->sprites;
					for (size_t i = 0; i < sprites.size(); ++i)
						sprites[i]->move(x, y);
				}
				break;
			default:
				break;
		}
		for (size_t i = 0; i < this->children.size(); ++i)
			this->children[i]->move(x, y);
	}

	void Node::rotate(const float r)
	{
		if (equalf(r, 0.0f))
			return;

		switch (this->type)
		{
			case SpriteNode:
				this->sprite->rotate(r);
				break;
			case SpriteBatchNode:
				{
					Vector<Sprite*> &sprites = this->sprite_batch->sprites;
					for (size_t i = 0; i < sprites.size(); ++i)
						sprites[i]->rotate(r);
				}
				break;
			default:
				break;
		}
		for (size_t i = 0; i < this->children.size(); ++i)
			this->children[i]->rotate(r);
	}

	void Node::scale(const float f)
	{
		switch (this->type)
		{
			case SpriteNode:
				this->sprite->set_scale(f);
				break;
			case SpriteBatchNode:
				{
					Vector<Sprite*> &sprites = this->sprite_batch->sprites;
					for (size_t i = 0; i < sprites.size(); ++i)
						sprites[i]->set_scale(f);
				}
				break;
			default:
				break;
		}
		for (size_t i = 0; i < this->children.size(); ++i)
			this->children[i]->scale(f);
	}

	void Node::update(const unsigned long dt)
	{
		if (!this->enabled)
			return;

		switch (this->type)
		{
			case AnimationNode:
				this->animation->update(dt);
				break;
			case DrawableNode:
			case LabelNode:
				this->drawable->update();
				break;
			case SpriteBatchNode:
				this->sprite_batch->update();
				break;
			default:
				break;
		}
		for (size_t i = 0; i < this->children.size(); ++i)
			this->children[i]->update(dt);
	}

	Node& Node::operator=(const Node &n)
	{
		this->type = n.type;
		this->data = n.data;
		return *this;
	}
}
