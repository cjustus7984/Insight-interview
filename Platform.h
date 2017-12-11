#pragma once
#ifndef __PLATFORM_H
#define __PLATFORM_H
#include "Stock.h"
#include <unordered_map>
#define REMOVE_BETA
#define LOOP_LIST( List, Iterator ) for ( auto Iterator = List.begin(); Iterator != List.end(); ++Iterator )
#define LOOP_LIST_REVERSE( List, Iterator ) for ( auto Iterator = List.rbegin(); Iterator != List.rend(); ++Iterator )

class CPlatform {
public:
	CPlatform() : m_NumPeriods(0), m_Stocks(), m_StockMap() {}
	~CPlatform() {}
	CStock * FindStockBySymbol(std::string Symb) { auto sit = m_StockMap.find(Symb); return sit == m_StockMap.end() ? 0 : sit->second; }
	bool LoadData( std::string Filename );
	void InitializeStockConsts();
	void Run();
	std::vector<double> m_Market;
	int m_NumPeriods;
	std::vector<CStock*> m_Stocks;
	std::unordered_map<std::string, CStock*> m_StockMap;
};

extern CPlatform Platform;

#endif