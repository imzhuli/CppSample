#pragma once
#include <xel/Common.hpp>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>

X_NS
{
	/***
	 * @brief Parse Command Line to Options and Params
	 * @desc:
	 *     Each option has a KeyName, which is used to obtain value;
	 *     Each option has a ShortName, a LongName, or both as command line input hint;
	 *     Each option can require a value;
	 *     After the first apparence of "--" if it's not considered a value (@see Parsing priority):
	 *       all the following arguments will be considered subcommand arguments,
	 *       and will not be converted to option values or arguments;
	 *     If an option is marked as requiring a value and is the last argv in command line:
	 *        it can have an empty string value if its value field is omitted;
	 *     ShortName options begins with a single minus sign, and can be combined together.
	 *     If any of a combination of ShortNames options need values, the following segment will be considered value;
	 *     Parsing priority:
	 *        Obtaining value for option previously checked,
	 *        non-option arguments.
	 *        Checking subcommmand mark ("--"),
	 *        LongName options,
	 *        Combination of ShortName options,
	 *     As mentioned above, though ShortName of '-' and LongName beginning with '-' are not forbidden,
	 *        one should be very carefull, if he insists on using such options.
	 *     If more than one options with same KeyName have values,
	 *        the later one will overwrite the prior one;
	 *     Note!!!
	 *        If a non-subcommand argv starts with '-' or '--', and its KeyName is not found in option list,
	 *        it will simply be ignored. please keep this in mind.
	 * */

	class xCommandLine final
	: xNonCopyable
	{
	public:
		/* SubCommandSeperator = "--" */

		struct xOption {
			char ShortName;
			const char * LongName;
			const char * KeyName;
			bool NeedValue;
		};
		using xOptionValue = xOptional<std::string>;

		X_INLINE xCommandLine() = default;
		X_INLINE ~xCommandLine() = default;
		X_API_MEMBER xCommandLine(const std::vector<std::string> & Arguments, const std::vector<xOption> & OptionList = {});
		X_API_MEMBER xCommandLine(size_t argc, const char ** argv, const std::vector<xOption> & OptionList = {});
		X_INLINE xCommandLine(size_t argc, char ** argv, const std::vector<xOption> & OptionList = {})
		: xCommandLine(argc, const_cast<const char **>(argv), OptionList)
		{}

		X_API_MEMBER void AddOption(const xOption &Option);
		X_API_MEMBER void Parse(int Argc, const char * Argv[]);

		X_API_MEMBER xOptionValue GetOptionValue(const std::string & Key) const;
		X_INLINE     xOptionValue operator[](const std::string & Key) const { return GetOptionValue(Key); }

		X_INLINE const std::vector<std::string> &
			GetArgs() const { return _NonOptionArguments; }
		X_INLINE size_t
			GetArgCount() const { return _NonOptionArguments.size(); }
		X_INLINE const std::string &
			operator[](const size_t Index) const { return _NonOptionArguments[Index]; }

		X_INLINE const std::vector<std::string> &
			GetSubArgs() const { return _SubCommandArguments; }
		X_INLINE size_t
			GetSubArgCount() const { return _SubCommandArguments.size(); }

		X_API_MEMBER void CleanOptions();
		X_API_MEMBER void CleanValues();
		X_API_MEMBER std::string DescribeOptions();

	private:
		struct xCoreOption {
			std::string KeyName;
			bool NeedValue;
		};

		std::unordered_set<std::string> _KeySet;
		std::unordered_map<char, xCoreOption> _ShortOptions;
		std::unordered_map<std::string, xCoreOption> _LongOptions;
		std::unordered_map<std::string, std::string> _ParsedValues;
		std::vector<std::string> _NonOptionArguments;
		std::vector<std::string> _SubCommandArguments;
	};

}
