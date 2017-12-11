#include "Cluster.h"

bool CCluster::AddStock(CStock *S, double dist) {
	if (!S) { printf("Error <AddStock>: input S does not exist!\n)"); return false; }
	auto sfnd = std::find(m_StockList.begin(), m_StockList.end(), S);
	if (sfnd != m_StockList.end()) {
		printf("Warning <AddStock>: Input stock %s already in stocklist!", S->GetSymbol());
		return true;
	}
	m_SumDist += dist;
	m_StockList.push_back(S);
	m_SymbList[S->GetSymbol()] = dist;
	m_NumStocks++;
	return true;
}

void CCluster::RecalcCentroid() {
	if (!m_Centroid.size()) return;
	if (!m_CentSize) return;
	
	ZeroCentroid();
	int N = 0;
	for (auto s = m_StockList.begin(); s != m_StockList.end(); s++) {
		CStock *S = *s;
		if (S->GetNumFeatures() != m_CentSize) { printf("Error <RecalcCentroid>: CentSize %d is not equal to stock %s NumFeatures %d\n", m_CentSize, S->GetSymbol(), S->GetNumFeatures()); continue; }
		//m_Centroid += S->GetFeatures();
		for (int f = 0; f < S->GetNumFeatures(); f++) {
			m_Centroid(f) += S->GetFeature(f);
			m_VecCentroid[f] += S->GetFeature(f);
		}
		N++;
	}

	m_Centroid /= N;
	for (int f = 0; f < m_CentSize; f++) m_VecCentroid[f] /= N;
}

double CCluster::CalcDist2(CStock *S) {
	if (!S) { printf("Error <CalcDist2>: Input stock does not exist!\n"); return 0.; }
	if (S->GetNumFeatures() != m_CentSize) { printf("Error <CalcDist2>: CentroidSize %d is not equal to input stock %s feature size %d\n", m_CentSize, S->GetSymbol(), S->GetNumFeatures()); return 0.; }
	return S->CalcFeatureDist2(m_Centroid);
}

