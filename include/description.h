#pragma once

#include <type_traits>
#include <string_view>

namespace desc
{
	template <class T, class MemberT>
	struct Member
	{
		const char* name;
		MemberT T::* offset;
	};

	template <class T, class... Members>
	struct MemberList { };

	template <class T, class Last>
	struct MemberList<T, Last>
	{
		Member<T, Last> last;
	};

	template <class T, class First, class... Rest>
	struct MemberList<T, First, Rest...>
	{
		Member<T, First> first;
		MemberList<T, Rest...> rest;
	};

	template <class T, class First, class... Rest>
	constexpr MemberList<T, First, Rest...> members(Member<T, First> first, MemberList<T, Rest...> rest) { return { first, rest }; }

	template <class T, class Last>
	constexpr auto members(const char* name, Last T::* offset) { return MemberList<T, Last>{ {name, offset} }; }

	template <class T, class First, class... Rest>
	constexpr auto members(const char* first_name, First T::* first_offset, Rest&&... rest)
	{
		return members(Member<T, First>{ first_name, first_offset }, members(std::forward<Rest>(rest)...));
	}

	struct ObjectDescription { };
	struct ScalarDescription { };

	template <class T, class = void>
	struct Description : ObjectDescription
	{

	};
	template <>
	struct Description<const char*> : ScalarDescription
	{
	};
	template <class T>
	struct Description<T, std::enable_if_t<std::is_arithmetic_v<T>>> : ScalarDescription
	{

	};
}
