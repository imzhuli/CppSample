#include <zec/Common.hpp>
#include <iostream>

using namespace zec;
using namespace std;

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
	xTestOptionalMove TryMoveMe;
	returnMove(TryMoveMe);
	if (!OptionalMoved) {
		Error();
	}
	return 0;
}