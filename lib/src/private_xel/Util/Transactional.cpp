#include <xel/Util/Transactional.hpp>

X_NS
{

	bool xTransactional::Execute()
	{
		Pure();
		return false;
	}

	void xTransactional::Undo()
	{
		Pass();
	}

}
