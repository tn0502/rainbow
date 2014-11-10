// Copyright (c) 2010-14 Bifrost Entertainment AS and Tommy Nguyen
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

#ifndef COMMON_VEC2_H_
#define COMMON_VEC2_H_

#include "Common/Algorithm.h"

namespace Rainbow
{
	/// Structure for storing a two-dimensional vector.
	template<typename T, typename Enable = void>
	struct Vec2;

	template<typename T>
	struct Vec2<T, Arithmetic<T>>
	{
		union
		{
			T x;
			T width;
		};

		union
		{
			T y;
			T height;
		};

		Vec2() : Vec2(0, 0) { }
		Vec2(const T x, const T y) : x(x), y(y) { }

		/// Returns the angle (in radians) between two points.
		float angle(const Vec2 &v) const
		{
			return atan2f(v.y - this->y, v.x - this->x);
		}

		/// Returns the distance between two points.
		float distance(const Vec2 &v) const
		{
			T dx = v.x - this->x;
			T dy = v.y - this->y;
			return sqrt(dx * dx + dy * dy);
		}

		/// Returns the dot product of two vectors
		T dot(const Vec2 &v) const
		{
			return this->x * v.x + this->y * v.y;
		}

		template<typename U = T>
		EnableIfIntegral<U, bool> is_zero() const
		{
			return this->x == 0 && this->y == 0;
		}

		template<typename U = T>
		EnableIfFloatingPoint<U, bool> is_zero() const
		{
			return is_equal<T>(0.0, this->x) && is_equal<T>(0.0, this->y);
		}

		Vec2& operator+=(const Vec2 &v)
		{
			this->x += v.x;
			this->y += v.y;
			return *this;
		}

		Vec2& operator-=(const Vec2 &v)
		{
			this->x -= v.x;
			this->y -= v.y;
			return *this;
		}

		Vec2& operator*=(const T &f)
		{
			this->x *= f;
			this->y *= f;
			return *this;
		}

		friend bool operator==(const Vec2 &a, const Vec2 &b)
		{
			return a.x == b.x && a.y == b.y;
		}

		friend Vec2 operator+(const Vec2 &a, const T offset)
		{
			return Vec2(a.x + offset, a.y + offset);
		}
	};
}

using Vec2f = Rainbow::Vec2<float>;
using Vec2i = Rainbow::Vec2<int>;

#endif
