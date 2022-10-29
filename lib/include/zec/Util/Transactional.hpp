#pragma once
#include <zec/Common.hpp>

X_NS
{

	class xTransactional
	: xNonCopyable
	{
	public:
		X_API_MEMBER virtual bool Execute();
		X_API_MEMBER virtual void Undo();

		X_STATIC_INLINE bool BatchExecute(xTransactional * TransactionList, size_t Count) {
			size_t i = 0;
			while (i < Count) {
				if (!TransactionList[i].Execute()) {
					break;
				}
				++i;
			}
			if (i == Count) {
				return true;
			}
			while(i) {
				TransactionList[--i].Undo();
			}
			return false;
		}

		template<typename tTransactional>
		X_STATIC_INLINE bool BatchExecute(tTransactional && TransactionalRef) {
			return std::forward<tTransactional>(TransactionalRef).Execute();
		}

		template<typename tTransactional, typename ... tTransactionalRefs>
		X_STATIC_INLINE bool BatchExecute(tTransactional && TransactionalRef, tTransactionalRefs && ... TransactionRefs) {
			if (!std::forward<tTransactional>(TransactionalRef).Execute()) {
				return false;
			}
			if (!BatchExecute(std::forward<tTransactionalRefs>(TransactionRefs)...)) {
				std::forward<tTransactional>(TransactionalRef).Undo();
				return false;
			}
			return true;
		}

	};

}
