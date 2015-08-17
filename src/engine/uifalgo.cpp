//
// Uif algo implementation file
// Author:yeyouqun@163.com
// 2010-4-25
//
#include <gmpre.h>
#include <engine/defs.h>
#include <engine/uifblock.h>
#include <engine/util.h>
#include <engine/uifalgo.h>

void ClearUifSourceTree (GmUifSourcePairT * pSourceTree)
{
	if (pSourceTree == 0) return;

	if (pSourceTree->first != 0) delete pSourceTree->first;
	if (pSourceTree->second != 0) delete pSourceTree->second;

	pSourceTree->first = 0;
	pSourceTree->second = 0;
}

void ClearUifTree (GmUifSourceVectorT * pUifTree)
{
	if (pUifTree == 0) return;

	for (size_t index = 0; index < pUifTree->size (); ++index) {
		ClearUifSourceTree ((*pUifTree)[index]);
		delete (*pUifTree)[index];
	}

	pUifTree->clear ();
}

void ClearRootTree (GmUifRootPairT * pTree)
{
	if (pTree == 0) return;
	
	if (pTree->first != 0) delete pTree->first;

	if (pTree->second != 0) {
		ClearUifTree (pTree->second);
		delete pTree->second;
	}

	pTree->first = 0;
	pTree->second = 0;
}
