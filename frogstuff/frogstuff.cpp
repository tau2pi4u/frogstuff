// frogstuff.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <vector>
#include <map>
#include <assert.h>

#define SVG_RECT_WIDTH 175
#define SVG_RECT_HEIGHT 50
#define SVG_LAYER_SPACING 225
#define SVG_Y_SPACING 75
#define SVG_X_OFFET 10
#define SVG_Y_OFFSET 10
#define FONT_SIZE 12
#define TEXT_OFFSET 10

struct NODE
{
	~NODE()
	{
		for (auto & child : children)
		{
			delete child;
		}
	}
	NODE * parent;
	std::vector<NODE*> children;
	unsigned int position;
	double probability;
	unsigned int depth;
};

struct LAYER
{
	unsigned int depth;
	unsigned int nodeCount = 0;
	unsigned int terminatingCount = 0;
};

void ProcessNode(NODE * node, std::vector<NODE*> & baseList, std::vector<NODE*> & toProcess, unsigned int distance, std::map<unsigned int, LAYER> & depthMap)
{
	unsigned int remainingDist = distance - node->position;
	node->children.reserve(remainingDist);
	for (unsigned int i = 0; i < remainingDist; i++)
	{
		NODE * child = new NODE();
		child->parent = node;
		child->position = node->position + i + 1; // must move at least one
		child->probability = node->probability * (1 / static_cast<double>(remainingDist));
		child->depth = node->depth + 1;
		node->children.push_back(child);
		if (depthMap.count(child->depth) == 0)
		{
			depthMap[child->depth].depth = child->depth;
		}
		if (child->position != distance)
		{
			toProcess.push_back(child);
		}
		else
		{
			baseList.push_back(child);
			depthMap[child->depth].terminatingCount++;
		}
		depthMap[child->depth].nodeCount++;
	}
}

unsigned int DrawSVG(FILE * svgFile, NODE * node, unsigned int y, unsigned int depth, unsigned int EndNodeSize)
{
	if (node->depth == 0)
	{
		fprintf(svgFile, "<svg width=\"%u\" height=\"%u\" xmlns=\"http://www.w3.org/2000/svg\">\n", (SVG_LAYER_SPACING * depth) + (SVG_X_OFFET * (1 + depth)), SVG_Y_SPACING * EndNodeSize);
	}
	const char * nodeCol = node->children.empty() ? "aquamarine" : "dodgerblue";
	unsigned int x = (SVG_LAYER_SPACING * node->depth) + (SVG_X_OFFET * (1 + node->depth));
	fprintf(svgFile, "<g>\n");
	fprintf(svgFile, "<rect width=\"%u\" height = \"%u\" x = \"%u\" y = \"%u\" fill=\"%s\"/>\n", SVG_RECT_WIDTH, SVG_RECT_HEIGHT, x, y, nodeCol);
	fprintf(svgFile, "<text x=\"%u\" y=\"%u\" font-family=\"Verdana\" font-size=\"%u\" fill=\"black\">Pos: %u, Prob: %.3e</text>", x + TEXT_OFFSET, y + (SVG_RECT_HEIGHT / 2), FONT_SIZE, node->position, node->probability);
	fprintf(svgFile, "</g>\n");
	unsigned int totalHeight = node->children.empty() ? SVG_Y_SPACING : 0;
	unsigned int childY = y;
	for (auto itr = node->children.rbegin(); itr != node->children.rend(); itr++)
	{
		auto & child = *itr;
		auto height = DrawSVG(svgFile, child, childY, depth, EndNodeSize);
		unsigned int childX = (SVG_LAYER_SPACING * child->depth) + (SVG_X_OFFET * (1 + child->depth));
		unsigned int childYMid = childY + (SVG_RECT_HEIGHT / 2);
		fprintf(svgFile, "<line x1=\"%u\" y1=\"%u\" x2=\"%u\" y2=\"%u\" stroke=\"black\"/>\n", x + SVG_RECT_WIDTH, y + (SVG_RECT_HEIGHT / 2), childX, childYMid);
		totalHeight += height;
		childY += height;
		
	}
	if (node->depth == 0)
	{
		fprintf(svgFile, "</svg>");
	}
	return totalHeight;
}

int main(int argc, char * argv[])
{
	if (argc != 3)
	{
		printf("Please use as frogstuff.exe MIN_DEPTH MAX_DEPTH\n");
		return -1;
	}
	unsigned int min = atoi(argv[1]);
	unsigned int max = atoi(argv[2]);
	if (max < min)
	{
		printf("Max (%u) < min (%u)\n", max, min);
		return -1;
	}
	std::vector<std::map<unsigned int, LAYER>> depthMaps;
	depthMaps.reserve(max - min);
	for (unsigned int i = min; i <= max; i++)
	{
		NODE * baseNode = new NODE{ NULL, {}, 0, 1, 0 };
		std::vector<NODE*> toProcess = { baseNode };
		std::vector<NODE*> toProcessCpy;
		std::vector<NODE*> baseList;
		std::map<unsigned int, LAYER> depthMap;
		while (!toProcess.empty())
		{
			toProcessCpy.swap(toProcess);
			toProcess.clear();
			for (auto & node : toProcessCpy)
			{
				ProcessNode(node, baseList, toProcess, i, depthMap);
			}
		}
		FILE * svgFile = NULL;
		static char buf[256];
		sprintf_s(buf, 256, "%u_distance_tree.svg", i);
		fopen_s(&svgFile, buf, "w");
		DrawSVG(svgFile, baseNode, SVG_Y_OFFSET, i + 1, static_cast<unsigned int>(baseList.size()));
		fclose(svgFile);
		double total = 0;
		double weightedSum = 0;
		for (auto & node : baseList)
		{
			total 
				+= node->probability;
			weightedSum += static_cast<double>(node->depth) * node->probability;
		}
		printf("Size: %u\nTotal probability: %lf\nWeighted sum: %lf\n", i, total, weightedSum);
		delete baseNode;
		depthMaps.push_back(depthMap);
		for (auto & pair : depthMap)
		{
			printf("Depth: %u, Node Count: %u, Terminating Count: %u\n", pair.first, pair.second.nodeCount, pair.second.terminatingCount);
		}
		printf("\n\n");
	}
}