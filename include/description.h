#pragma once

#include <type_traits>
#include <string_view>

namespace desc
{
	template <class, class>
	struct Description;

	class Serializer
	{
	protected:
		virtual void _write(std::string_view) = 0;
		virtual void _write(long long) = 0;
		virtual void _write(double) = 0;

		template <class T>
		void _write(const T& obj);
	public:
		virtual ~Serializer() = default;

		virtual void beginSequence() = 0;
		virtual void   endSequence() = 0;

		virtual void beginMap() = 0;
		virtual void   endMap() = 0;

		template <class T>
		void write(T&& arg) { _write(std::forward<T>(arg)); }
	};

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

		void write(Serializer& out, const T& obj) const
		{
			out.write(last.name);
			out.write(obj.*last.offset);
		}
	};

	template <class T, class First, class... Rest>
	struct MemberList<T, First, Rest...>
	{
		Member<T, First> first;
		MemberList<T, Rest...> rest;

		void write(Serializer& out, const T& obj) const
		{
			out.write(first.name);
			out.write(obj.*first.offset);
			rest.write(out, obj);
		}
	};

	template <class T, class First, class... Rest>
	MemberList<T, First, Rest...> members(Member<T, First> first, MemberList<T, Rest...> rest) { return { first, rest }; }

	template <class T, class Last>
	auto members(const char* name, Last T::* offset) { return MemberList<T, Last>{ {name, offset} }; }

	template <class T, class First, class... Rest>
	auto members(const char* first_name, First T::* first_offset, Rest&&... rest)
	{
		return members(Member<T, First>{ first_name, first_offset }, members(std::forward<Rest>(rest)...));
	}

	template <class T, class = void>
	struct Description 
	{
		static void write(Serializer& out, const T& value)
		{
			out.beginMap();
			T::members().write(out, value);
			out.endMap();
		}
	};
	template <>
	struct Description<const char*>
	{
		static void write(Serializer& out, std::string_view text) { out.write(text); }
	};
	template <class T>
	struct Description<T, std::enable_if_t<std::is_integral_v<T>>>
	{
		static void write(Serializer& out, long long value) { out.write(value); }
	};
	template <class T>
	struct Description<T, std::enable_if_t<std::is_floating_point_v<T>>>
	{
		static void write(Serializer& out, double value) { out.write(value); }
	};

	template<class T>
	inline void Serializer::_write(const T & obj)
	{
		Description<T>::write(*this, obj);
	}
}
