#pragma once

#include <Windows.h>
#include <chrono>
#include <iterator>
#include <algorithm>
#include <map>
#include <vector>
#include <string>
//static inline const std::vector<uint8_t> &xs64_extp_key{ 11,190,135,131,55,23,111,102,87,234,154,208,43,44,44,58,81,104,209,65,157,38,8,96,172,0,114,164,28,142,187,208,18,214,46,100,174,2,98,63,239,73,196,220,214,214,25,173,100,184,228,163,190,139,37,246,244,64,165,127,2,196,61,90,4,190,54,56,8,216,54,146,97,252,60,110,78,103,180,143,161,43,38,168,229,194,251,26,122,223,117,220,132,206,36,107,68,226,19,85,44,231,238,162,11,45,6,58,173,15,84,233,124,24,202,142,132,65,24,91,114,203,55,211,92,235,196,252,118,73,220,198,222,234,58,122,54,85,71,214,47,42,32,146,49,110,156,118,106,251,5,170,255,246,65,53,88,160,102,139,135,31,228,100,26,119,238,41,132,111,137,75,147,47,96,10,81,149,33,158,109,189,33,237,118,158,57,129,55,230,35,62,159,201,208,230,165,236,223,102,160,29,226,26,250,221,136,202,38,106,180,85,21,50,245,92,184,89,217,175,242,148,222,60,114,68,134,17,49,13,21,158,26,242,165,136,208,13,206,252,191,112,101,24,215,244,43,19,220,242,92,51,87,0,155,213,146,100,157,159,94,244,108,172,153,126,150,108,176,234,172,208,167,191,88,84,108,21,200,135,218,117,210,176,86,207,36,40,62,177,7,163,176,172,247,11,171,12,156,85,34,176,191,25,50,142,45,190,110,90,166,99,53,180,90,131,81,190,182,17,204,169,248,139,155,193,156,54,182,214,114,16,71,95,26,203,134,47,157,49,172,130,123,61,196,80,248,22,136,142,85,44,63,126,81,190,252,84,170,144,111,156,186,200,175,33,184,7,254,17,133,120,45,25,122,237,80,175,84,2,222,16,149,165,6,100,100,105,5,83,214,96,140,138,180,181,173,145,47,207,164,11,122,229,237,145,98,163,189,248,103,169,244,101,87,221,81,49,237,51,191,157,138,89,167,172,51,149,173,44,158,245,76,86,137,149,129,214,190,31,54,12,151,250,97,149,15,19,107,255,207,230,70,69,58,201,135,59,50,211,37,1,123,120,99,234,216,27,31,235,51,167,142,146,183,142,156,2,127,50,221,138,64,146,28,54,222,54,205,162,7,0,125,150,173,214,93,7,86,213,158,27,114,7,17,123,188,108,159,26,151,16,125,93,69,110,58,190,10,204,213,96,64,68,38,249,7,156,192,114,218,173,3,239,157,109,212,253,148,168,177,38,249,205,220,106,211,123,92,20,181,204,36,156,71,132,254,74,174,209,82,130,172,249,226,163,82,224,48,5,68,216,242,3,182,228,72,145,188,236,75,194,16,63,141,197,93,223,192,148,116,206,152,165,147,37,238,73,252,216,134,210,176,221,37,4,60,178,136,188,58,238,155,95,3,127,171,203,167,133,187,29,242,0,216,86,114,76,243,136,25,125,37,209,227,118,91,249,158,97,208,71,49,219,145,254,73,213,98,201,70,35,196,225,218,17,152,54,54,58,83,214,217,119,120,176,221,199,200,234,231,186,93,177,8,185,207,73,47,175,36,78,252,116,251,81,144,200,250,198,110,205,249,226,162,43,161,227,92,212,249,75,104,132,177,232,113,16,55,157,66,198,222,87,177,121,199,185,139,142,12,31,167,234,73,116,230,16,160,206,160,139,177,190,42,193,118,88,153,117,250,198,225,200,108,149,19,82,44,251,120,165,78,139,96,193,145,181,123,130,8,186,246,247,253,104,202,232,187,43,46,31,107,217,32,91,118,232,236,94,206,115,12,57,253,219,141,207,74,254,182,107,51,218,119,29,248,143,71,201,192,122,87,37,17,62,66,17,31,60,26,232,106,34,124,171,237,222,94,220,105,145,226,209,27,158,141,184,207,140,229,205,153,196,15,177,103,151,178,108,87,96,14,234,98,97,37,186,161,197,181,100,79,199,121,23,4,123,216,109,39,182,31,8,75,86,226,112,218,95,148,100,183,216,6,227,59,35,198,230,245,216,182,14,224,160,2,136,86,104,34,217,146,191,83,57,54,240,172,48,41,3,51,47,104,38,114,136,187,0,182,137,59,140,140,215,36,2,112,12,161,137,86,194,88,16,188,242,145,78,27,109,223,191,214,121,2,2,2,42,23,204,235,63,245,93,97,228,238,146,105,26,102,179,187,90,66,8,98,202,13,170,212,88,255,14,218,248,115,141,106,139,11,194,18,101,10,145,123,17,240,64,131,211,90,48,42,97,13,20,143,224,148,49,67,14,2,88,17,188,169,32,188,140,214,154,232,203,110,105,215,177,114,174,41,247,166,193,33,21,22,17,18,19 };
static uint64_t XS64_SEED = 1;
static inline const std::vector<uint8_t>& xs64_extp_key2{ 13, 12 };
static inline const std::vector<uint8_t>& xs64_extp_key{ 61, 89, 11, 43 };

