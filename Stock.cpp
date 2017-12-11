#include "Stock.h"
#include "MathScrips.h"
#include "Platform.h"
void CStock::CalcStockConsts() {
	if (!GetDataSize()) { printf("Error <CalcStockConsts>: Cannot calculate stock consts with no data!\n"); return; }
	m_Beta = Regress(m_Returns.data(), Platform.m_Market.data(), 0, GetDataSize() - 1);
#ifdef REMOVE_BETA
	for (int r = GetDataSize() - 1; r >= 0; r--) {
		m_Returns[r] -= m_Beta * Platform.m_Market[r];
	}
#endif
	m_DevRtn = CalcStd(m_Returns.data(), 0, GetDataSize() - 1, &m_AvgRtn);
	
}

double CStock::CalcFeatureDist2(CStock *S) {
	if (!S) { printf("Error <CalcFeatureDist2>: Input stock does not exist!\n"); return 0.; }
	if (S->m_Features.size() != m_Features.size()) {
		printf("Error <CalcFeatureDist2>: Feature size %d is not equal to input feature size %d\n", m_Features.size(), S->m_Features.size());
		return 0.0;
	}
	Eigen::VectorXd DiffV = m_Features - S->m_Features;
	return DiffV.dot(DiffV);
}

double CStock::CalcFeatureDist2(Eigen::VectorXd &V1) {
	if (V1.size() != m_Features.size()) {
		printf("Error <CalcFeatureDist2>: FeatureVector size %d not equal to input vector size %d\n", m_Features.size(), V1.size());
		return 0.0;
	}
	Eigen::VectorXd DiffV = m_Features - V1;
	double Sum = 0.;
	for (int i = 0; i < m_Features.size(); i++) {
		double d = Sqr(m_Features[i] - V1(i));
		Sum += d;
	}
	double dist_Eigen = DiffV.dot(DiffV);
	return DiffV.dot(DiffV);
}