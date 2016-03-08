#include <cstdio>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <ctime>
#include <cmath>
#include <cstdlib>

#define dataSize 10000
#define Dimension 128
#define pb push_back
#define K 10

// new features
#define sampleRate 0.8
#define Termination 0.001

using namespace std;

struct point{
	int dim[Dimension];
};

struct position{ // for elements in K-NN heap
	int id;
	double dist;
	bool sample;
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

vector<int> rev_old[dataSize];	// only store id   old'  
vector<int> rev_new[dataSize];	// only store id   new'
unordered_set<int> old_un[dataSize];	// old[v] U SAMPLE(old'[v])
unordered_set<int> new_un[dataSize];	// new[v] U SAMPLE(new'[v])

class myHeap{	 // in order to maintain nearest K elements, we should maintain a large-root heap of size K+1 and each time we erase the top one
public:
	position capa[K+2]; 
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
			if(capa[i].id==k)
				return ;
		}
		
		dist_new = calcDistance(k, centralPoint_id);
		//if new element farther then most large one (heap's root), and  heap is full, then it's no more need to add
		if(heapSize==K){
			if(dist_new > capa[1].dist)
			return ;
		}
		
		//otherwise heap's root should be the farthest one, we need to erase if heap is full
		update_flag = 1;
		if(heapSize<K){ // not full
			heapSize++;
			capa[heapSize].id = k;
			capa[heapSize].dist = dist_new;
			capa[heapSize].sample = true;
			//R[k].insert(centralPoint_id);
			upMaintain(heapSize);
		}
		
		else{
			// sth problems with erase
		//	rooterase();  // erase the root element
			capa[1].id = k;
			capa[1].dist = dist_new;
			capa[1].sample = true;
			//R[k].insert(centralPoint_id); 	// reserve array			
			downMaintain();  // update heap 
		}

		return ;
	}
	
	void upMaintain(int pos){
		int x = pos;
		while(x>1){
			int fa = x >> 1;
			double dist_up = capa[fa].dist;
			double dist_cur = capa[x].dist;
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
			double farrdist = capa[cur].dist;
			double dist_left = capa[left].dist;
			if(dist_left > farrdist){
				farrdist = dist_left;
				rec = left;
			}
			
			if(right<=heapSize){
				double dist_right = capa[right].dist;
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
	
	/*
	void rooterase(){
		int re_id = capa[1].id;	// erase information in reserve array 
		R[re_id].erase(centralPoint_id);
		return ;
	}
	*/
	
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

vector<int> genInit(){	// generate initial K-NN 
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
	
	//clear all
	for(int i=0;i<dataSize;i++){
		old_un[i].clear();
		rev_old[i].clear();
		rev_new[i].clear();
		new_un[i].clear();
	}
	
	// re-build old & new union for each element
	for(int i=0;i<dataSize;i++){	// we should make it parallel work !!
		vector<int> tmp_sample;
		for(int k=1;k<=B[i].heapSize;k++){
			int t_id = B[i].capa[k].id;
			if(B[i].capa[k].sample==false){	// old one	
				old_un[i].insert(t_id);
				rev_old[t_id].push_back(B[i].centralPoint_id);
			}
			else{	//new one
				tmp_sample.push_back(k); // not insert t_id  cuz we should set bool flag of these sample marked to false
			}
		}
		
		if(tmp_sample.empty())
			continue;
		//deal with SAMPLE WORK
		int sample_num = int(sampleRate * K);
		random_shuffle(tmp_sample.begin(),tmp_sample.end());	// sample pick
		int minm = tmp_sample.size() < sample_num ? tmp_sample.size() : sample_num;
		for(int k=0;k<minm;k++){
			int t_id = B[i].capa[ tmp_sample[k] ].id;
			new_un[i].insert(t_id);
			rev_new[t_id].push_back(i); 
			B[i].capa[tmp_sample[k]].sample = false; //	set bool flag
		}
	}
	
	// re-build old union & new union
	for(int i=0;i<dataSize;i++){
		// old  combine all elements
		for(auto &k : rev_old[i]){
			if(old_un[i].find(k)==old_un[i].end())	//no repeated
				old_un[i].insert(k);
		}
		// new only combine pK elements
		random_shuffle(rev_new[i].begin(),rev_new[i].end());
		
		int lim = int(sampleRate * K);
		int cnt = 0;
		for(auto &k: rev_new[i]){
			if(new_un[i].find(k)==new_un[i].end()){
				new_un[i].insert(k);
				cnt ++;
				if(cnt==lim)
					break;
			}
		}
	}
	
	return ;
}

void work(){	// basic NN_DESCENT
	int update_counter;
	int termi = int(Termination * dataSize * K);
	while(true){
		update_set();
		update_counter = 0;
		for(int i=0;i<dataSize;i++){	// need parallel
			// both u1 and u2 are new
			for(auto &u1 : new_un[i])
				for(auto &u2 : new_un[i])
					if(u1<u2){
						B[u1].append(u2);
						update_counter += B[u1].update_flag;
						B[u2].append(u1);
						update_counter += B[u2].update_flag;
					}
			// u1 is new and u2 is old
			for(auto &u1 : new_un[i])
				for(auto &u2 : old_un[i])
					if(u1!=u2){
						B[u1].append(u2);
						update_counter += B[u1].update_flag;
						B[u2].append(u1);
						update_counter += B[u2].update_flag;	
					}		
		}
		
		if(update_counter<=termi)
			break;
		
		cout<<update_counter<<"####"<<endl;
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