static __forceinline uint64_t xs64_from_seed(uint64_t seed)
{
	seed ^= seed << 13;
	seed ^= seed >> 7;
	seed ^= seed << 17;
	return seed;
}

static __forceinline void xs64_reset_seed()
{
	XS64_SEED = xs64_from_seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
}

static __forceinline uint64_t xs64()
{
	return XS64_SEED = xs64_from_seed(XS64_SEED);
}

class bit_vector
{
protected:
	std::vector<uint8_t>& data;
public:
	__forceinline bit_vector(std::vector<uint8_t>& data) : data(data)
	{
	}
	__forceinline uint8_t operator[](const size_t pos) const
	{
		return (data[pos / 8] & (1 << (pos % 8)));
	}
	__forceinline uint8_t set(const size_t pos, const bool val)
	{
		const size_t pos_b = pos / 8;
		return (data[pos_b] = (data[pos_b] ^ (*this)[pos]) | ((val ? 1 : 0) << (pos % 8)));
	}
	__forceinline size_t size() const
	{
		return data.size() * 8;
	}
};
namespace bXor {
	//extern std::vector<uint8_t> g_Key;
	static __forceinline std::vector<uint8_t> encrypt(std::vector<uint8_t> data, const std::vector<uint8_t>& key) {
		const uint64_t r_u64 = xs64();
		const uint8_t* r_u8 = reinterpret_cast<const uint8_t*>(&r_u64);
		const size_t key_size = key.size();
		size_t data_size = data.size();
		for (size_t i = 0; i < data_size; ++i)
			data[i] ^= r_u8[i % sizeof(uint64_t)];
		data.insert(data.end(), r_u8, r_u8 + sizeof(uint64_t));
		data_size = data.size();
		const size_t seed_size = (key_size < 8) ? key_size : 8;
		uint64_t seed = 0;
		for (size_t i = 0; i < seed_size; ++i)
			seed |= (key[i] << (8 * i));
		bit_vector data_b(data);
		const size_t data_b_size = data_b.size();
		for (size_t i = 0; i < data_b_size; ++i) {
			seed = xs64_from_seed(seed);
			const size_t j = (seed % data_b_size);
			if (i != j) {
				uint8_t a = data_b[i], b = data_b[j], c;
				c = a;
				data_b.set(i, b > 0);
				data_b.set(j, c > 0);
			}
		}
		for (size_t i = 0; i < data_size; ++i)
			data[i] ^= key[i % key_size];
		return data;
	}

	static __forceinline std::vector<uint8_t> decrypt(std::vector<uint8_t> data, const std::vector<uint8_t>& key) {
		const size_t key_size = key.size();
		size_t data_size = data.size();
		for (size_t i = 0; i < data_size; ++i)
			data[i] ^= key[i % key_size];
		const size_t seed_size = (key_size < 8) ? key_size : 8;
		uint64_t seed = 0;
		for (size_t i = 0; i < seed_size; ++i)
			seed |= (key[i] << (8 * i));
		bit_vector data_b(data);
		const size_t data_b_size = data_b.size();
		std::vector<size_t> jv;
		jv.resize(data_b_size);
		for (size_t i = 0; i < data_b_size; ++i) {
			seed = xs64_from_seed(seed);
			const size_t j = (seed % data_b_size);
			jv[i] = j;
		}
		size_t i = data_b_size;
		while (i > 0) {
			--i;
			const size_t j = jv[i];
			if (i != j)

			{
				uint8_t a = data_b[i], b = data_b[j], c;
				c = a;
				data_b.set(i, b > 0);
				data_b.set(j, c > 0);
			}
		}
		const uint8_t* r_u8 = &data[data_size - sizeof(uint64_t)];
		data_size -= sizeof(uint64_t);
		for (size_t i = 0; i < data_size; ++i)
			data[i] ^= r_u8[i % sizeof(uint64_t)];
		data.resize(data_size);
		return data;
	}
}

#define crypt_key 0xA5

