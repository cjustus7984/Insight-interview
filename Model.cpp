#include <sstream>

#include "Model.h"
#include "MathScrips.h"
#include "Platform.h"

Eigen::MatrixXd CModel::CollectData() {
	Eigen::MatrixXd Zs(Platform.m_NumPeriods, Platform.m_Stocks.size());

	std::vector< std::vector<double> > v_Zs;
	v_Zs.resize(Platform.m_NumPeriods);

	for (int r = 0; r < Platform.m_NumPeriods; r++) {
		v_Zs[r].resize(Platform.m_Stocks.size(), 0.0);
		for (int c = Platform.m_Stocks.size() - 1; c >= 0; c--) {
			CStock *S = Platform.m_Stocks[c];
			double R = S->GetReturn(r);
			double A = S->GetAvgReturn();
			double D = S->GetDevReturn();
			double z = S->GetReturn(r) - S->GetAvgReturn(); //Divide(S->GetReturn(r) - S->GetAvgReturn(), S->GetDevReturn());
			Zs(r, c) = z;
			v_Zs[r][c] = z;
			if (abs(z) > 5.0) {
				printf("Warning: Z = %.2f in stock %s at prd %d\n", z, S->GetSymbol(), r);
			}
		}
	}
	return Zs;
}

bool CModel::RunPCA( Eigen::MatrixXd &InMat ) {
	if (!InMat.rows()) { printf("Error <RunPCA>: Input matrix has no data\n"); return false; }
	if (!InMat.cols()) { printf("Error <RunPCA>: Input matrix has no columns\n"); return false; }
	if (InMat.rows() <= InMat.cols()) { printf("Error <RunPCA>: Input matrix is undetermined. Nrows (%d) must be > Ncols (%d)\n", InMat.rows(), InMat.cols()); return false; }
	Eigen::JacobiSVD<Eigen::MatrixXd> svd(InMat, Eigen::ComputeFullV);
	Eigen::MatrixXd PCs = svd.matrixV();
	for (int c = 0; c < PCs.cols(); c++) {
		double Avg = 0.;
		double Dev = CalcStd(PCs.col(c).data(), 0, PCs.rows() - 1, &Avg);
		for (int r = 0; r < PCs.rows(); r++) PCs(r, c) = (PCs(r, c) - Avg) / Dev;
	}
	double TotalVar = 0.;
	for (int v = 0; v < svd.singularValues().size(); v++) {
		TotalVar += Sqr(svd.singularValues()[v]);
	}
	m_NumFeatures = 0;
	double RollingSum = 0.;
	const int NF = 4;
	for (int v = 0; v < NF/*svd.singularValues().size()*/; v++) {
		RollingSum += Sqr(svd.singularValues()[v]);
		m_NumFeatures = v + 1;
		if (RollingSum/TotalVar >= FEAT_THRESH) break; //FEAT_THRESH set to 85%
	}
	printf("Running with %d of %d features.\n", m_NumFeatures, PCs.cols());
	m_FeatureMatrix.resize(PCs.rows(), m_NumFeatures);
	m_Weights.resize(m_NumFeatures);
	for (int c = 0; c < m_NumFeatures; c++) {
		m_FeatureMatrix.col(c) = PCs.col(c);
		double w = Sqr(svd.singularValues()[c]) / TotalVar;
		m_Weights(c) = w;
	}
	double MaxFeature[NF] = {};
	double MinFeature[NF] = {};
	for (int i = 0; i < m_NumFeatures; i++) { MaxFeature[i] = -9999.; MinFeature[i] = 9999.; }
	size_t Nstocks = Platform.m_Stocks.size();
	for (size_t s = 0; s < Platform.m_Stocks.size(); s++) {
		CStock *S = Platform.m_Stocks[s];
		S->InitializeFeatures(m_NumFeatures);
		Eigen::VectorXd FV = m_FeatureMatrix.row(s);
		S->SetFeatures(FV);
		for (int f = 0; f < m_NumFeatures; f++) {
			double F = S->GetFeature(f);
			if (F > MaxFeature[f]) { MaxFeature[f] = F; }
			if (F < MinFeature[f]) { MinFeature[f] = F; }
		}
	}
	double SumW = 0.;
	for (int f = 0; f < m_NumFeatures; f++) {
		SumW += m_Weights(f);
		printf("Feature %d: wght = %.4f/%.4f, Min = %.5f, Max = %.5f\n", f, m_Weights(f), SumW, MinFeature[f], MaxFeature[f]);
	}

	return true;
}


