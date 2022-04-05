#pragma once
#include <zec/Common.hpp>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <string>
#include <string_view>

ZEC_NS
{

	ZEC_STATIC_INLINE std::string JsonString(const rapidjson::Document & JDoc) {
		rapidjson::StringBuffer JBuffer;
		rapidjson::Writer<rapidjson::StringBuffer> JWriter(JBuffer);
		JDoc.Accept(JWriter);
		return {JBuffer.GetString(), JBuffer.GetSize()};
	}

	ZEC_STATIC_INLINE xOptional<int64_t> GetInt(const rapidjson::Value & JsonValue, const char * Key)
	{
		auto NotFound = JsonValue.MemberEnd();
		auto IterAccountId = JsonValue.FindMember(Key);
		if (IterAccountId == NotFound || !IterAccountId->value.IsInt64()) {
			return {};
		}
		return { IterAccountId->value.GetInt64() };
	}

	ZEC_STATIC_INLINE xOptional<bool> GetBool(const rapidjson::Value & JsonValue, const char * Key)
	{
		auto NotFound = JsonValue.MemberEnd();
		auto IterAccountId = JsonValue.FindMember(Key);
		if (IterAccountId == NotFound || !IterAccountId->value.IsBool()) {
			return {};
		}
		return { IterAccountId->value.GetBool() };
	}

	ZEC_STATIC_INLINE xOptional<double> GetDouble(const rapidjson::Value & JsonValue, const char * Key)
	{
		auto NotFound = JsonValue.MemberEnd();
		auto IterAccountId = JsonValue.FindMember(Key);
		if (IterAccountId == NotFound || !IterAccountId->value.IsDouble()) {
			return {};
		}
		return { IterAccountId->value.GetDouble() };
	}

	ZEC_STATIC_INLINE xOptional<std::string_view> GetStringView(const rapidjson::Value & JsonValue, const char * Key)
	{
		auto NotFound = JsonValue.MemberEnd();
		auto IterAccountId = JsonValue.FindMember(Key);
		if (IterAccountId == NotFound || !IterAccountId->value.IsString()) {
			return {};
		}
		return std::string_view{ IterAccountId->value.GetString(), IterAccountId->value.GetStringLength() };
	}

	ZEC_STATIC_INLINE xOptional<std::string> GetString(const rapidjson::Value & JsonValue, const char * Key)
	{
		auto OptView = GetStringView(JsonValue, Key);
		return OptView() ? xOptional<std::string>{ *OptView } : xOptional<std::string>{};
	}

	ZEC_STATIC_INLINE xOptional<xRef<const rapidjson::Value>> GetObject(const rapidjson::Value & JsonValue, const char * Key)
	{
		auto NotFound = JsonValue.MemberEnd();
		auto IterAccountId = JsonValue.FindMember(Key);
		if (IterAccountId == NotFound || !IterAccountId->value.IsObject()) {
			return {};
		}
		return xRef{IterAccountId->value} ;
	}

	ZEC_STATIC_INLINE xOptional<rapidjson::Value::ConstArray> GetArray(const rapidjson::Value & JsonValue, const char * Key)
	{
		auto NotFound = JsonValue.MemberEnd();
		auto IterAccountId = JsonValue.FindMember(Key);
		if (IterAccountId == NotFound || !IterAccountId->value.IsArray()) {
			return {};
		}
		return IterAccountId->value.GetArray();
	}

}
