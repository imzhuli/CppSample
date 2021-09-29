#include <zec/Util/Command.hpp>
#include <iostream>

using namespace zec;
using namespace std;

int main(int argc, char * argv[])
{

    // -abcv "Value for V" Arg1 --LongValue "LongValue" Arg2 --EmptyValue)
    // -abcv "Value for V" Arg1 --LongValue "LongValue" Arg2 -- SubCmd1 SubCmd2)
	xCommandLine CommandLine(argc, argv, {
		{ 'a', nullptr, "OptA", false },
		{ 'b', nullptr, "OptB", false },
		{ 'c', nullptr, "OptC", false },
		{ 'v', nullptr, "OptV", true  },
		{ 0, "LongValue", "OptV", true },
		{ 'e', "EmptyValue", "OptE", true},
		{ 'n', "NoValue", "OptN", true},
	});

	xCommandLine::xOptionValue OptionValue;
	OptionValue = CommandLine["OptA"];
	cout << "OptA has value: " << YN(OptionValue()) << "  " << OptionValue.Or("") << endl;
	OptionValue = CommandLine["OptB"];
	cout << "OptB has value: " << YN(OptionValue()) << "  " << OptionValue.Or("") << endl;
	OptionValue = CommandLine["OptC"];
	cout << "OptC has value: " << YN(OptionValue()) << "  " << OptionValue.Or("") << endl;
	OptionValue = CommandLine["OptV"];
	cout << "OptV has value: " << YN(OptionValue()) << "  " << OptionValue.Or("") << endl;
	OptionValue = CommandLine["OptE"];
	cout << "OptE has value: " << YN(OptionValue()) << "  " << OptionValue.Or("") << endl;
	OptionValue = CommandLine["OptN"];
	cout << "OptN has value: " << YN(OptionValue()) << "  " << OptionValue.Or("") << endl;

	size_t Argc = CommandLine.GetArgCount();
	for (size_t i = 0; i < Argc ; ++i) {
		cout << CommandLine[i] << ' ';
	}
	cout << endl;
	return 0;
}