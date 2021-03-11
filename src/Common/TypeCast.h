// Copyright (c) 2010-present Bifrost Entertainment AS and Tommy Nguyen
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

#ifndef COMMON_TYPECAST_H_
#define COMMON_TYPECAST_H_

#include <limits>
#include <type_traits>

namespace rainbow
{
    template <typename T, typename U>
    constexpr auto narrow_cast(U value) -> T
    {
        if constexpr (!std::is_same_v<T, U> &&
                      (std::is_signed_v<T> != std::is_signed_v<U> ||
                       std::numeric_limits<T>::digits <
                           std::numeric_limits<U>::digits))
        {
            return static_cast<T>(value);
        }
        else
        {
            return value;
        }
    }

    template <typename T>
    constexpr auto to_underlying_type(T value)
    {
        return static_cast<std::underlying_type_t<T>>(value);
    }

    template <typename T, typename U>
    constexpr auto truncate(U value)
    {
        static_assert(std::is_integral_v<T> && std::is_floating_point_v<U>);
        return static_cast<T>(value);
    }
}  // namespace rainbow

#endif
