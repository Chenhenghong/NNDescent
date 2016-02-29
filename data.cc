#include <cstdio>
#include <iostream>
#include <ctime>

#define dataSize 1000
#define Dimension 128
#define Magic 256

using namespace std;

int main(){
	srand(time(NULL));
	for(int i=0;i<dataSize;i++){
		int v;
		v = rand()%Magic - (Magic/2);
		printf("%d",v);
		for(int j=1;j<Dimension;j++){
			v = rand()%Magic - (Magic/2);
			printf(" %d",v);
		}
		puts("");
	}
	return 0;
}