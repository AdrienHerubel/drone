#include "MeshExport.hpp"
#include "Utils.hpp"
#include "drnTranslator.hpp"

#include <drone/drn_writer.h>
#include <drone_scene/drone_scene.hpp>

#include <maya/MStringArray.h>
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>
#include <maya/MItSelectionList.h>
#include <maya/MItDag.h>
#include <maya/MDagPath.h>
#include <maya/MFnDagNode.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnMesh.h>
#include <maya/MFnCamera.h>
#include <maya/MMatrix.h>
#include <maya/MBoundingBox.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MPointArray.h>
#include <maya/MPlug.h>
#include <maya/MFnNumericData.h>
#include <maya/MPlugArray.h>

#include <map>

#define DRNT_DBG_LVL 1
#if DRNT_DBG_LVL == 0
#define DRNT_DBG_LVL1(EXP)
#define DRNT_DBG_LVL2(EXP)
#elif DRNT_DBG_LVL == 1
#include <iostream>
#define DRNT_DBG_LVL1(EXP) EXP
#define DRNT_DBG_LVL2(EXP)
#elif DRNT_DBG_LVL > 1
#include <iostream>
#define DRNT_DBG_LVL1(EXP) EXP
#define DRNT_DBG_LVL2(EXP) EXP
#endif


bool writeMeshContainerInCache(drn_writer_t * cache,
	DRNTDagNode & node,
	drn_scene::MeshContainer * meshContainer,
	drn_scene::MeshDynamicDataContainer * dynContainers,
	unsigned int frameCount, MeshExportMode mode)
{
	int32_t status;
	status = drn_writer_add_chunk(cache, dynContainers, frameCount * sizeof (drn_scene::MeshDynamicDataContainer));
	meshContainer->dynamicData = drn_writer_get_last_chunk_id(cache);
	status = drn_writer_add_chunk(cache, meshContainer, sizeof(drn_scene::MeshContainer));
	node.container = drn_writer_get_last_chunk_id(cache);
	status = drn_writer_add_chunk(cache, node.dagPath.fullPathName().asChar(), node.dagPath.fullPathName().length() + 1);
	node.fullPath = drn_writer_get_last_chunk_id(cache);
	return status;
}


