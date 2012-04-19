#include "drone_scene.hpp"
#include <cmath>

#define DRN_DBG_LVL 1
#if DRN_DBG_LVL == 0
#define DRN_DBG_LVL1(EXP)
#define DRN_DBG_LVL2(EXP)
#elif DRN_DBG_LVL == 1
#include <iostream>
#define DRN_DBG_LVL1(EXP) EXP
#define DRN_DBG_LVL2(EXP)
#elif DRN_DBG_LVL > 1
#include <iostream>
#define DRN_DBG_LVL1(EXP) EXP
#define DRN_DBG_LVL2(EXP) EXP
#endif


namespace drn_scene
{
	double frameToTime(uint32_t frame, uint32_t startFrame, uint32_t endFrame, uint32_t subFrames)
	{
		return (frame + 1.0 + subFrames) / (double) (subFrames + 1.0) + startFrame - 1.0;
	}
	uint32_t timeToFrame(double time, uint32_t startFrame, uint32_t endFrame, uint32_t subFrames)
	{
		int maxFrame = (endFrame-startFrame+1)*(subFrames+1)-(1+subFrames);
		double sf = subFrames; double st = startFrame;
		int currentFrame =	(int) floor((time-st+1.0)*(sf+1.0)-(1.0+sf));
		if (currentFrame < 0) currentFrame = 0;
		else if (currentFrame > maxFrame) { currentFrame = currentFrame%(maxFrame+1); }
		return currentFrame;
	}

