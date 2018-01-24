#include <vector>
#include <list>
#include <map>
#include <set>
#include <deque>
#include <queue>
#include <stack>
#include <bitset>
#include <algorithm>
#include <functional>
#include <numeric>
#include <utility>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cctype>
#include <string>
#include <cstring>
#include <ctime>
#include <string.h>

using namespace std;

typedef long long int64;
typedef unsigned long long uint64;
#define two(X) (1<<(X))
#define twoL(X) (((int64)(1))<<(X))
#define contain(S,X) (((S)&two(X))!=0)
#define containL(S,X) (((S)&twoL(X))!=0)
const double pi=acos(-1.0);
const double eps=1e-11;
template<class T> inline void checkmin(T &a,T b){if(b<a) a=b;}
template<class T> inline void checkmax(T &a,T b){if(b>a) a=b;}
template<class T> inline T sqr(T x){return x*x;}
typedef pair<int,int> ipair;
#define SIZE(A) ((int)A.size())
#define LENGTH(A) ((int)A.length())
#define MP(A,B) make_pair(A,B)
#define PB(X) push_back(X)
typedef vector<int> VI;


int source_group=3,target_group=2;


int n,m,c;      // node number and edge number in original graph
int *degree,**graph;
int *area;
double *pagerank;
double *pagerank_tmp;

void load_area(string filename)  // set global variable area
{
	area=new int[n];
	FILE *f=fopen(filename.c_str(),"r");
	for (int i=0;i<n;i++) fscanf(f,"%d",&area[i]);
	std::fclose(f);
}

void load_graph(string filename)  // set global variable degree and graph
{
	FILE *f=fopen(filename.c_str(),"r");
	fscanf(f,"%d%d",&n,&m);
	int *e_list=new int[m+m];
	for (int i=0;i<m+m;i++) fscanf(f,"%d",&e_list[i]);
	degree=new int[n];
	for (int i=0;i<n;i++) degree[i]=0;
	for (int i=0;i<m+m;i++) 
		if (e_list[i]!=e_list[i^1]) // ignore self-loops
			degree[e_list[i]]++;
	graph=new int* [n];
	for (int i=0;i<n;i++) graph[i]=new int[degree[i]];
	for (int i=0;i<n;i++) degree[i]=0;
	pagerank=new double [n];
	pagerank_tmp=new double [n];
	for (int i=0;i<n;i++) pagerank[i]=0;
	for (int i=0;i<m+m;i++) if (e_list[i]!=e_list[i^1]) graph[e_list[i]][degree[e_list[i]]++]=e_list[i^1];
	delete[] e_list;
	std::fclose(f);
}

int myrandom()
{
	int v1=rand()&32767;
	int v2=rand()&32767;
	return (v1<<15)|v2;
}

int myrandom(int n)
{
	return myrandom()%n;
}

VI get_community_kernel(int mask) // mask is binary 00..010..00
{
	VI ret;
	vector<pair<int, int> > q;
	for (int i = 0; i < n; ++i)
		if ((area[i] & mask) == mask)
			q.push_back(MP(degree[i], i));  // make pair for sorting
	sort(q.begin(), q.end());
	std::reverse(q.begin(), q.end());
	for (int i = 0; i < SIZE(q) / 1; ++i)  // choose top 1% users (with highest degrees) in this area
		ret.push_back(q[i].second);
	return ret;
}

void build_network(vector<VI>& kernels, vector< pair<double,int> >& candidates, string candidate_file)
{
	//printf("DEBUG : build_network : ");  // c:SIZE(kernels)
	set<int> S1,S2;
	for (int j=0;j<SIZE(kernels[source_group]);j++) S1.insert(kernels[source_group][j]);
	for (int j=0;j<SIZE(kernels[target_group]);j++) S2.insert(kernels[target_group][j]);

	if (SIZE(S1)==0 || SIZE(S2)==0) return;

	set<int> candidates_tmp;
	FILE* fp=fopen(candidate_file.c_str(),"wt");
	for (set<int>::iterator it=S1.begin();it!=S1.end();++it)
	{
		for (int j=0;j<degree[*it];j++)
		{
			int v=graph[*it][j];
			if ((S1.find(v) == S1.end()) && S2.find(v) == S2.end()) candidates_tmp.insert(v);
		}
	}
	for (set<int>::iterator it=candidates_tmp.begin();it!=candidates_tmp.end();++it)
	{
		candidates.push_back(pair<double,int>(0,*it));
		fprintf(fp,"%d\n",*it);
	}
	std::fclose(fp);
	//printf("node = %d   edge = %d\n",node,edge_number);
}


const int pagerank_iteration=10;
const double back_prob=0.1;

void calculate_pagerank(VI targets)
{
	for (int i=0;i<SIZE(targets);i++) pagerank[targets[i]]=1/SIZE(targets);
	for (int iteration=0;iteration<pagerank_iteration;iteration++)
	{
		for (int i=0;i<n;i++)
		{
			pagerank_tmp[i]=0;
			for (int j=0;j<degree[i];j++) pagerank_tmp[i]+=pagerank[graph[i][j]]/degree[graph[i][j]];
		}
		for (int i=0;i<n;i++) pagerank_tmp[i]*=(1-back_prob);
		for (int i=0;i<SIZE(targets);i++) pagerank_tmp[targets[i]]+=back_prob/SIZE(targets);
		for (int i=0;i<n;i++) pagerank[i]=pagerank_tmp[i];
	}
}


int main(int argc,char **args)
{
	string graph_file="dblp_graph.txt";
	string community_file="community.txt";
	string output_file="result.txt";
	string candidate_file="candidates.txt";
    FILE* fp=fopen(output_file.c_str(),"wt");
	vector<int> area;
	for (int i=0;i<6;i++) area.push_back(pow(2,i));
	int size=200;
	/*
	for (int i=1;i+1<argc;i++) if (args[i][0]=='-')
		if (args[i][1]=='g')
			graph_file=args[i+1];
		else if (args[i][1]=='c')
			community_file=args[i+1];
		else if (args[i][1]=='a')
			area.push_back(atoi(args[i+1]));
		else if (args[i][1]=='k')
			size=atoi(args[i+1]);
	*/
	load_graph(graph_file);
	load_area(community_file); 
	vector<VI> kernels;
	for (int i=0;i<SIZE(area);i++) kernels.push_back(get_community_kernel(area[i]));
	if (SIZE(kernels)<2)
	{
		printf("ERROR : we should have at least 2 communities.");
		return 0;
	}
	for (int i=0;i<SIZE(kernels);i++) if (SIZE(kernels[i])==0)
	{
		printf("Community %d is too small.",i);
		return 0;
	}
	calculate_pagerank(kernels[target_group]);
	vector< pair<double,int> > mycandidates;
	build_network(kernels,mycandidates,candidate_file);
	for (int i=0;i<SIZE(mycandidates);i++)
	{
		mycandidates[i].first=pagerank[mycandidates[i].second];
	}
	sort(mycandidates.begin(),mycandidates.end());
	std::reverse(mycandidates.begin(),mycandidates.end());
	for (int i=0;i<size;i++) fprintf(fp,"%d\n",mycandidates[i].second);
	std::fclose(fp);
	return 0;
}

