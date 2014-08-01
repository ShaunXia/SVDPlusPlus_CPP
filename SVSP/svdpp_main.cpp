#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <time.h>
#include "Matrix.h"
using namespace std;

#define MAXLINE 100
#define MAXCOUNT 100000
int DEBUG = 1;


class SVDPP
{
	//SVDPP(){};
	//~SVDPP(){};
public:
	void initModel();
	void loadData();
	void buildModel();
	float predict(int user, int item);
	float mulVector(vector<float>, vector<float>);
	float randomUniform(float dMinValue, float dMaxValue);
	float getRegularization(int user, int item);
	void vectorAdd(vector<float>, vector<float>, vector<float>&);
	void vectorMul(vector<float>, vector<float>, float&);
	void real_vectorMul(float, vector<float>&);
	void real_vectorMul(float, const vector<float>&, vector<float>&);
	float update(int user, int item);
private:
	double globalAverage;
	float bi;
	float bu;
	float alpha_u;
	float alpha_v;
	float alpha_w;
	float beta_u;
	float beta_v;
	double learningRate;
	double learningDesc;
	float eui;
	int userNum;	//user Number
	int itemNum;	//item Number
	int FuserNum;
	int FitemNum;
	int d;	//Number of latent dimensions
	int iterNum;	//Number of Iteration
	int *userCount;
	int *itemCount;

	Matrix m_oriRating;
	Matrix m_V;
	Matrix m_U;
	Matrix m_W;

	Matrix userRated;

	float *Wj;
	float *userBias;
	float *itemBias;
	
	float regularization;
};
void SVDPP::real_vectorMul(float coef, const vector<float>& input, vector<float> &result)
{
	result.resize(input.size());
	for (int i = 0; i < input.size(); i++)
	{
		result[i]=(coef*input[i]);
	}
}
void SVDPP::real_vectorMul(float coef, vector<float> &result)
{

	for (int i = 0; i < result.size(); i++)
	{
		result[i]=coef*result[i];
	}
}
void SVDPP::vectorAdd(vector<float> a, vector<float> b, vector<float> &result )
{
	result.resize(a.size());
	for (int i = 0; i < a.size(); i++)
	{
		result[i]=(a[i] + b[i]);
	}
}
void SVDPP::vectorMul(vector<float> a, vector<float> b, float &result)
{
	float sum = 0.0f;
	for (int i = 0; i < a.size(); i++)
	{
		sum += a[i] * b[i];
	}
	result = sum;
}

float SVDPP::getRegularization(int user,int item)
{
	vector<float> U;
	vector<float> V;
	m_U.getRowVector(user, &U);
	m_V.getRowVector(item, &V);
	bu = userBias[user];
	bi = itemBias[item];
	float normU = 0;
	float normV = 0;
	float normW = 0;
	vector<float>::iterator it;
	float tpsum = 0.0f, tpcount = 0;
	for (it = U.begin(); it != U.end(); it++)
	{
		tpsum += (*it)*(*it);
	}
	normU = sqrt(tpsum);
	tpsum = 0.0f;
	for (it = V.begin(); it != V.end(); it++)
	{
		tpsum += (*it)*(*it);
	}
	normV = sqrt(tpsum);


	float tpSumW = 0.0f;
	float tpNormW = 0.0f;

	int RatedNum = userRated.get(user, 0);
	vector<float> tpv1;
	for (int i = 1; i <= RatedNum; i++)
	{
		m_W.getRowVector(userRated.get(user, i), &tpv1);
		for (int j = 0; j < tpv1.size(); j++)
		{
			tpNormW += tpv1[j] * tpv1[j];
		}
		normW += sqrt(tpNormW);
		tpv1.clear();
	}


	regularization = alpha_u / 2 * normU+alpha_v/2*normV+beta_u/2*bu+
		beta_v/2*bi+alpha_w/2*normW;

	return regularization;
}
float SVDPP::randomUniform(float min, float max)
{
	int minInteger = (int)(min * 10000);
	int maxInteger = (int)(max * 10000);
	int randInteger = rand()*rand();
	int diffInteger = maxInteger - minInteger;
	int resultInteger = randInteger % diffInteger + minInteger;
	return resultInteger / 10000.0;
}
float SVDPP::mulVector(vector<float> v_a, vector<float> v_b)
{
	return 0;
}

void SVDPP::initModel()
{
	globalAverage = 0.0f;
	bi = 0.0f;
	bu = 0.0f;
	alpha_u = 0.01f;
	alpha_v = 0.01f;
	alpha_w = 0.01f;
	beta_u = 0.01f;
	beta_v = 0.01f;
	learningRate = 0.01f;
	learningDesc = 0.9f;
	d = 20;
	iterNum = 100;
}

