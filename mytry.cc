#include <cstdio>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <ctime>

#define dataSize 1000
#define Dimension 128
#define pb push_back
#define K 10
using namespace std;

struct point{
	int dim[Dimension];
};

vector<point> DataSet;

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


unordered_set<int> R[dataSize];	// only store id   
unordered_set<int> BB[dataSize];	// BB[i] = B[i] U R[i]

class myHeap{	 // in order to maintain nearest K elements, we should maintain a large-root heap of size K+1 and each time we erase the top one
public:
	pair<int, double> capa[K+2]; // id + distance
	int heapSize;
	int centralPoint_id; 	// cuz we build heap for each vertex, this is the identity of that vertex
	int update_flag;
	double dist_new;
	
	myHeap(){
		heapSize = 0;
	}
	
	void append(int k){	
		//append new one
		update_flag = 0;
		
		if(k==centralPoint_id)
			return ;
		
		for(int i=1;i<=heapSize;i++){ //if new element exist?  only compare there ID, less cost
			if(capa[i].first==k)
				return ;
		}
		
		dist_new = calcDistance(k, centralPoint_id);
		//if new element farther then most large one (heap's root), and  heap is full, then it's no more need to add
		if(heapSize==K){
			if(dist_new > capa[1].second)
			return ;
		}
		
		//otherwise heap's root should be the farthest one, we need to erase if heap is full
		update_flag = 1;
		if(heapSize<K){ // not full
			heapSize++;
			capa[heapSize].first=k;
			capa[heapSize].second=dist_new;
			R[k].insert(centralPoint_id);
			upMaintain(heapSize);
		}
		
		else{
			// sth problems with erase
			rooterase();  // erase the root element
			capa[1].first = k;
			capa[1].second = dist_new;
			R[k].insert(centralPoint_id); 	// reserve array			
			downMaintain();  // update heap 
		}

		return ;
	}
	
	void upMaintain(int pos){
		int x = pos;
		while(x>1){
			int fa = x >> 1;
			double dist_up = capa[fa].second;
			double dist_cur = capa[x].second;
			if(dist_cur <= dist_up) // large-root heap 
				break;
			swap(capa[fa],capa[x]); // continue  updating
			x = fa;
		}
		
		return ;
	}
	
	void downMaintain(){
		int cur = 1;
		int left,right;
		while(cur*2<=heapSize){
			left = cur << 1;
			right = left | 1;
			int rec = cur;
			double farrdist = capa[cur].second;
			double dist_left = capa[left].second;
			if(dist_left > farrdist){
				farrdist = dist_left;
				rec = left;
			}
			
			if(right<=heapSize){
				double dist_right = capa[right].second;
				if(dist_right > farrdist){
					farrdist = dist_right;
					rec =right;
				}
			}
			
			if(rec==cur)	// no update
			 	break;
			else{
				swap(capa[cur],capa[rec]);
				cur = rec;
			}
		}
		return ;
	}
	
	void rooterase(){
		int re_id = capa[1].first;	// erase information in reserve array 
		R[re_id].erase(centralPoint_id);
		return ;
	}
	
	void clear(){
		heapSize = 0;
		return ;
	}
};


myHeap B[dataSize];   //	only store id

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

vector<int> genInit(){
	//cuz K is small, we choose compare directly instead of unordered_map	
	vector<int> tmp;
	int sum;
	//c++11
	random_device rd;
	uniform_int_distribution<int> dist(0,dataSize-1); //closed
	bool need = true;
	while(need){
		bool repeat = false;
		tmp.clear();
		sum = 0;
		while(sum<K){
			tmp.pb(dist(rd));
			sum++;
		}
		for(int i=0;i<tmp.size();i++){
			for(int j=i+1;j<tmp.size();j++)
				if(tmp[i]==tmp[j]){ // re-produce 
					repeat = true;
					break;
				}
			if(repeat)
				break;
		}		
		if(!repeat)
			need = false;
	}
	
	return tmp;
}

void init(){
	for(int i=0;i<dataSize;i++)	//initial array B
		B[i].centralPoint_id = i;
	
	srand(time(NULL));// cuz K is so small
	for(int i=0;i<dataSize;i++){
		vector<int> initial = genInit();
		for(auto &k : initial){
			B[i].append(k); 
		}
		
	}
	return ;
}

void update_set(){ // it seems that iterative time is not too many, we choose to lazy update our BB set first
	for(int i=0;i<dataSize;i++){	// we should make it parallel work !!
		for(int k=1;k<=B[i].heapSize;k++)
			BB[i].insert(B[i].capa[k].first);
		for(auto &k : R[i]){
			if(BB[i].find(k)==BB[i].end()) // no repeated element
				BB[i].insert(k);
		}	
	}
	return ;
}

void work(){	// basic NN_DESCENT
	//initial BB array  BB[i] = B[i] U R[i]
	update_set();
	//now we construct our initial BB array
	int update_counter;
	while(true){
		update_counter = 0;
		for(int v=0;v<dataSize;v++){ // need parallel
			for(auto &u1 : BB[v]){
				for(auto &u2 : BB[u1]){
					B[v].append(u2);// try to add new element u2 (from neighbor's neighbor)
					update_counter += B[v].update_flag;
				}
			}
		}
		if(update_counter==0)
			break;
		cout<<update_counter<<"####"<<endl;
		update_set();
	}
	// finish NN-DESCENT
	return ;
}

void eval(){	// to eval the acc rate of algorithm
	return ;
}

int main(){
	loadData();
	auto start = clock();
//	puts("Finish loading...");
	init();
//	puts("Finish initilize...");  // ok!
	work();
//	puts("Finish algorithm...");
	eval();
	auto finish = clock();
    cout<<(finish-start)/CLOCKS_PER_SEC<<endl;
	return 0;
}