	int32_t resolveScene(drn_t * cache, Scene * scene, const void * sceneContainer)
	{
		scene->d = (const SceneContainer *) sceneContainer;
		scene->timeRange = scene->d->timeRange;
		DRN_DBG_LVL1(std::cout<<"resolveScene():numDagNodes:"<<scene->d->numDagNodes<<std::endl;);
		DRN_DBG_LVL1(std::cout<<"resolveScene():numMaterials:"<<scene->d->numMaterials<<std::endl;);
		scene->numDagNodes = scene->d->numDagNodes;
		scene->dagNodes = new DagNode[scene->d->numDagNodes];
		const DagNodeContainer * dnContainers = (DagNodeContainer *) drn_get_chunk(cache, scene->d->dagNodes);
		scene->numMeshes = 0;
		scene->numCameras = 0;
		scene->numSpotlights = 0;
		for (uint64_t i =0; i < scene->d->numDagNodes; ++i)
		{
			scene->dagNodes[i].d = dnContainers + i;
			scene->dagNodes[i].type = dnContainers[i].type;
			DRN_DBG_LVL1(std::cout<<"resolveScene():dagNode:"<<i<<":type:"<<dnContainers[i].type<<":mid:"<< dnContainers[i].materialId<< std::endl;);
			if (dnContainers[i].type == DRN_SCENE_MESH)
				++scene->numMeshes;
			else if (dnContainers[i].type == DRN_SCENE_CAMERA)
				++scene->numCameras;
			else if (dnContainers[i].type == DRN_SCENE_SPOTLIGHT)
				++scene->numSpotlights;
			scene->dagNodes[i].materialId = dnContainers[i].materialId;
			scene->dagNodes[i].container = drn_get_chunk(cache, dnContainers[i].container);
			scene->dagNodes[i].fullPath = (const char *) drn_get_chunk(cache, dnContainers[i].fullPath);
			DRN_DBG_LVL1(std::cout<<"resolveScene():dagNode:"<<i<<":fullPath:"<<scene->dagNodes[i].fullPath<<std::endl;);
		}
		scene->dagNodeStates = (DagNodeStateContainer *) drn_get_chunk(cache, scene->d->dagStates);
		scene->numMaterials = scene->d->numMaterials;
		scene->materials = new Material[scene->d->numMaterials];
		const MaterialContainer * mContainers = (const MaterialContainer *) drn_get_chunk(cache, scene->d->materials);
		for (uint64_t i =0; i < scene->d->numMaterials; ++i)
		{
			scene->materials[i].d = mContainers + i;
			scene->materials[i].name = (const char *) drn_get_chunk(cache, mContainers[i].name);
			if (mContainers[i].diffuseTexture)
			{
				scene->materials[i].diffuseTexture = new drn_scene::Texture;
				scene->materials[i].diffuseTexture->d  = (const TextureContainer * ) drn_get_chunk(cache, mContainers[i].diffuseTexture);
				scene->materials[i].diffuseTexture->width = scene->materials[i].diffuseTexture->d->width;
				scene->materials[i].diffuseTexture->height = scene->materials[i].diffuseTexture->d->height;
				scene->materials[i].diffuseTexture->ncomp = scene->materials[i].diffuseTexture->d->ncomp;
				scene->materials[i].diffuseTexture->bytes = scene->materials[i].diffuseTexture->d->bytes;
				scene->materials[i].diffuseTexture->data = (const unsigned char * ) drn_get_chunk(cache, scene->materials[i].diffuseTexture->d->data);
			}
			else
			{
				scene->materials[i].diffuseTexture = 0;
			}
			scene->materials[i].specularPower = mContainers[i].specularPower;
			for (uint32_t j = 0; j < 4; ++j)
				scene->materials[i].diffuseColor[j] = mContainers[i].diffuseColor[j];
			for (uint32_t j = 0; j < 4; ++j)
				scene->materials[i].specularColor[j] = mContainers[i].specularColor[j];
			for (uint32_t j = 0; j < 4; ++j)
				scene->materials[i].ambientColor[j] = mContainers[i].ambientColor[j];
			for (uint32_t j = 0; j < 4; ++j)
				scene->materials[i].transparency[j] = mContainers[i].transparency[j];
			scene->materials[i].diffuseColor[3] = 1.0 - mContainers[i].transparency[0];
		}
		scene->meshes = new Mesh[scene->numMeshes];
		uint64_t meshIdx = 0;
		scene->cameras = new Camera[scene->numCameras];
		scene->spotlights = new Spotlight[scene->numSpotlights];
		uint64_t cameraIdx = 0;
		uint64_t spotlightIdx = 0;
		for (uint64_t i =0; i < scene->d->numDagNodes; ++i)
		{
			if (dnContainers[i].type == DRN_SCENE_MESH)
			{
				const MeshContainer * d = (const MeshContainer *) drn_get_chunk(cache, dnContainers[i].container);
				scene->meshes[meshIdx].d = d;
				scene->meshes[meshIdx].dagNodeId = i;
				scene->meshes[meshIdx].numPolygons = d->numPolygons;
				scene->meshes[meshIdx].numTriangles = d->numTriangles;
				DRN_DBG_LVL1(std::cout<<"resolveScene():mesh:"<<i<<":numTriangles:"<<d->numTriangles<<std::endl;);
				scene->meshes[meshIdx].numVertices = d->numVertices;
				scene->meshes[meshIdx].numHwVertices = d->numHwVertices;
				scene->meshes[meshIdx].numNormals = d->numNormals;
				scene->meshes[meshIdx].numUVSets = d->numUVSets;
				scene->meshes[meshIdx].defaultUVSet = d->defaultUVSet;
				if (d->numPolygons)
				{
					scene->meshes[meshIdx].vertexCountPerFace = (const int *) drn_get_chunk(cache, d->vertexCountPerFace);
					scene->meshes[meshIdx].vertexListPerFace = (const int *) drn_get_chunk(cache, d->vertexListPerFace);
					scene->meshes[meshIdx].normalCountPerFace = (const int *) drn_get_chunk(cache, d->normalCountPerFace);
					scene->meshes[meshIdx].normalListPerFace = (const int *) drn_get_chunk(cache, d->normalListPerFace);
				}
				scene->meshes[meshIdx].triangleList = (const int *) drn_get_chunk(cache, d->triangleList);
				if (d->numUVSets && d->numPolygons)
				{
					scene->meshes[meshIdx].uvSets = new UVSet[d->numUVSets];
					const UVSetContainer * uvsContainers = (const UVSetContainer *) drn_get_chunk(cache, d->uvSets);
					for (uint64_t j = 0; j < d->numUVSets; ++j)
					{
						scene->meshes[meshIdx].uvSets[j].d = uvsContainers + j;
						scene->meshes[meshIdx].uvSets[j].name = (const char *) drn_get_chunk(cache, uvsContainers[j].name);
						scene->meshes[meshIdx].uvSets[j].uvCountPerFace = (const int *) drn_get_chunk(cache, uvsContainers[j].uvCountPerFace);
						scene->meshes[meshIdx].uvSets[j].uvListPerFace = (const int *) drn_get_chunk(cache, uvsContainers[j].uvListPerFace);
						scene->meshes[meshIdx].uvSets[j].u = (const float *) drn_get_chunk(cache, uvsContainers[j].u);
						scene->meshes[meshIdx].uvSets[j].v = (const float *) drn_get_chunk(cache, uvsContainers[j].v);
					}
				}
				scene->meshes[meshIdx].hwUVs = (const float *) drn_get_chunk(cache, d->hwUVs);
				scene->meshes[meshIdx].dynamicData = new MeshDynamicData[scene->timeRange.numFrames];
				const MeshDynamicDataContainer * mddContainers = (const MeshDynamicDataContainer *) drn_get_chunk(cache, d->dynamicData);
				for (uint64_t j = 0; j < scene->timeRange.numFrames; ++j)
				{
					scene->meshes[meshIdx].dynamicData[j].d = mddContainers + j;
					if (d->numPolygons)
					{
						scene->meshes[meshIdx].dynamicData[j].vertices = (const float *) drn_get_chunk(cache, mddContainers[j].vertices);
						scene->meshes[meshIdx].dynamicData[j].normals = (const float *) drn_get_chunk(cache, mddContainers[j].normals);
					}
					scene->meshes[meshIdx].dynamicData[j].hwVertices = (const float *) drn_get_chunk(cache, mddContainers[j].hwVertices);
					scene->meshes[meshIdx].dynamicData[j].hwNormals = (const float *) drn_get_chunk(cache, mddContainers[j].hwNormals);
				}
				++meshIdx;
			}
			else if (dnContainers[i].type == DRN_SCENE_CAMERA)
			{
				const CameraContainer * d = (const CameraContainer *) drn_get_chunk(cache, dnContainers[i].container);
				scene->cameras[cameraIdx].dynamicData = (const CameraDynamicDataContainer *) drn_get_chunk(cache, d->dynamicData);
				++cameraIdx;
			}
			else if (dnContainers[i].type == DRN_SCENE_SPOTLIGHT)
			{
				const SpotlightContainer * d = (const SpotlightContainer *) drn_get_chunk(cache, dnContainers[i].container);
				scene->spotlights[spotlightIdx].dynamicData = (const SpotlightDynamicDataContainer *) drn_get_chunk(cache, d->dynamicData);
				scene->spotlights[spotlightIdx].dagNodeId = i;
				++spotlightIdx;
			}

		}
		return 0;
	}

	int32_t releaseScene(Scene * scene)
	{
		for (uint64_t i =0; i < scene->numMeshes; ++i)
		{
			delete[] scene->meshes[i].uvSets;
			delete[] scene->meshes[i].dynamicData;
		}
		delete[] scene->meshes;
		for (uint64_t i =0; i < scene->numMaterials; ++i)
		{
			if (scene->materials[i].diffuseTexture)
			{
				delete scene->materials[i].diffuseTexture;
			}
		}
		delete[] scene->materials;
		delete[] scene->dagNodes;
		return 0;
	}

} // namespace drn_scene
