#pragma once
#ifndef __MODEL_H
#define __MODEL_H
#include "Cluster.h"



const int NUM_TRIALS = 10;
const double FEAT_THRESH = 0.85;

class CModel {
public:
	CModel() : m_NumFeatures(0) {}
	~CModel() {}

	Eigen::MatrixXd CollectData();
	bool RunPCA( Eigen::MatrixXd &InMat );
	double RunOneClustering( size_t NC, std::vector<CCluster> &Clusters, bool OUT=false);
	bool WriteClusters(std::vector<CCluster> &Clusters);
	void RunFullClustering(int MaxNclusters);

	
	Eigen::MatrixXd m_FeatureMatrix;
	Eigen::VectorXd m_Weights;
	int m_NumFeatures;
};

extern CModel Mdl;

#endif