void SVDPP::loadData()
{
	initModel();
	FILE *targetTrain, *fauxiliaryTrain;

	//Load Target Data
	targetTrain = fopen("FtargetTrain.txt","r");
	if (!targetTrain) printf("Load Train Error!\n");
	char timestamp[20];
	int userID, itemID, oriRating;
	while (!feof(targetTrain))
	{
		fscanf(targetTrain, "%d\t%d\t%d\t%s\n", &userID, &itemID, &oriRating, timestamp);
		if (userNum <= userID)
			userNum = userID;

		if (itemNum <= itemID)
		{
			itemNum = itemID;
		}
	}

	printf("uNUM %d,iNUM %d\n", userNum, itemNum);
	m_oriRating.init(userNum+1,itemNum+1);


	//	User and Item Bias
	userCount = new int[userNum + 1];
	itemCount = new int[itemNum + 1];
	userBias = new float[userNum + 1];
	itemBias = new float[itemNum + 1];

	//	initialize data array
	for (int i = 0; i <= userNum; i++)
	{
		userCount[i] = 0;
		userBias[i] = (randomUniform(0.0f, 1.0f) - 0.5)*0.01;
	}

	for (int i = 0; i <= itemNum; i++)
	{
		itemCount[i] = 0;
		itemBias[i] = (randomUniform(0.0f, 1.0f) - 0.5)*0.01;
	}

	fseek(targetTrain, 0L, SEEK_SET);	//	Save Data to Array
	int ratingCount=0;
	while (!feof(targetTrain))
	{
		fscanf(targetTrain, "%d\t%d\t%d\t%s\n", &userID, &itemID, &oriRating, timestamp);

		m_oriRating.set(userID, itemID, oriRating); //Save Rating Matrix
		userCount[userID]++;
		userBias[userID] += oriRating;				//user Bias
		itemCount[itemID]++;
		itemBias[itemID] += oriRating;				//item Bias
		ratingCount++;
		globalAverage += oriRating;					//globalAverage
	}



	// BaseLine

	// globalAverage 
	globalAverage /= ratingCount;
	printf("global average: %f \n", globalAverage);

	// user Bias
	for (int i = 1; i <= userNum; i++)
	{
		if (userCount[i] == 0)
			continue;
		userBias[i] /= userCount[i];
		userBias[i] -= globalAverage;
	}


	//item Bias
	for (int i = 1; i <= itemNum; i++)
	{
		if (itemCount[i] == 0)
			continue;
		itemBias[i] /= itemCount[i];
		itemBias[i] -= globalAverage;
	}



	if (!DEBUG)
	{
		for (int i = 1; i < userNum; ++i)
		{
			for (int j = 1; j < itemNum; j++)
			{
				printf("%d %d %d\n", i, j, m_oriRating.get(i,j));
			}
		}
	}

	// initialize U V W matrix
	m_U.init(userNum, d);

	srand(time(NULL));
	for (int i = 0; i < userNum; i++)
	{
		for (int j = 0; j < d; j++)
		{
			m_U.set(i, j, (randomUniform(0.0f, 1.0f) - 0.5)*0.01);
		}
	}

	m_V.init(itemNum, d);
	for (int i = 0; i < itemNum; i++)
	{
		for (int j = 0; j < d; j++)
		{
			m_V.set(i, j, (randomUniform(0.0f, 1.0f) - 0.5)*0.01);
		}
	}

	m_W.init(itemNum, d);
	for (int i = 0; i < itemNum; i++)
	{
		for (int j = 0; j < d; j++)
		{
			m_W.set(i, j, (randomUniform(0.0f, 1.0f) - 0.5)*0.01);
		}
	}



	//Load Fauxiliary Data
	FuserNum = 0;
	FitemNum = 0;

	fauxiliaryTrain = fopen("FauxiliaryTrain.txt", "r");
	if (!fauxiliaryTrain) printf("Load Fauxiliary Error!\n");
	while (!feof(fauxiliaryTrain))
	{
		fscanf(fauxiliaryTrain, "%d\t%d\t%d\t%s\n", &userID, &itemID, &oriRating, timestamp);
		if (FuserNum <= userID)
			FuserNum = userID;
		if (FitemNum <= itemID)
		{
			FitemNum = itemID;
		}
	}

	userRated.init(FuserNum + 1, FitemNum + 1);	//Rated Matrix

	fseek(fauxiliaryTrain, 0L, SEEK_SET);	//	Save Data to Array
	while (!feof(fauxiliaryTrain))
	{
		fscanf(fauxiliaryTrain, "%d\t%d\t%d\t%s\n", &userID, &itemID, &oriRating, timestamp);
		userRated.set(userID, 0, userRated.get(userID, 0) + 1);
		userRated.set(userID, (int)userRated.get(userID, 0), itemID);
	}
	
	printf("fauxiliaryTrain uNUM %d,iNUM %d\n", FuserNum, FitemNum); 

}

