#include <xel/Common.hpp>
#include <iostream>

using namespace xel;
using namespace std;

static struct xRefed
{
	bool Copied = false;
	xRefed() = default;
	xRefed(const xRefed &) : Copied(true) {}

	void operator ()() const {
		if (Copied) {
			cout << "This is a copied call" << endl;
		} else {
			cout << "This is a refed call" << endl;
		}
	}
} Refed;

static bool OptionalMoved = false;
struct xTestOptionalMove
{
	xTestOptionalMove() = default;
	xTestOptionalMove(const xTestOptionalMove & ) = delete;
	xTestOptionalMove(xTestOptionalMove &&) {
		OptionalMoved = true;
	}
};

static xOptional<xTestOptionalMove> returnMove(xTestOptionalMove & MovableTarget)
{
	return std::move(MovableTarget);
}

int main(int, char *[])
{
	auto guard1 = xScopeGuard(Refed);
	auto guard2 = xScopeGuard(xRef(Refed));

	xTestOptionalMove TryMoveMe;
	returnMove(TryMoveMe);
	if (!OptionalMoved) {
		Error();
	}

	int i = 0;
	int &j = i;
	do {
		auto Guard = xScopeGuard([j]() mutable {
			cout << (j = 100) << endl;
		});
 	} while(false);
	cout << "I=" << i << endl;
	if (i) {
		return -1;
	}

	cout << "befored guarded ref call" << endl;
	return 0;
}