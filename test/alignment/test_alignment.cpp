#include <iostream>
#include <zec/Common.hpp>
#include <zec/String.hpp>
#include <zec/Byte.hpp>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <smmintrin.h>

using namespace std;
using namespace zec;

static void test()
{
	stringstream err;
	for (int i = 1 ; i <= 16; ++i) {
		auto Al = Align(i, 16);
		if (Al != 16) {
			err << "Invalid Alignment at " << i << ", Value=" << Al;
			throw std::runtime_error(err.str());
		}
	}
	for (int i = 17 ; i <= 32; ++i) {
		auto Al = Align(i, 16);
		if (Al != 32) {
			err << "Invalid Alignment at " << i << ", Value=" << Al;
			throw std::runtime_error(err.str());
		}
	}
	for (int i = 33 ; i <= 48; ++i) {
		auto Al = Align(i, 16);
		if (Al != 48) {
			err << "Invalid Alignment at " << i << ", Value=" << Al;
			throw std::runtime_error(err.str());
		}
	}
	for (int i = 49 ; i <= 64; ++i) {
		auto Al = Align(i, 16);
		if (Al != 64) {
			err << "Invalid Alignment at " << i << ", Value=" << Al;
			throw std::runtime_error(err.str());
		}
	}

	static uint32_t i = 0;
	union U{
		__m128i data;
		struct {
			uint32_t a0;
			uint32_t a1;
			uint32_t a2;
			uint32_t a3;
		} ii = { i, i + 1, i + 2, i + 3 };
	};

	U U1;
	i += 10;
	U U2;

	U1.data = _mm_max_epi32(U1.data, U2.data);
	cout <<  U1.ii.a0 << U1.ii.a1 << U1.ii.a2 << U1.ii.a3 << endl;
}

int main(int, char **)
{
	try {
		test();
	}
	catch (const std::exception & e) {
		cerr << "Exception: " << e.what() << endl;
		return -1;
	}
	catch (const char * s) {
		cerr << "Error: " << s << endl;
		return -1;
	}
	catch (...) {
		return -1;
	}
  	return 0;
}