bool writeMeshStaticDataInCache(drn_writer_t * cache,
	DRNTDagNode & node,
	drn_scene::MeshContainer * meshContainer,
	DRNTHardEdge & he, MeshExportMode mode)
{
	uint32_t drnStatus = 0;
	MStatus status;
	MFnMesh mesh(node.dagPath);

	if (mode == MESH_EXPORT_FULL_TOPOLOGY)
	{
		meshContainer->numPolygons = mesh.numPolygons(&status);
		meshContainer->numVertices = mesh.numVertices(&status);
		meshContainer->numNormals = mesh.numNormals(&status);
	}
	else
	{
		meshContainer->numPolygons = 0;
		meshContainer->numVertices = 0;
		meshContainer->numNormals = 0;
	}

	MIntArray vertexCount;
	MIntArray vertexList;
	MIntArray normalCount;
	MIntArray normalList;
	MIntArray uvCounts;
	MIntArray uvIds;
	mesh.getAssignedUVs(uvCounts, uvIds);
	MIntArray triangleCount;
	MIntArray triangleList;
	MIntArray triangleNList;
	MIntArray triangleUVList;
	mesh.getVertices(vertexCount, vertexList);
	mesh.getNormalIds(normalCount, normalList);
	mesh.getTriangles(triangleCount, triangleList);
	triangleNList.setLength(triangleList.length());
	triangleUVList.setLength(triangleList.length());
	meshContainer->numTriangles = triangleList.length()  / 3;
	int * vcarray = new int[vertexCount.length()];
	int * vlarray = new int[vertexList.length()];
	int * ncarray = new int[normalCount.length()];
	int * nlarray = new int[normalList.length()];
	// Triangulation
	int poly_idx_offset = 0;
	int tri_idx_offset = 0;
	for (int i = 0; i < mesh.numPolygons(); ++i)
	{
		for (int j = 0; j < triangleCount[i]; ++j)
		{
			for(unsigned int k=0; k < 3; ++k)
			{
				int v_idx = triangleList[tri_idx_offset+j*3 + k];
				int match = -1;
				int l = 0;
				while (match < 0 && l < vertexCount[i])
				{
					if (vertexList[poly_idx_offset+l] == v_idx)
					match = l;
					++l;
				}
				triangleNList[tri_idx_offset+j*3 + k] = normalList[poly_idx_offset+match];
				int id = 0;
				if (uvIds.length() != 0)
				mesh.getPolygonUVid(i, match, id);
				triangleUVList[tri_idx_offset+j*3 + k] = id;
			}
		}
		poly_idx_offset += vertexCount[i];
		tri_idx_offset += 3 * triangleCount[i];
	}
	he.tlist.resize(triangleList.length(), -1);
	//he.itlist.resize(triangleList.length());
	he.itlist.resize(triangleList.length());
	//std::map<std::pair<int, int>, int> h;
	std::map<triplet, int> h;
	int idx = 0;
	for (int i = 0, n = triangleList.length(); i != n; ++i)
	{
		//std::pair<int, int> p = std::make_pair(triangleList[i], triangleNList[i]);
		triplet p(triangleList[i], triangleNList[i], triangleUVList[i]);
		//std::map<std::pair<int, int>, int>::const_iterator match = h.find(p);
		std::map<triplet, int>::const_iterator match = h.find(p);
		if (match != h.end())
		{
			he.tlist[i] = match->second;
			he.itlist[i] = match->first;
		}
		else
		{
			h[p] = idx;
			he.tlist[i] = idx;
			he.itlist[i] = p;
			++idx;
		}
	}
	meshContainer->numHwVertices = he.idmax = idx;
	if (mode == MESH_EXPORT_FULL_TOPOLOGY)
	{
		vertexCount.get(vcarray);
		vertexList.get(vlarray);
		normalCount.get(ncarray);
		normalList.get(nlarray);
		drnStatus = drn_writer_add_chunk(cache, vcarray, sizeof(int) * vertexCount.length());
		meshContainer->vertexCountPerFace = drn_writer_get_last_chunk_id(cache);
		drnStatus = drn_writer_add_chunk(cache, vlarray, sizeof(int) * vertexList.length());
		meshContainer->vertexListPerFace = drn_writer_get_last_chunk_id(cache);
		drnStatus = drn_writer_add_chunk(cache, ncarray, sizeof(int) * normalCount.length());
		meshContainer->normalCountPerFace = drn_writer_get_last_chunk_id(cache);
		drnStatus = drn_writer_add_chunk(cache, nlarray, sizeof(int) * normalList.length());
		meshContainer->normalListPerFace = drn_writer_get_last_chunk_id(cache);
		drnStatus = drn_writer_add_chunk(cache, & (he.tlist[0]), sizeof(int) * he.tlist.size());
		meshContainer->triangleList = drn_writer_get_last_chunk_id(cache);
		delete[] vcarray;
		delete[] vlarray;
		delete[] ncarray;
		delete[] nlarray;
	}
	else
	{
		meshContainer->numUVSets = 0;
		drnStatus = drn_writer_add_chunk(cache, & (he.tlist[0]), sizeof(int) * he.tlist.size());
		meshContainer->triangleList = drn_writer_get_last_chunk_id(cache);
		meshContainer->defaultUVSet = 0;
	}
	DRNT_DBG_LVL2(std::cout << "trianglelist" << triangleList << std::endl;);
	int numUVSets = meshContainer->numUVSets = mesh.numUVSets();
	drn_scene::UVSetContainer * uvSets = new drn_scene::UVSetContainer[numUVSets];
	MStringArray uvSetNames;
	mesh.getUVSetNames(uvSetNames);
	std::vector<float> hwUVs(he.idmax*2, 0.0);
	for (int i = 0; i < numUVSets; ++i)
	{
		MFloatArray u;
		MFloatArray v;
		MIntArray uvCounts;
		MIntArray uvIds;
		mesh.getUVs(u, v, &uvSetNames[i]);
		mesh.getAssignedUVs(uvCounts, uvIds, &uvSetNames[i]);
		if (uvSetNames[i] == mesh.currentUVSetName())
		{
			meshContainer->defaultUVSet = i;
			if (uvIds.length())
			{
				for (int j = 0, n = he.tlist.size(); j != n; ++j)
				{
					hwUVs[he.tlist[j]*2] = u[he.itlist[j].third];
					hwUVs[he.tlist[j]*2+1] = v[he.itlist[j].third];
				}
			}
			else
			{
				for (int j = 0, n = he.tlist.size(); j != n; ++j)
				{
					hwUVs[he.tlist[j]*2] = 0;
					hwUVs[he.tlist[j]*2+1] = 0;
				}
			}
		}
		if (mode == MESH_EXPORT_FULL_TOPOLOGY)
		{
			float * uArray = new float[u.length()];
			float * vArray = new float[v.length()];
			int * uvCountsArray = new int[uvCounts.length()];
			int * uvIdsArray = new int[uvIds.length()];
			u.get(uArray);
			v.get(vArray);
			uvCounts.get(uvCountsArray);
			uvIds.get(uvIdsArray);
			drnStatus = drn_writer_add_chunk(cache, uvSetNames[i].asChar(), uvSetNames[i].length() + 1);
			uvSets[i].name = drn_writer_get_last_chunk_id(cache);
			drnStatus = drn_writer_add_chunk(cache, uvCountsArray, sizeof(int) * uvCounts.length());
			uvSets[i].uvCountPerFace = drn_writer_get_last_chunk_id(cache);
			drnStatus = drn_writer_add_chunk(cache, uvIdsArray, sizeof(int) * uvIds.length());
			uvSets[i].uvListPerFace = drn_writer_get_last_chunk_id(cache);
			drnStatus = drn_writer_add_chunk(cache, uArray, sizeof(float) * u.length());
			uvSets[i].u = drn_writer_get_last_chunk_id(cache);
			drnStatus = drn_writer_add_chunk(cache, vArray, sizeof(float) * v.length());
			uvSets[i].v = drn_writer_get_last_chunk_id(cache);
			delete[] uArray;
			delete[] vArray;
			delete[] uvCountsArray;
			delete[] uvIdsArray;
		}
	}
	drnStatus = drn_writer_add_chunk(cache, &hwUVs[0], sizeof(float) * hwUVs.size());
	meshContainer->hwUVs = drn_writer_get_last_chunk_id(cache);
	if (mode == MESH_EXPORT_FULL_TOPOLOGY)
	{
		drnStatus =	drn_writer_add_chunk(cache, uvSets, sizeof(drn_scene::UVSetContainer) * numUVSets);
		meshContainer->uvSets = drn_writer_get_last_chunk_id(cache);
	}
	delete[] uvSets;
	return drnStatus;
}

