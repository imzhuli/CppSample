#include <zec/Util/Command.hpp>
#include <cstring>

ZEC_NS
{

	xCommandLine::xCommandLine(int Argc, const char ** Argv, const std::vector<xOption> & OptionList)
	{
		for (const auto & Option : OptionList) {
			AddOption(Option);
		}
		Parse(Argc, Argv);
	}

	xCommandLine::xOptionValue xCommandLine::GetOptionValue(const std::string & Key) const
	{
		auto Iter = _ParsedValues.find(Key);
		if (Iter == _ParsedValues.end()) {
			return {};
		}
		return Iter->second;
	}

	void xCommandLine::AddOption(const xOption &Option)
	{
		assert(Option.KeyName);
		assert(Option.ShortName || Option.LongName);
		xCoreOption CoreOption = { Option.KeyName, Option.NeedValue	};
		if (_KeySet.find(Option.KeyName) != _KeySet.end()) {
			Error("Duplicate OptionKey");
		}
		_KeySet.insert(Option.KeyName);
		if (Option.ShortName) {
			_ShortOptions.insert_or_assign(Option.ShortName, CoreOption);
		}
		if (Option.LongName) {
			_LongOptions.insert_or_assign(Option.LongName, CoreOption);
		}
	}

	void xCommandLine::Parse(int Argc, const char * Argv[])
	{
		std::vector<std::string> Keys;
		int i = 0;
		while(i < Argc) {
			const char * Param = Argv[i++];
			if (Keys.size()) {
				for(const auto & Key : Keys) {
					_ParsedValues.insert_or_assign(Key, Param);
				}
				Keys.clear();
				continue;
			}
			else if ('-' != Param[0]) {
				_NonOptionArguments.push_back(Param);
				continue;
			}
			else if ('-' == Param[1]) {
				if ('\0' == Param[2]) {
					// subcommand, end option parsing
					break;
				}
				// process long values:
				const char * OptionName = Param + 2;
				auto KeyNameIter = _LongOptions.find(OptionName);
				if (KeyNameIter != _LongOptions.end()) {
					const auto & Option = KeyNameIter->second;
					if (Option.NeedValue) {
						Keys.push_back(Option.KeyName);
					} else {
						_ParsedValues.insert_or_assign(Option.KeyName, std::string{});
					}
				}
				continue;
			}
			else { // process short values
				for (++Param; *Param != '\0'; ++Param) {
					char OptionName = *Param;
					auto KeyNameIter = _ShortOptions.find(OptionName);
					if (KeyNameIter != _ShortOptions.end()) {
						const auto & Option = KeyNameIter->second;
						if (Option.NeedValue) {
							Keys.push_back(Option.KeyName);
						} else {
							_ParsedValues.insert_or_assign(Option.KeyName, std::string{});
						}
					}
				}
				continue;
			}
		}
		if (Keys.size()) { // if last value field is omitted, add empty value
			assert(i == Argc);
			for (const auto & Key : Keys) {
				_ParsedValues.insert_or_assign(Key, std::string{});
			}
		}
		else {
			while(i < Argc) {
				_NonOptionArguments.push_back(Argv[i++]);
			}
		}
	}

	void xCommandLine::CleanOptions()
	{
		_ShortOptions.clear();
		_LongOptions.clear();
		_KeySet.clear();
	}

	void xCommandLine::CleanValues()
	{
		_ParsedValues.clear();
		_NonOptionArguments.clear();
	}

}
