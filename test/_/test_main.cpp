#include <iostream>
#include <zec/Common.hpp>
#include <zec/Byte.hpp>
#include <zec/String.hpp>

using namespace std;
using namespace zec;

static bool Exit = false;
static const unsigned char SC = '\0';

struct A
{
	operator const unsigned char & () const {
		cout << "TypeCastCalled" << endl;
		return SC;
	}
	operator const void * () const {
		cout << "OperatorVoid*Called" << endl;
		return nullptr;
	}
	const void * operator & () const {
		cout << "Operator&Called" << endl;
		return nullptr;
	}
};
struct B
{
	int a;
	int b;
	int c;
};

#define Assert(cond) if (!(cond)) { cerr << "assertion:( " #cond " ) failed" << endl; exit(-1); }

void TestAddressOf()
{
	A a;
    auto Address = AddressOf(a);
    Assert(Address != &a);
    Assert(Address != (const void*)a);
	Assert(Address != &static_cast<const unsigned char &>(a));
	Assert(Address == &reinterpret_cast<const unsigned char &>(a));
	Assert((ZEC_AddressOf)(a) == Address);


	B b;
	auto Address_B = AddressOf(b);
	Assert(Address_B == ZEC_Entry(ZEC_AddressOf(b.c), B, c));
}



int main(int, char **)
{
    uint64_t Version = 2'2'0105'111'01UL;

    cout << "Version: " << Version << endl;
    cout << "V32: " << (uint32_t)Version << endl;

    auto Guard = []{
        auto Guard = xScopeGuard([]{
            if (Steal(Exit, true)) {
                Error("Dup exit");
            }
            cout << "!Guard!!!" << endl;
        });
        cout << "InScope" << endl;
        return Guard;
    }();

    auto Guard1 = std::move(Guard);
    auto Guard2 = std::move(Guard);

    ubyte Buffer[1024];
    auto W = xStreamWriter(Buffer);
    auto R = xStreamReader(Buffer);

    (void)W;
    (void)R;

    TestAddressOf();
    return 0;
}