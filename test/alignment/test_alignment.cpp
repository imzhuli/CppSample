#include <iostream>
#include <zec/Common.hpp>
#include <zec/String.hpp>
#include <zec/Byte.hpp>
#include <sstream>
#include <stdexcept>
#include <cmath>

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

}

void testOrder()
{
	ubyte B1[8] = {};
	ubyte B2[8] = {};

	xStreamWriter W1(B1);
	xStreamReader R1(B1);
	xStreamWriter W2(B2);
	xStreamReader R2(B2);

	int16_t I2 = 0;
	int32_t I4 = 0;
	int64_t I8 = 0;

	W1.W8(0x0102030405060708);
	I8 = R1.R8L();
	if (I8 != 0x0807060504030201) {
		Error();
	}

	W2.W(B1, 8);
	I8 = R2.R8();
	if (I8 != 0x0102030405060708) {
		Error();
	}
	R2.Reset();

	I2 = R2.R2L();
	I4 = R2.R4L();
	if (I2 != 0x0201 || I4 != 0x06050403) {
		Error();
	}
}

int main(int, char **)
{
	try {
		test();
		testOrder();
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
