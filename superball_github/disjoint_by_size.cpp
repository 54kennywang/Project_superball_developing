/*
Union by Size implementation of Disjoint Sets.
*/

#include <vector>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include "disjoint.h"
using namespace std;

DisjointSetBySize::DisjointSetBySize(int nelements)
{
	links.resize(nelements, -1);
	sizes.resize(nelements, 1);
}

int DisjointSetBySize::Union(int s1, int s2)
{
	int p, c;

	if (links[s1] != -1 || links[s2] != -1) {
		cerr << "Must call union on a set, and not just an element.\n";
		exit(1);
	}

	if (sizes[s1] > sizes[s2]) {
		p = s1;
		c = s2;
	} else {
		p = s2;
		c = s1;
	}

	links[c] = p;
	sizes[p] += sizes[c];
	return p;
}

int DisjointSetBySize::Find(int element)
{
	while (links[element] != -1) element = links[element];
	return element;
}

int DisjointSetBySize::Size(int index){
	return sizes[Find(index)];
}

void DisjointSetBySize::Print()
{
	int i;

	printf("\n");
	printf("Node:  ");
	for (i = 0; i < links.size(); i++) printf("%3d", i);  
	printf("\n");

	printf("Links: ");
	for (i = 0; i < links.size(); i++) printf("%3d", links[i]);  
	printf("\n");

	printf("Sizes: ");
	for (i = 0; i < links.size(); i++) printf("%3d", sizes[i]);  
	printf("\n\n");
}

