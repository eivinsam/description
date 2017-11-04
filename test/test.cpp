#include <description.h>

#include <vector>
#include <string>
#include <iostream>
#include <sstream>

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

	void write(const char* text)        { _newline(); _out << '"' << text << '"'; _break = true; }
	void write(std::string_view text)   { _newline(); _out << '"' << text << '"'; _break = true; }
	void write(const std::string& text) { _newline(); _out << '"' << text << '"'; _break = true; }
	template <class T>
	void write(const T& value) { _newline(); _write(value, desc::Description<T>{}); _break = true; }
};

class JsonIn
{
	std::istream& _in;
	void _skipws()
	{
		for (;;) switch (_in.peek())
		{
		case ' ': case '\t': case '\r': case '\n': 
			_in.get(); 
			continue;
		default: 
			return;
		}
	}


	template <class T>
	void _read_colon_value(T& value)
	{
		std::string sep;
		_in >> sep;
		if (sep != ":")
			throw std::runtime_error("expected key-value separator");
		read(value);
	}

	template <class T, class Last>
	bool _read_object(T& value, const desc::MemberList<T, Last>& members, const std::string& name)
	{
		if (name != members.last.name)
			return false;
		_read_colon_value(value.*members.last.offset);
		return true;
	}
	template <class T, class First, class... Rest>
	bool _read_object(T& value, const desc::MemberList<T, First, Rest...>& members, const std::string& name)
	{
		if (name != members.first.name)
			return _read_object(value, members.rest, name);
		_read_colon_value(value.*members.first.offset);
		return true;
	}

	template <class T, class... Members>
	bool _read_object_check(T& value, const desc::MemberList<T, Members...>& members, bool first)
	{
		_skipws();
		if (_in.peek() == '}')
			return false;
		if (!first)
		{
			if (_in.get() != ',')
				throw std::runtime_error("expected comma");
			_in.get();
		}
		else
		{
			if (_in.peek() != '"')
				throw std::runtime_error("expected string key");
		}
		std::string name;
		read(name);
		if (!_read_object(value, members, name))
			throw std::runtime_error("unknown member " + name);
		return true;
	}
	
	template <class T>
	void _read(T& value, const desc::ObjectDescription&)
	{
		static constexpr auto members = T::members();
		_skipws();
		if (_in.get() != '{')
			throw std::runtime_error("expected start of map");
		for (bool first = true; _read_object_check(value, members, first); first = false);
		if (_in.get() != '}')
			throw std::runtime_error("expected end of map");
	}
	template <class T>
	void _read(T& value, const desc::ScalarDescription&)
	{
		_in >> value;
	}
public:
	JsonIn(std::istream& in) : _in(in) { }

	void read(std::string& value)
	{
		_skipws();
		if (_in.get() != '"')
			throw std::runtime_error("expected start of string");
		value.clear();
		for (char ch = _in.get(); _in.good() && ch != '"'; ch = _in.get())
		{
			if (ch == '\\')
				throw std::runtime_error("escape sequences not supported");
			value.push_back(ch);
		}
	}
	template <class T>
	void read(T& value) { _read(value, desc::Description<T>{}); }

};

struct TestStruct
{
	struct SubStruct
	{
		double x;
		std::string text;

		static constexpr auto members()
		{
			return desc::members(
				"x", &SubStruct::x,
				"text", &SubStruct::text);
		}
	};

	int a;
	float b;
	SubStruct c;


	static constexpr auto members()
	{
		return desc::members(
			"a", &TestStruct::a, 
			"b", &TestStruct::b, 
			"c", &TestStruct::c);
	}
};

int main(int argc, char* argv[])
{
	TestStruct test{ 1, 2, { 3.141592, "foo"} };

	std::istringstream data("{ \"b\": 3, \"c\": { \"text\": \"bar\", \"x\": 2.718281 } }");
	JsonIn in(data);
	try { in.read(test); }
	catch (std::exception& e)
	{
		std::cout << e.what() << '\n';
	}

	JsonOut out(std::cout);

	out.write(test);

	return 0;
}
