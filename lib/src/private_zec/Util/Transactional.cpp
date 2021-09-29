#include <zec/Util/Transactional.hpp>

ZEC_NS
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