void SVDPP::buildModel()
{
	loadData();
	srand(time(NULL));

	for (int i = 0; i < iterNum; i++) //Iteration Num
	{
		
		for (int i = 1; i < FitemNum; i++) //1 . 2 .3 ...Fauxiliary Num
		{
			int R = rand() % userNum + 1;
			int It = rand() % itemNum + 1;
			if (m_oriRating.get(R, It) != 0)
				update(R, It);
			else
				--i;
		}
		learningRate = learningRate*learningDesc; //Update LearningRate
	}

}

float SVDPP::update(int user, int item)
{

	//get Current User Item Bias
	bi = itemBias[item];
	bu = userBias[user];

	//get Regularization for User-Item
	regularization= getRegularization(user, item);


	vector<float> v_wj;

	int RatedNum = userRated.get(user, 0);
	vector<float> tpv1, tpv2;
	m_W.getRowVector(userRated.get(user, 0), &tpv2);
	float coef = 1 / (sqrt(!RatedNum ? 1 : RatedNum));

	
	//Get SUM(Wj)
	for (int i = 1; i <= RatedNum; i++)
	{
		m_W.getRowVector(userRated.get(user, i), &tpv1);
		vectorAdd(tpv2, tpv1, tpv2);
		tpv1.clear();
	}

	real_vectorMul(coef, tpv2, v_wj);	//¦¤U= |Rated|/sqrt(|Rated|)*SUM(Wj)


	vector<float> v_U, v_V, tpVec1, tpVec2,tpVec3,tpVec4,tpVec5;
	float tempRes;
	m_U.getRowVector(user, &v_U);
	m_V.getRowVector(item, &v_V);

	vectorAdd(v_U, v_wj, tpVec1);		//(U + ¦¤U)
	vectorMul(tpVec1, v_V, tempRes);	//(U+¦¤U)*V

	float est_rui = bu + bi + globalAverage +regularization + tempRes;	//Est R-ui

	float err = m_oriRating.get(user, item) - est_rui;	//Error E_ui
	float nErr = -1.0*err;				//-E_ui

	//Now Update
	tpVec1.clear();
	tpVec2.clear();
	float new_bu, new_bi, new_globalAverage;
	vector<float> new_v_U, new_v_V, new_v_W;
	new_v_U = v_U;
	new_v_V = v_V;
	new_v_W = v_wj;

	//Update U
	real_vectorMul(nErr, v_V, tpVec1);			//-Eui*V
	real_vectorMul(alpha_u, v_U, tpVec2);		//alpha_u* U
	vectorAdd(tpVec1, tpVec2, tpVec3);			//(-Eui*V) + (alpha_u* U)

	//theta = theta - learningRate*( (-Eui*V) + (alpha_u* U) )
	real_vectorMul(-1.0*learningRate, tpVec3, tpVec4); 
	vectorAdd(v_U, tpVec4, new_v_U);

	tpVec1.clear();
	tpVec2.clear();
	tpVec3.clear();
	tpVec4.clear();
	
	//Update V
	vectorAdd(v_wj, v_U, tpVec1);			//U+¦¤U
	real_vectorMul(nErr, tpVec1, tpVec2);	//-Eui*(U+¦¤U)
	real_vectorMul(alpha_v, v_V, tpVec3);	//alpha_v*V
	vectorAdd(tpVec2, tpVec3, tpVec4);		//-Eui*(U+¦¤U) + alpha_v*V

	//theta = theta - learningRate*( -Eui*(U+¦¤U) + alpha_v*V )
	real_vectorMul(-1 * learningRate, tpVec4, tpVec5);
	vectorAdd(v_V, tpVec5, new_v_V);

	//Update W
	tpVec1.clear();
	tpVec2.clear();
	tpVec3.clear();
	tpVec4.clear();
	tpv1.clear();
	float updateW;
	updateW = nErr*coef; // -Eui*1/sqrt(|Rated|)
	real_vectorMul(updateW, v_V, tpVec1);//updateW * V

	for (int i = 1; i <= RatedNum; i++)
	{
		m_W.getRowVector(userRated.get(user, i), &tpv1);	//get item Vector
		real_vectorMul(alpha_w, tpv1, tpVec2);				//alpha_w*W
		vectorAdd(tpVec1, tpVec2, tpVec3);					//updateW * V + alpha_w*W

		//theta = theta - learningRate*( updateW * V + alpha_w*W )
		real_vectorMul(-1 * learningRate, tpVec3, tpVec4);
		vectorAdd(tpVec4, tpv1, new_v_W);

		//Save New Wj to W matrix
		m_W.setRowVector(userRated.get(user, i),new_v_W);

		new_v_W.clear();
		tpv1.clear();
	}
	

	//Update Bu
	new_bu = bu - learningRate*(nErr + beta_u*bu);

	//Update Bi
	new_bi = bi - learningRate*(nErr + beta_v*bi);

	//Update Mu globalAverage
	new_globalAverage = globalAverage - learningRate* nErr;


	//Save New Parameter
	globalAverage = new_globalAverage;

	bu = new_bu;
	bi = new_bi;
	v_U = new_v_U;
	v_V = new_v_V;
	v_wj = new_v_W;

	m_U.setRowVector(user, v_U);
	m_V.setRowVector(item, v_V);

	return est_rui;
}

