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
	float predict();
	int getColVector(int, vector<float>*);
	int getRowVector(int, vector<float>*);
	float mulVector(vector<float>, vector<float>);
	float randomUniform(float dMinValue, float dMaxValue);
	float getRegularization(vector<float>, vector<float>);
private:
	float globalAverage;
	float bi;
	float bu;
	float alpha_u;
	float alpha_v;
	float alpha_w;
	float beta_u;
	float beta_v;
	float learningRate;
	float learningDesc;
	float eui;
	int userNum;	//user Number
	int itemNum;	//item Number
	int d;	//Number of latent dimensions
	int iterNum;	//Number of Iteration
	int *userCount;
	int *itemCount;
	//int *m_oriRating;
	Matrix m_oriRating;
	float *m_V;
	float *m_U;
	float *m_W;

	float *userBias;
	float *itemBias;
	
	float regularization;
};
float SVDPP::getRegularization(vector<float> U, vector<float> V)
{
	regularization = alpha_u / 2;
	float normU = 0;
	float normV = 0;
	float normW = 0;
	float coefW = 0;
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
	vector<float> W;
	for (int i = 0; i <userNum; i++)
	{
		tpsum = 0.0f;
	}
	return 0;
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
int SVDPP::getColVector(int col, vector<float> *output)
{
	for (int i = 0; i <userNum; i++)
	{
		output->push_back(m_oriRating.get(i,col));
	}
	return 1;
}

int SVDPP::getRowVector(int row, vector<float> *output)
{
	for (int i = 0; i <=itemNum; i++)
	{
		output->push_back(m_oriRating.get(row,i));
	}
	return 1;
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
	//memset(userCount, 0, sizeof(userCount));
	//memset(itemCount, 0, sizeof(itemCount));
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
	m_oriRating.init(userNum,itemNum);

	m_oriRating = new int[(userNum+1)*(itemNum + 1)];

	//	User and Item Bias
	userCount = new int[userNum + 1];
	itemCount = new int[itemNum + 1];
	userBias = new float[userNum + 1];
	itemBias = new float[itemNum + 1];

	//	initialize data array
	for (int i = 0; i <= userNum; i++)
	{
		userCount[i] = 0;
		userBias[i] = 0.0f;
	}

	for (int i = 0; i <= itemNum; i++)
	{
		itemCount[i] = 0;
		itemBias[i] = 0.0f;
	}
	for (int i = 0; i <(userNum+1)*(itemNum + 1) ; i++)
	{
		m_oriRating[i] = 0;
	}

	fseek(targetTrain, 0L, SEEK_SET);	//	Save Data to Array
	int ratingCount=0;
	while (!feof(targetTrain))
	{
		fscanf(targetTrain, "%d\t%d\t%d\t%s\n", &userID, &itemID, &oriRating, timestamp);
		m_oriRating[(itemNum+1)*userID + itemID]=oriRating;
		
		userCount[userID]++;
		userBias[userID] += oriRating;
		itemCount[itemID]++;
		itemBias[itemID] += oriRating;
		ratingCount++;
		globalAverage += oriRating;
	}

	int tpIndex = 802;
	vector<float> rowTest,colTest;
	getRowVector(tpIndex, &rowTest);
	getColVector(tpIndex, &colTest);
	vector<float>::iterator it;

	float tpsum = 0.0f, tpcount = 0;
	for (it = rowTest.begin(); it != rowTest.end(); it++)
	{
		if (*it!=0)
		{
			tpcount++;
		}
		tpsum += *it;
	}

	printf("user %d bias %f ", tpIndex, tpsum / tpcount);

	tpsum = 0.0f;
	tpcount = 0;
	for (it = colTest.begin(); it != colTest.end(); it++)
	{
		if (*it != 0)
		{
			tpcount++;
		}
		tpsum += *it;
	}

	printf("item %d bias %f \n", tpIndex, tpsum / tpcount);

	// BaseLine

	// globalAverage 
	globalAverage /= ratingCount;
	printf("global average: %f \n", globalAverage);

	// user Bias
	for (int i = 1; i <= userNum; i++)
	{
		userBias[i] /= userCount[i];
		userBias[i] -= globalAverage;
	}


	//item Bias
	for (int i = 1; i <= itemNum; i++)
	{
		itemBias[i] /= itemCount[i];
		itemBias[i] -= globalAverage;
	}

	printf("user [%d] Bias %f, item [%d] Bias %f\n", tpIndex, userBias[tpIndex], tpIndex,  itemBias[tpIndex]);


	if (!DEBUG)
	{
		for (int i = 1; i < userNum; ++i)
		{
			for (int j = 1; j < itemNum; j++)
			{
				printf("%d %d %d\n", i, j, m_oriRating[itemNum*i + j]);
			}
		}
	}

	// initialize U V matrix
	int tp_MNum = userNum*d + 1;
	m_U = new float[tp_MNum];

	srand(time(NULL));
	for (int i = 0; i < tp_MNum; i++)
	{
		m_U[i] = (randomUniform(0.0f, 1.0f)-0.5)*0.01;
	}
	tp_MNum = itemNum*d + 1;
	m_V = new float[tp_MNum];
	for (size_t i = 0; i < tp_MNum; i++)
	{
		m_V[i] = (randomUniform(0.0f, 1.0f) - 0.5)*0.01;
	}

	tp_MNum = itemNum*d + 1;
	m_W = new float[tp_MNum];
	for (size_t i = 0; i < tp_MNum; i++)
	{
		m_V[i] = (randomUniform(0.0f, 1.0f) - 0.5)*0.01;
	}


	//Load Fauxiliary Data
	fauxiliaryTrain = fopen("FauxiliaryTrain.txt", "r");
	if (!fauxiliaryTrain) printf("Load Fauxiliary Error!\n");
	while (!feof(fauxiliaryTrain))
	{
		fscanf(fauxiliaryTrain, "%d\t%d\t%d\t%s\n", &userID, &itemID, &oriRating, timestamp);
		//printf("%d %d %d %s\n", userID, itemID, oriRating, timestamp);
		if (userNum <= userID)
			userNum = userID;
		if (itemNum <= itemID)
		{
			itemNum = itemID;
		}
	}
	printf("uNUM %d,iNUM %d\n", userNum, itemNum);

}

void SVDPP::buildModel()
{

}

class Evaulator
{
public:
	float RMSEEvaluator();
	float MAEEvaluator();
private:
	int type;
};


int main()
{
	SVDPP svp;
	svp.loadData();
	Matrix mat(3,4);
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			mat.set(i, j, i*j);
		}
	}
	printf("%f\n",mat.get(2, 3));
	mat.print();
	return 0;
}