bool writeMeshDynamicDataInCache(drn_writer_t * cache,
	DRNTDagNode & node,
	drn_scene::MeshContainer * meshContainer,
	DRNTHardEdge & he,
	drn_scene::MeshDynamicDataContainer * meshDynContainer,
	unsigned int frame, MeshExportMode mode)
{
	MStatus status;
	int32_t drnStatus;
	MFnMesh mesh(node.dagPath);
	MPointArray vertices;
	MFloatVectorArray normals;
	mesh.getPoints(vertices);
	mesh.getNormals(normals);
	std::vector<float> hwVertices(he.idmax*3);
	std::vector<float> hwNormals(he.idmax*3);

	for (int i = 0, n = he.tlist.size(); i != n; ++i)
	{
		hwVertices[he.tlist[i]*3] = vertices[he.itlist[i].first].x;
		hwVertices[he.tlist[i]*3+1] = vertices[he.itlist[i].first].y;
		hwVertices[he.tlist[i]*3+2] = vertices[he.itlist[i].first].z;
		hwNormals[he.tlist[i]*3] = normals[he.itlist[i].second].x;
		hwNormals[he.tlist[i]*3+1] = normals[he.itlist[i].second].y;
		hwNormals[he.tlist[i]*3+2] = normals[he.itlist[i].second].z;
	}

	DRNT_DBG_LVL2(std::cout<<"verticesB:"<<vertices.length()<<std::endl;);
	DRNT_DBG_LVL2(std::cout<<"normalsB:"<<normals.length()<<std::endl;);
	DRNT_DBG_LVL2(std::cout<<"hwVerticesB:"<<hwVertices.size()/3<<std::endl;);
	DRNT_DBG_LVL2(std::cout<<"hwNormalsB:"<<hwVertices.size()/3<<std::endl;);
	DRNT_DBG_LVL2(std::cout<<"vertices:"<<vertices.length()<<std::endl;);
	DRNT_DBG_LVL2(std::cout<<"normals:"<<normals.length()<<std::endl;);
	DRNT_DBG_LVL2(std::cout<<"hwVertices:"<<hwVertices.size()/3<<std::endl;);
	DRNT_DBG_LVL2(
		for (unsigned int i = 0; i < hwVertices.size()/3; i++)
		{
			std::cout << i << "["<<hwVertices[i*3]<<";"<<hwVertices[i*3+1]<<";"<<hwVertices[i*3+2] <<"] ";
			std::cout<<std::endl;
		}
		);
	DRNT_DBG_LVL2(std::cout<<"hwNormals:"<<hwNormals.size()/3<<std::endl;);
	if (mode == MESH_EXPORT_FULL_TOPOLOGY)
	{
		drnStatus = drn_writer_add_chunk(cache, mesh.getRawPoints(&status), sizeof(float) * mesh.numVertices()*3);
		meshDynContainer[frame].vertices = drn_writer_get_last_chunk_id(cache);
		drnStatus = drn_writer_add_chunk(cache, mesh.getRawNormals(&status), sizeof(float) * mesh.numNormals()*3);
		meshDynContainer[frame].normals = drn_writer_get_last_chunk_id(cache);
	}
	drnStatus = drn_writer_add_chunk(cache, &hwVertices[0], sizeof(float) * hwVertices.size());
	meshDynContainer[frame].hwVertices = drn_writer_get_last_chunk_id(cache);
	drnStatus = drn_writer_add_chunk(cache, &hwNormals[0], sizeof(float) * hwNormals.size());
	meshDynContainer[frame].hwNormals = drn_writer_get_last_chunk_id(cache);

	return drnStatus;
}