double CModel::RunOneClustering(size_t NC, std::vector<CCluster> &Clusters, bool OUT) {
	FILE * pfile = 0;
	if (OUT) {
		std::stringstream ss_Filename;
		ss_Filename << "C:/data/ClusterComps_" << NC << ".log";
		std::string Filename = ss_Filename.str();
		fopen_s(&pfile, Filename.c_str(), "w");
	}
	if (!NC) { printf("Error <RunOneClusters>: Input number of clusters must be g.t. 0\n"); return -88888.0; }
	printf("Performing k-means clustering with %d clusters:\n", NC);
	size_t NumStocks = Platform.m_Stocks.size();
	//Initialize clusters
	//std::vector<CCluster> TempClusters;
	for (size_t C = 0; C < NC; C++) {
		CCluster Cl(m_NumFeatures);
		int RandIndx = rand() % NumStocks;
		Cl.AddStock(Platform.m_Stocks[RandIndx]);
		Cl.RecalcCentroid();
		Clusters.push_back(Cl);
	}

	double prevJ = 0.;
	double deltaJ = 9999.;
	int Niters = 0;
	int NumValidClusters = NC;
	while (abs(deltaJ) > 1.0E-6 && Niters < 1000000 ) {
		if (pfile) fprintf(pfile, "I\t%d\n", m_NumFeatures );
		for (size_t c = 0; c < NC; c++) { Clusters[c].ClearStocks(); }
		LOOP_LIST(Platform.m_Stocks, s) {
			CStock *S = *s;
			double MinDist = 9999.;
			int IndxAtMin = -1;
			for (size_t c = 0; c < NC; c++) {
				CCluster &C = Clusters[c];
				if (!C.m_IsValid) continue;
				double dist = S->CalcFeatureDist2(C.m_Centroid);
				if (dist < MinDist) { MinDist = dist; IndxAtMin = c; }
			}
			Clusters[IndxAtMin].AddStock(S, MinDist);
		}
		double J = 0.;
		for (size_t c = 0; c < NC; c++) {
			CCluster &C = Clusters[c];
			if (!C.m_IsValid) continue;
			if (!C.GetNumStocks()) { NumValidClusters--; Clusters[c].m_IsValid = false; }
			double AvgDist2 = C.GetAvgDist2();
			J += AvgDist2;
			if (pfile) {
				fprintf(pfile, "C\t%d\t%d\t", c, C.GetNumStocks());
				std::stringstream ss_Centroid;
				for (int i = 0; i < m_NumFeatures; i++) {
					ss_Centroid << C.m_Centroid(i) << "\t";
				}
				fprintf(pfile, "%s\n", ss_Centroid.str().c_str());
				LOOP_LIST(C.m_StockList, s) {
					CStock* S = *s;
					fprintf(pfile, "S\t%s\t", S->GetSymbol());
					std::stringstream ss_SFeatures;
					for (int i = 0; i < m_NumFeatures; i++) ss_SFeatures << S->GetFeature(i) << "\t";
					fprintf(pfile, "%s\n", ss_SFeatures.str().c_str());
				}
				fprintf(pfile, "\n");
			}
			C.RecalcCentroid();
		}
		J /= NumValidClusters;
		deltaJ = J - prevJ;
		prevJ = J;
		std::stringstream ss_OutStat; 
		ss_OutStat<<"\tNiter " << Niters << ": J = " << J;
		for (size_t c = 0; c < NC; c++) { ss_OutStat << ", C[" << c << "] " << Clusters[c].GetNumStocks(); }
		if (Niters < 10 || Niters % 100 == 0) printf("%s\n", ss_OutStat.str().c_str());
		Niters++;
	}
	if (!NumValidClusters) {
		printf("No valid clusters found!\n");
		return -99999.;
	}
	//Stats:
	double SumSize = 0.;
	int MinSize = 9999, MaxSize = -9999;
	for (size_t c = 0; c < NC; c++) {
		CCluster &C = Clusters[c];
		if (!C.m_IsValid) continue;
		int NS = C.GetNumStocks();
		SumSize += static_cast<double>(NS);
		if (NS < MinSize) MinSize = NS;
		if (NS > MaxSize) MaxSize = NS;
	}
	//if (pfile) fprintf(pfile, "W\t%d\n", Niters);
	printf("Results: %d-means clusters:\n", NC);
	printf("Algorithm converged after %d iterations with a mininum cost %.3f\n", Niters, prevJ);
	printf("%d valid clusters", NumValidClusters);
	printf("MinSize = %d\nMaxSize = %d\nAvgSize = %.0f\n\n", MinSize, MaxSize, SumSize / NumValidClusters);
	if (pfile) fclose(pfile);
	return prevJ;

}

void CModel::RunFullClustering( int MaxNclusters ) {
	Eigen::MatrixXd InData = CollectData();
	if (!RunPCA(InData)) {
		printf("Error <RunFullClustering>: PCA failed. Aborting clustering...\n");
		return;
	}
	double MinJ = 9999.;
	int NCatMinJ = -1;
	std::vector<CCluster> BestClusters;
	for (int K = 2; K <= MaxNclusters; K++) {
		double SumJ = 0.;
		int    NumRuns = 0;
		for (int R = 0; R < NUM_TRIALS; R++) {
			std::vector<CCluster> Clusters;
			double J = RunOneClustering(K, Clusters);
			if (J < 0) {
				printf("Error <RunFullClustering>: Bad result in running clustering algorithm for Nclusters = %d, error code %.0f\n", K, J);
				continue;
			}
			SumJ += J;
			NumRuns++;
		}
		double AvgJ = NumRuns ? SumJ / NumRuns : 99999.;
		if (AvgJ < MinJ) {
			MinJ = AvgJ;
			NCatMinJ = K;
		}
	}
	//Best K found
	bool OUTPUT = true;
	RunOneClustering(NCatMinJ, BestClusters, OUTPUT);
	WriteClusters(BestClusters);
}

bool CModel::WriteClusters(std::vector<CCluster> &Clusters) {
	FILE *pfile = 0;
	fopen_s(&pfile, "C:/data/Clusters.prn", "w");
	if (!pfile) {
		printf("Error <WriteClusters>: Cluster output could not be opened!\n");
		return false;
	}
	for (size_t c = 0; c < Clusters.size(); c++) {
		CCluster &C = Clusters[c];
		fprintf(pfile, "C%d\t", c);
		for (int f = 0; f < C.m_CentSize; f++) fprintf(pfile, "%.5f\t", C.m_Centroid(f));
		fprintf(pfile, "\n");
		LOOP_LIST(C.m_StockList, s) {
			CStock *S = *s;
			fprintf(pfile, "%s\t", S->GetSymbol());
			for (int f = 0; f < S->GetNumFeatures(); f++) fprintf(pfile, "%.5f\t", S->GetFeature(f));
			fprintf(pfile, "\n");
		}
		fprintf(pfile, "\n");
	}
	fclose(pfile);
	return true;
}

CModel Mdl;