#include <description.h>

#include <vector>
#include <string>
#include <iostream>

class JsonOut
{
	std::ostream& _out;
	int _indent = 0;
	bool _break = false;

	template <class T, class MemberT>
	void _write_member(const T& value, const desc::Member<T, MemberT>& member, bool first)
	{
		if (!first) _out << ',';
		write(member.name);
		_out << ": ";
		_break = false;
		write(value.*member.offset);
	}

	template <class T, class Last>
	void _write_object(const T& value, const desc::MemberList<T, Last>& members, bool first)
	{
		_write_member(value, members.last, first);
	}

	template <class T, class First, class... Rest>
	void _write_object(const T& value, const desc::MemberList<T, First, Rest...>& members, bool first)
	{
		_write_member(value, members.first, first);
		_write_object(value, members.rest, false);
	}

	template <class T>
	void _write(const T& value, const desc::ObjectDescription&)
	{
		static constexpr auto members = T::members();
		_out << "{";
		_break = true;
		++_indent;
		_write_object(value, members, true);
		--_indent;
		_newline();
		_out << "}";
	}
	template <class T>
	void _write(const T& value, const desc::ScalarDescription&)
	{
		_out << value;
	}
	void _newline()
	{
		if (_break)
		{
			_out << '\n';
			for (int left = _indent * 2; left > 0; --left)
				_out << ' ';
			_break = false;
		}
	}
public:
	JsonOut(std::ostream& out) : _out{ out } { }

	template <class T>
	void write(const T& value) { _newline(); _write(value, desc::Description<T>{}); _break = true; }
};

struct TestStruct
{
	int a;
	float b;

	static constexpr auto members()
	{
		return desc::members("a", &TestStruct::a, "b", &TestStruct::b);
	}
};

int main(int argc, char* argv[])
{
	TestStruct test{ 1, 2 };
	JsonOut out(std::cout);

	out.write(test);

	return 0;
}
