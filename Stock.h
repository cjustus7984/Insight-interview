#pragma once
#ifndef __STOCK_H
#define __STOCK_H
#include <vector>
#include <string>
#include <Eigen/SVD>

class CStock {
public:
	CStock() : m_DataSize(0), m_Symbol(""), m_NumFeatures(0), m_AvgRtn(0), m_DevRtn(0), m_Returns() {}
	CStock(std::string symb) : m_DataSize(0), m_Symbol(symb), m_AvgRtn(0), m_DevRtn(0), m_NumFeatures(0), m_Returns() {}
	~CStock() {}
	void SetDataSize(int size)					{ 
		m_DataSize = size; 
		m_Returns.resize(m_DataSize, 0.0);
	}
	int GetDataSize() const { return m_DataSize;  }
	void CalcStockConsts();
	void InsertReturn(int i, double rtn)		{ 
		if (i >= m_DataSize) return; 
		m_Returns[i] = rtn;
	}
	double GetReturn(int i)						{ if (i >= m_DataSize) return 0.0; return m_Returns[i]; }
	std::vector<double> &GetReturns()			{ return m_Returns; }
	Eigen::VectorXd &GetFeatures()				{ return m_Features; }
	void SetSymbol(const char* symb)			{ m_Symbol = symb; }
	void SetSymbol(std::string symb)			{ m_Symbol = symb;  }
	const char* GetSymbol()						{ return m_Symbol.c_str(); }
	double CalcFeatureDist2(Eigen::VectorXd &V1);
	double CalcFeatureDist2(CStock *S1);
	int GetNumFeatures() const { return m_NumFeatures;  }
	void InitializeFeatures(int size) { m_NumFeatures = size; m_Features = Eigen::VectorXd::Zero(size); m_VecFeatures.resize(size, 0.0); }
	double GetFeature(int f) { if (f >= m_Features.size()) return 0.0; return m_Features(f); }
	void   SetFeatures( Eigen::VectorXd &FV ) {
		if (!m_NumFeatures) return;
		if (m_NumFeatures != FV.size()) return;
		for (int f = 0; f < m_NumFeatures; f++) {
			m_Features(f) = FV(f); 
			m_VecFeatures[f] = FV(f);
		}
	}
	double GetAvgReturn() const { return m_AvgRtn;  }
	double GetDevReturn() const { return m_DevRtn;  }
	double GetBeta() const { return m_Beta;  }
private:
	std::vector<double> m_Returns;
	std::string m_Symbol;
	int m_DataSize;
	int m_NumFeatures;
	Eigen::VectorXd m_Features;
	std::vector<double> m_VecFeatures;
	double m_AvgRtn;
	double m_DevRtn;
	double m_Beta;
};

#endif