float SVDPP::predict(int user, int item)
{
	// The same as Update

	bi = itemBias[item];
	bu = userBias[user];
	
	getRegularization(user, item);

	float delta_U_Coef = 1.0 / sqrt(userRated.get(user, 0)*1.0f)+0.0001f;
	vector<float> v_wj;

	int RatedNum = userRated.get(user, 0);
	vector<float> tpv1, tpv2;
	m_W.getRowVector(userRated.get(user, 0), &tpv2);
	float coef = RatedNum / (sqrt(!RatedNum? 1 : RatedNum));

	for (int i = 1; i <= RatedNum; i++)
	{
		m_W.getRowVector(userRated.get(user, i), &tpv1);
		vectorAdd(tpv2, tpv1, tpv2);
		tpv1.clear();
	}
	real_vectorMul(coef, tpv2, v_wj);


	vector<float> v_U, v_V, tpVec1, tpVec2, tpVec3, tpVec4;
	float tempRes;
	m_U.getRowVector(user, &v_U);
	m_V.getRowVector(item, &v_V);

	real_vectorMul(delta_U_Coef, v_wj); 
	vectorAdd(v_U, v_wj, tpVec1);
	vectorMul(tpVec1, v_V, tempRes);

	float est_rui = tempRes + bu + bi + globalAverage;
	return est_rui;
}
class Evaulator
{
public:
	float RMSEEvaluator(SVDPP &svdpp);
	float MAEEvaluator(SVDPP &svdpp);
private:
	int type;
	FILE *testSet;
};
float Evaulator::RMSEEvaluator(SVDPP &svdpp)
{
	testSet = fopen("FtargetValidation.txt", "r");
	if (!testSet) printf("Load testSet Error!\n");
	char timestamp[20];
	int userID, itemID, oriRating;
	int userNum=0, itemNum=0;
	float est_ui = 0.0f;
	float err;
	float sumErr = 0.0f;
	int recCount = 0;
	while (!feof(testSet))
	{
		fscanf(testSet, "%d\t%d\t%d\t%s\n", &userID, &itemID, &oriRating, timestamp);
		est_ui = svdpp.predict(userID, itemID);
		err = oriRating - est_ui;

		//ignore item not in U V 
		if (est_ui<0 || est_ui>5)
		{
			err = 0;
			continue;
		}

		
		sumErr += err*err;
		++recCount;
	}
	float RMSE = sqrt(sumErr / recCount);
	return RMSE;
}
float Evaulator::MAEEvaluator(SVDPP &svdpp)
{
	testSet = fopen("FtargetValidation.txt", "r");
	if (!testSet) printf("Load testSet Error!\n");
	char timestamp[20];
	int userID, itemID, oriRating;
	int userNum = 0, itemNum = 0;
	float est_ui = 0.0f;
	float err;
	float sumErr = 0.0f;
	int recCount = 0;
	while (!feof(testSet))
	{
		fscanf(testSet, "%d\t%d\t%d\t%s\n", &userID, &itemID, &oriRating, timestamp);
		est_ui = svdpp.predict(userID, itemID);
		err = abs(oriRating - est_ui);
		if (est_ui<0 || est_ui>5)
		{
			err = 0;
			continue;
		}
		sumErr += err;
		++recCount;
	}
	float MAE = sqrt(sumErr / recCount);
	return MAE;
}

int main()
{
	SVDPP svp;
	svp.buildModel();
	Evaulator evaul;
	printf("RMSE: %f\n", evaul.RMSEEvaluator(svp));
	
	return 0;

}