template<std::size_t strLen>
class _hiddenString
{
protected:
	static __forceinline constexpr std::uint64_t hash(std::uint64_t x, std::uint64_t sol)
	{
		x ^= 948274649985346773LLU ^ sol;
		x ^= x << 13;
		x ^= x >> 7;
		x ^= x << 17;
		return x;
	}
	mutable bool m_isDecrypted;
	mutable char m_str[strLen];
	std::uint64_t m_hashingSol;
public:
	__forceinline constexpr _hiddenString(const char(&str)[strLen], std::uint64_t hashingSol) : m_isDecrypted(false), m_str{ 0 }, m_hashingSol(hashingSol)
	{
		for (std::size_t i = 0; i < strLen; ++i)
			this->m_str[i] = str[i] ^ _hiddenString<strLen>::hash(i, this->m_hashingSol);
	}
	__forceinline constexpr operator std::string() const
	{
		if (!this->m_isDecrypted)
		{
			this->m_isDecrypted = true;
			for (std::size_t i = 0; i < strLen; ++i)
				this->m_str[i] ^= _hiddenString<strLen>::hash(i, this->m_hashingSol);
		}
		return { this->m_str, this->m_str + strLen - 1 };
	}
};

template<std::size_t strLen>
class _hiddenWString
{
protected:
	static __forceinline constexpr std::uint64_t hash(std::uint64_t x, std::uint64_t sol)
	{
		x ^= 948274649985346773LLU ^ sol;
		x ^= x << 13;
		x ^= x >> 7;
		x ^= x << 17;
		return x;
	}
	mutable bool m_isDecrypted;
	mutable wchar_t m_str[strLen];
	std::uint64_t m_hashingSol;
public:
	__forceinline constexpr _hiddenWString(const wchar_t(&str)[strLen], std::uint64_t hashingSol) : m_isDecrypted(false), m_str{ 0 }, m_hashingSol(hashingSol)
	{
		for (std::size_t i = 0; i < strLen; ++i)
			this->m_str[i] = str[i] ^ _hiddenWString<strLen>::hash(i, this->m_hashingSol);
	}
	__forceinline constexpr operator std::wstring() const
	{
		if (!this->m_isDecrypted)
		{
			this->m_isDecrypted = true;
			for (std::size_t i = 0; i < strLen; ++i)
				this->m_str[i] ^= _hiddenWString<strLen>::hash(i, this->m_hashingSol);
		}
		return { this->m_str, this->m_str + strLen - 1 };
	}
};

#define crypt_str(s) ([]() -> std::string \
{ \
	static constexpr _hiddenString hiddenStr { s, __COUNTER__ }; \
	return hiddenStr; \
})().c_str()

#define crypt_strw(ws) ([]() -> std::wstring \
{ \
	static constexpr _hiddenWString hiddenStr { ws, __COUNTER__ }; \
	return hiddenStr; \
})().c_str()


template<std::size_t SIZE>
class hiddenString
{
public:
	short d[SIZE];
	constexpr hiddenString() :d{ 0 } { }

	__forceinline std::string s() const
	{
		std::string rv;
		rv.reserve(SIZE + 1);
		std::transform(d, d + SIZE - 1, std::back_inserter(rv), [](auto ch) {
			return ch ^ crypt_key;
			});
		int p = rv.find('\0');
		while (p != -1) {
			rv.erase(rv.begin() + p);
			p = rv.find('\0');
		}
		return rv;
	}
};

template<typename T, std::size_t N> constexpr std::size_t sizeCalculate(const T(&)[N])
{
	return N;
}

template<std::size_t SIZE>
constexpr auto encoder(const char str[SIZE])
{
	hiddenString<SIZE> encoded;
	for (std::size_t i = 0; i < SIZE/* - 1*/; i++)
		encoded.d[i] = str[i] ^ crypt_key;
	//encoded.d[SIZE - 1] = 0;
	return encoded;
}

#define _(name, x) static constexpr auto name = encoder<sizeCalculate(x)>(x)

class encrypted_string {
private:
	std::string base;
public:
	operator std::string() const {
		auto dec = bXor::decrypt(std::vector<uint8_t>(base.begin(), base.end()), xs64_extp_key);
		return std::string(dec.begin(), dec.end());
		//return base.c_str();
	}
	std::string operator()() const {
		return *this;
	}
	template<size_t size>
	void operator()(char(&str)[size]) {
		base = std::string(str, str + size);
	}
	template<size_t size>
	encrypted_string& operator=(char(&str)[size])
	{
		auto s = std::string(str);
		auto enc = bXor::encrypt(std::vector<uint8_t>(s.begin(), s.end()), xs64_extp_key);
		base = std::string(enc.begin(), enc.end());
		return *this;
	}
	encrypted_string& operator=(const char* str)
	{
		/*	base = str;
			return *this;*/
		auto s = std::string(str, str + strlen(str));
		auto enc = bXor::encrypt(std::vector<uint8_t>(s.begin(), s.end()), xs64_extp_key);
		base = std::string(enc.begin(), enc.end());
		return *this;
	}
};

typedef std::map<unsigned int, encrypted_string> crypted_map;
typedef std::map<unsigned int, unsigned int> hook_index;

extern hook_index g_HookIndices;
extern crypted_map g_Patterns;
extern crypted_map g_Modules;
