#pragma once
#ifndef __CLUSTER_H
#define __CLUSTER_H
#include "Stock.h"
#include <list>
#include <unordered_map>
class CCluster {
public:
	CCluster() : m_CentSize(0), m_StockList(), m_NumStocks(0), m_SumDist(0), m_IsValid(true) {}
	CCluster( int nf) : m_StockList(), m_NumStocks(0), m_SumDist(0), m_IsValid(true) { SetCentSize( static_cast<size_t>(nf)); }
	~CCluster() {}
	void RecalcCentroid();
	void SetCentSize(size_t size) { m_CentSize = static_cast<int>(size); ZeroCentroid(); }
	void ZeroCentroid() { if (!m_CentSize) return; m_Centroid = Eigen::VectorXd::Zero(m_CentSize); m_VecCentroid.clear(); m_VecCentroid.resize(m_CentSize, 0.0); }
	bool AddStock(CStock *S, double dist = 0.);
	void ClearStocks() { m_NumStocks = 0; m_SumDist = 0.; if (m_StockList.size()) m_StockList.clear(); if (m_SymbList.size()) m_SymbList.clear(); }
	int GetNumStocks() const { return m_NumStocks;  }
	double GetAvgDist2() { return m_NumStocks ? m_SumDist / m_NumStocks : 0.; }
	double CalcDist2(CStock *S);
	std::list<CStock*> m_StockList;
	std::unordered_map<std::string, double> m_SymbList;
	Eigen::VectorXd m_Centroid;
	std::vector<double> m_VecCentroid;
	int m_CentSize;
	int m_NumStocks;
	double m_SumDist;
	bool m_IsValid;
};

#endif