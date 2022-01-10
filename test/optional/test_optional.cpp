#include <zec/Common.hpp>
#include <iostream>

using namespace zec;
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

	cout << "befored guarded ref call" << endl;
	return 0;
}