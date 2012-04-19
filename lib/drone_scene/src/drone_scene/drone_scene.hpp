#ifndef __DRN_SCENE_SCENE_HPP__
#define __DRN_SCENE_SCENE_HPP__

#include <drone/drn_types.h>
#include <drone/drn_reader.h>

#define DRN_SCENE_CACHE_DESCRIPTION "DRNScene V0.0"

#define DRN_SCENE_NONE 0x0
#define DRN_SCENE_MESH 0x1
#define DRN_SCENE_CAMERA 0x2
#define DRN_SCENE_SPOTLIGHT 0x4

#define DRN_SCENE_CHUNK_TYPE_TAG "scene_chunk_type"
#define DRN_SCENE_CHUNK_TAG_TYPE_VALUE "scene"

namespace drn_scene
{
	struct TimeRangeContainer
	{
		uint32_t numFrames;
		uint32_t startFrame;
		uint32_t endFrame;
		uint32_t subFrames;
	};

	struct SpotlightDynamicDataContainer
	{
		float wPosition[4];
		float wDirection[4];
		float color[4];
		float intensity;
		float angle;
		enum DecayRate { NO_DECAY, LINEAR, QUADRATIC };
		enum ShadowType { NO_SHADOW, HARD, SOFT };
		enum DecayRate decayRate;
		enum ShadowType shadow;
	};

	struct SpotlightContainer
	{
		drn_chunk_id_t dynamicData;
	};

	struct Spotlight
	{
		uint64_t dagNodeId;
		const SpotlightDynamicDataContainer * dynamicData;
		const SpotlightContainer * d;
	};

	struct TextureContainer
	{
		int width;
		int height;
		int ncomp;
		int bytes;
		drn_chunk_id_t data;
	};

	struct Texture
	{
		int width;
		int height;
		int ncomp;
		int bytes;
		const unsigned char * data;
		const TextureContainer * d;
	};

	struct MaterialContainer
	{
		drn_chunk_id_t name;
		drn_chunk_id_t diffuseTexture;
		float specularPower;
		float diffuseColor[4];
		float specularColor[4];
		float ambientColor[4];
		float transparency[4];
	};

	struct Material
	{
		const char * name;
		Texture * diffuseTexture;
		float specularPower;
		float diffuseColor[4];
		float specularColor[4];
		float ambientColor[4];
		float transparency[4];
		const MaterialContainer * d;
	};

	struct DagNodeContainer
	{
		uint32_t type;
		uint32_t materialId;
		drn_chunk_id_t container;
		drn_chunk_id_t fullPath;
	};

	struct DagNode
	{
		uint32_t type;
		uint32_t materialId;
		const void * container;
		const char * fullPath;
		const DagNodeContainer * d;
	};

	struct DagNodeStateContainer
	{
		float aabb[8];
		float otwTransform[16];
	};

	struct UVSetContainer
	{
		drn_chunk_id_t name;
		drn_chunk_id_t uvCountPerFace;
		drn_chunk_id_t uvListPerFace;
		drn_chunk_id_t u;
		drn_chunk_id_t v;
	};

	struct UVSet
	{
		const char * name;
		const int * uvCountPerFace;
		const int * uvListPerFace;
		const float * u;
		const float * v;
		const UVSetContainer * d;
	};

	struct MeshDynamicDataContainer
	{
		drn_chunk_id_t vertices;
		drn_chunk_id_t normals;
		drn_chunk_id_t hwVertices;
		drn_chunk_id_t hwNormals;
	};

	struct MeshDynamicData
	{
		const float * vertices;
		const float * normals;
		const float * hwVertices;
		const float * hwNormals;
		const MeshDynamicDataContainer * d;
	};

	struct MeshContainer
	{
		uint32_t numPolygons;
		uint32_t numTriangles;
		uint32_t numVertices;
		uint32_t numHwVertices;
		uint32_t numNormals;
		uint32_t numUVSets;
		uint32_t defaultUVSet;
		drn_chunk_id_t vertexCountPerFace;
		drn_chunk_id_t vertexListPerFace;
		drn_chunk_id_t normalCountPerFace;
		drn_chunk_id_t normalListPerFace;
		drn_chunk_id_t triangleList;
		drn_chunk_id_t uvSets;
		drn_chunk_id_t hwUVs;
		drn_chunk_id_t dynamicData;
	};

	struct Mesh
	{
		uint64_t dagNodeId;
		uint32_t numPolygons;
		uint32_t numTriangles;
		uint32_t numVertices;
		uint32_t numHwVertices;
		uint32_t numNormals;
		uint32_t numUVSets;
		uint32_t defaultUVSet;
		const int * vertexCountPerFace;
		const int * vertexListPerFace;
		const int * normalCountPerFace;
		const int * normalListPerFace;
		const int * triangleList;
		UVSet * uvSets;
		const float * hwUVs;
		MeshDynamicData * dynamicData;
		const MeshContainer * d;
	};

	struct CameraDynamicDataContainer
	{
		float eye[4];
		float center[4];
		float up[4];
		float right[4];
		float direction[4];
	};

	struct CameraContainer
	{
		drn_chunk_id_t dynamicData;
	};

	struct Camera
	{
		const CameraDynamicDataContainer * dynamicData;
		const CameraContainer * d;
	};

	struct SceneContainer
	{
		uint64_t numDagNodes;
		drn_chunk_id_t dagNodes;
		drn_chunk_id_t dagStates;
		uint64_t numMaterials;
		drn_chunk_id_t materials;
		TimeRangeContainer timeRange;
	};

	struct Scene
	{
		TimeRangeContainer timeRange;
		uint64_t numDagNodes;
		DagNode * dagNodes;
		const DagNodeStateContainer * dagNodeStates;
		uint64_t numMaterials;
		Material * materials;
		uint64_t numMeshes;
		Mesh * meshes;
		uint64_t numCameras;
		Camera * cameras;
		uint64_t numSpotlights;
		Spotlight * spotlights;
		const SceneContainer * d;
	};

	double frameToTime(uint32_t frame, uint32_t startFrame, uint32_t endFrame, uint32_t subFrames);
	uint32_t timeToFrame(double time, uint32_t startFrame, uint32_t endFrame, uint32_t subFrames);

	int32_t resolveScene(drn_t * cache, Scene * scene, const void * sceneContainer);
	int32_t releaseScene(Scene * scene);


} // namespace scene

#endif // __DRN_SCENE_SCENE_HPP__
