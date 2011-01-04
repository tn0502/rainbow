/// A generic game element.

/// Copyright 2010 Bifrost Games. All rights reserved.
/// \author Tommy Nguyen

#ifndef ELEMENT_H_
#define ELEMENT_H_

#include <Rainbow/Sprite.h>

class Element
{
public:
	bool active;  ///< Whether this element is active

	Element() : active(false), sprite(0) { }
	~Element() { }

	virtual void activate() = 0;
	virtual void fire() = 0;

	void set_sprite(Sprite *s)
	{
		this->sprite = s;
	}

protected:
	Sprite *sprite;  ///< The element's sprite
};

#endif
