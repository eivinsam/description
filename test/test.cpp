#include <description.h>

#include <vector>
#include <string>
#include <iostream>

class JsonOut : public desc::Serializer
{
	enum class State : char { value, seqStart, mapStart, mapKey, mapValue, done };
	std::vector<State> _stack;
	std::ostream& _out;
	void _next()
	{
		switch (_stack.back())
		{
		case State::value: 
		case State::mapValue:
			_out << ",\n" << _indent();
			break;
		case State::mapKey: 
			_out << ": ";
			break;
		default:
			_out << _indent();
			break;
		}
	}
	void _commit()
	{
		switch (_stack.back())
		{
		case State::seqStart: _stack.back() = State::value; return;
		case State::mapStart: _stack.back() = State::mapKey; return;
		case State::mapKey: _stack.back() = State::mapValue; return;
		case State::mapValue: _stack.back() = State::mapKey; return;
		default: return;
		}
	}
	std::string _indent() const { return std::string((_stack.size() - 1) * 2, ' '); }
public:
	JsonOut(std::ostream& out) : _stack{ State::seqStart }, _out{ out } { }

	// Inherited via Serializer
	virtual void beginSequence() override
	{
		_next();
		_stack.push_back(State::seqStart);
		_out << "[\n";
	}
	virtual void endSequence() override
	{
		_stack.pop_back();
		_commit();
		_out << "\n" << _indent() << "]";
	}
	virtual void beginMap() override
	{
		_next();
		_stack.push_back(State::mapStart);
		_out << "{\n";
	}
	virtual void endMap() override
	{
		_stack.pop_back();
		_commit();
		_out << "\n" << _indent() << "]";
	}
	virtual void _write(std::string_view text) override
	{
		_next();
		_commit();
		_out << '"' << text << '"';
	}
	virtual void _write(long long value) override
	{
		_next();
		_commit();
		_out << value;
	}
	virtual void _write(double value) override
	{
		_next();
		_commit();
		_out << value;
	}
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
