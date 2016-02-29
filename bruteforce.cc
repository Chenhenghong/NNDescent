#include <cstdio>
#include <iostream>
#include <algorithm>
#include <vector>
#include <utility>
#include <ctime>

#define dataSize 1000
#define Dimension 128
#define pb push_back

using namespace std;

struct point{
	int dim[Dimension];
};

vector<point> DataSet;

vector < pair<double , int> > element[dataSize]; // default of sort algorithm is incresing order   

double calcDistance(int a, int b){
	double tmp = 0.0;
	point X = DataSet[a];
	point Y = DataSet[b];
	for(int i=0;i<Dimension;i++){
		double tp = (X.dim[i]-Y.dim[i]); // we use Euclidean distance  
		tp *= tp;
		tmp += tp;
	}
	return tmp;
}

void loadData(){
	//assume we own 100000 data with 128 size 
	point tmp;
	for(int i=0;i<dataSize;i++){
		for(int k=0;k<Dimension;k++){
			int sc;
			scanf("%d",&sc);
			tmp.dim[k]=sc;
		}
		DataSet.pb(tmp);
	}
	return ;
}
int main(){
	loadData();
	auto start = clock();
	for(int i=0;i<dataSize;i++){
		for(int j=0;j<dataSize;j++){
			element[i].pb( make_pair( calcDistance(i,j), j) );
		}
		sort(element[i].begin(),element[i].end());
	}
	auto finish = clock();
    cout<<(finish-start)/CLOCKS_PER_SEC<<endl;
	return 0;
	
}