#include <iostream>
#include <xel/Common.hpp>
#include <xel/String.hpp>
#include <xel/Byte.hpp>
#include <sstream>
#include <stdexcept>
#include <cmath>

using namespace std;
using namespace xel;

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
