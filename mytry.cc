#include <cstdio>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <unordered_map>

#define dataSize 100000
#define Dimension 128
#define K 10
#define pb push_back
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


vector<int> R[dataSize];	// only store id   //maybe we could change to unordered_map to acclerate -> maybe we not need this array cuz  BB (not sure in parallel)
unordered_map<int, int> BB[dataSize];	// BB[i] = B[i] U R[i]

class myHeap{	 // in order to maintain nearest K elements, we should maintain a large-root heap of size K+1 and each time we erase the top one
public:
	int capa[K+2];	// only store id!!
	int heapSize;
	int centralPoint_id; 	// cuz we build heap for each vertex, this is the identity of that vertex
	int update_flag;
	
	myHeap(){
		heapSize = 0;
	}
	
	void append(int k){	
		//append new one
		update_flag = 0;
		for(int i=1;i<=heapSize;i++){ //if new element exist?  only compare there ID, less cost
			if(capa[i]==k)
				return ;
		}
		
		//if new element farther then most large one (heap's root), it's no more need to add
		double dist_root = calcDistance(capa[1], capa[centralPoint_id]);
		double dist_new = calcDistance(capa[k], capa[centralPoint_id]);
		if(dist_new > dist_root)
			return ;
		
		//otherwise heap's root should be the farthest one, we need to erase if heap is full
		update_flag = 1;
		if(heapSize<K){ // not full
			capa[++heapSize]=k;
			upMaintain(heapSize);
		}
		
		else{
			rooterase();  // erase the root element
			capa[1] = k;
			// we should add information of new element into reserve array and BB array	BB[v] = B[v] U R[v]
			R[k].pb(centralPoint_id); 	// reserve array
			
			if(BB[centralPoint_id][k]==0)	// new element not in BB array
 				BB[centralPoint_id].insert( make_pair(k,1) );	// update BB[v] from  B[v]
			R[k].pb(centralPoint_id);	// update Reserve array
			if(BB[k][centralPoint_id]==0)	// update BB[new_element] from  R[new_element]
				BB[k].insert( make_pair(centralPoint_id,1) );
			
			downMaintain();  // update heap 
		}

		return ;
	}
	
	void upMaintain(int pos){
		int x = pos;
		while(x>1){
			int fa = x >> 1;
			double dist_up = calcDistance(capa[fa], capa[centralPoint_id]);
			double dist_cur = calcDistance(capa[x], capa[centralPoint_id]);
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
			double farrdist = calcDistance(capa[cur], capa[centralPoint_id]);
			double dist_left = calcDistance(capa[left], capa[centralPoint_id]);
			if(dist_left > farrdist){
				farrdist = dist_left;
				rec = left;
			}
			
			if(right<=heapSize){
				double dist_right = calcDistance(capa[right], capa[centralPoint_id]);
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
		int re_id = capa[1];	// erase information in reserve array 
		R[re_id].erase(remove(R[re_id].begin(), R[re_id].end(), centralPoint_id), R[re_id].end());	// maybe change to unordered_map
		// also we should update our BB array!  BB[v] = B[v] U R[v]
		BB[centralPoint_id].erase(re_id); // erase element re_id in BB[id] comes from B[id]
		BB[re_id].erase(centralPoint_id); // erase element id in BB[re_id] comes from R[re_id]
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
	bool flag = true;
	do{
		tmp.clear();
		sum = 0;
		while(sum<K){
			tmp.pb(dist(rd));
			sum++;
		}
		for(int i=0;i<tmp.size();i++){
			for(int j=i+1;j<tmp.size();j++)
				if(tmp[i]==tmp[j]){ // re-produce 
					flag = false;
					break;
				}
			if(!flag)
				break;
		}		
	}while(!flag);	
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
			R[k].pb(i);
		}
	}
	return ;
}

void work(){	// basic NN_DESCENT
	for(int i=0;i<dataSize;i++){	// we should make it parallel work !!
		for(int k=1;k<=B[i].heapSize;k++)
			BB[i].insert( make_pair( B[i].capa[k],1) );
		for(auto &k : R[i]){
			if(BB[i][k]==0) // no repeated element
				BB[i].insert( make_pair(k,1) );
		}	
	}
	//now we construct our initial BB array
	int update_counter;
	while(true){
		update_counter = 0;
		for(int v=0;v<dataSize;v++){ // need parallel
			for(auto &t1 : BB[v]){
				int u1 = t1.first;
				for(auto &t2 : BB[u1]){
					int u2 = t2.first;
					B[v].append(u2);// try to add new element u2 (from neighbor's neighbor)
					update_counter += B[v].update_flag;
				}
			}
		}
		if(update_counter==0)
			break;
	}
	// finish NN-DESCENT
	return ;
}

void eval(){	// to eval the acc rate of algorithm
	return ;
}

int main(){
	loadData();
	init();
	work();
	eval();
	
	return 0;
}