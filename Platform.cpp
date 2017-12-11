#include "Platform.h"
#include "Model.h"
bool CPlatform::LoadData(std::string Filename) {
	FILE * pFile = _fsopen(Filename.c_str(), "rb", _SH_DENYNO);
	if (!pFile) {
		printf("Error <LoadData>: Filename %s could not be opened!\n", Filename.c_str());
		return false;
	}
	int NumStocks = 0;
	size_t floatsize = sizeof(float);
	fread(&NumStocks, sizeof(int), 1, pFile);
	fread(&m_NumPeriods, sizeof(int), 1, pFile);
	m_Market.resize(m_NumPeriods, 0.0);
	int Nduplicates = 0;
	m_Stocks.resize(NumStocks, 0);
	for (int s = 0; s < NumStocks; s++) {
		char Symb[128] = {};
		fread(Symb, 128, 1, pFile);
		CStock *S = FindStockBySymbol(Symb);
		if (!S) {
			S = new CStock(Symb);
			S->SetDataSize(m_NumPeriods);
			m_StockMap[Symb] = S;
			m_Stocks[s] = S;
		}
		else Nduplicates++;
		for (int p = 0; p < m_NumPeriods; p++) {
			float Rtn = 0.;
			fread(&Rtn, sizeof(float), 1, pFile);
			S->InsertReturn(p, static_cast<double>(Rtn));
			m_Market[p] += Rtn;
		}
	}

	printf("LoadData: %d stocks loaded with %d return periods.\n", m_Stocks.size(), m_NumPeriods);
	if (Nduplicates) printf("Warning <LoadData>: %d duplicate stocks found in file!\n", Nduplicates);
	for (int t = m_Market.size() - 1; t >= 0; t--) { m_Market[t] /= NumStocks; }
	return true;
}

void CPlatform::InitializeStockConsts() {
	LOOP_LIST(m_Stocks, s) {
		CStock *S = *s;
		S->CalcStockConsts();
	}
}


void CPlatform::Run() {
	std::string Filename = "C:/Dev2017/File.bin";
	if (!LoadData(Filename)) { printf("Error <Run>: Can not load data from file %s. Aborting...", Filename.c_str()); exit(1); }
	InitializeStockConsts();
	int ExptNumClusters = m_Stocks.size() / 50;
	if (ExptNumClusters < 2) ExptNumClusters = 2;
	printf("Running clustering model for %d max clusters\n", ExptNumClusters);
	Mdl.RunFullClustering( ExptNumClusters );

}



CPlatform Platform;