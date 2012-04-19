#include "drnTranslator.hpp"
#include "Utils.hpp"
#include "MeshExport.hpp"
#include "CameraExport.hpp"
#include "SpotlightExport.hpp"

#include <drone/drn_writer.h>
#include <drone_scene/drone_scene.hpp>
#include <stbi/stb_image.h>
#include <maya/MStringArray.h>
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>
#include <maya/MItSelectionList.h>
#include <maya/MItDag.h>
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
#include <vector>
#include <map>

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


struct DRNTParameters
{
	int startFrame;
	int endFrame;
	unsigned int subFrames;
	MeshExportMode meshExportMode;
};

struct DRNTDagTable
{
	std::vector<DRNTDagNode> nodes;
};


DRNTParameters defaultParameters();
bool parseOptions(const MString & optionsString, DRNTParameters & parameters);
bool buildDagPathArrayFromSelection(const MSelectionList & selection,
									MDagPathArray & dagPathArray);
bool buildDagTableFromSelection(const MSelectionList & selection,
								DRNTDagTable & dagTable);
bool writeDagTableInCache(drn_writer_t * cache, const DRNTDagTable & dagTable, drn_scene::SceneContainer & scene);
bool writeMaterialsInCache(drn_writer_t * cache, DRNTDagTable & dagTable, drn_scene::SceneContainer & scene);
bool buildDagStates(const DRNTDagTable & dagTable, drn_scene::DagNodeStateContainer * states, uint32_t frame);

DRNTranslator::DRNTranslator()
{}

DRNTranslator::~DRNTranslator()
{}

MStatus DRNTranslator::reader ( const MFileObject& file,
							   const MString& optionsString,
							   FileAccessMode mode)
{
	return MS::kFailure;
}

MStatus DRNTranslator::writer ( const MFileObject& file,
							   const MString& optionsString,
							   FileAccessMode mode )
{
	MStatus status;
	if( ( mode == MPxFileTranslator::kExportAccessMode ) ||
		( mode == MPxFileTranslator::kSaveAccessMode ) )
	{
		return MS::kFailure;
	}
	// Parse parameters
	DRNTParameters params;
	params = defaultParameters();
	if (!parseOptions(optionsString, params))
	{
		MString errMsg = "drnTranslator : Impossible to parse options";
		status.perror(errMsg);
		MGlobal::displayWarning(errMsg);
		return MS::kFailure;
	}
	// Build selection
	MSelectionList selection;
	MGlobal::getActiveSelectionList( selection );
	MItSelectionList selIt( selection );
	if (selIt.isDone())
	{
		MString errMsg = "drnTranslator : Nothing selected";
		status.perror(errMsg);
		MGlobal::displayWarning(errMsg);
		return MS::kFailure;
	}
	// Open cache writer
	drn_writer_t cacheWriter;
	int32_t drnStatus = drn_open_writer(&cacheWriter, file.fullName().asChar(), DRN_SCENE_CACHE_DESCRIPTION);
	if (drnStatus)
	{
		MString errMsg = "drnTranslator : Impossible to write to file";
		status.perror(errMsg);
		MGlobal::displayWarning(errMsg);
		return MS::kFailure;
	}
	// Prepare tags
	drn_writer_create_map(&cacheWriter, DRN_SCENE_CHUNK_TYPE_TAG);
	drn_map_id_t chunkTypeTagId = drn_writer_get_map_id(&cacheWriter, DRN_SCENE_CHUNK_TYPE_TAG);

	// Prepare scene containers
	drn_scene::SceneContainer scene;
	DRNTDagTable dagTable;
	buildDagTableFromSelection(selection, dagTable);
	scene.numDagNodes = dagTable.nodes.size();

	// Count frames
	unsigned int frameCount = (params.endFrame - params.startFrame + 1 ) * (1 + params.subFrames);
	scene.timeRange.startFrame = params.startFrame;
	scene.timeRange.endFrame = params.endFrame;
	scene.timeRange.subFrames = params.subFrames;
	scene.timeRange.numFrames = frameCount;
	DRN_DBG_LVL2(std::cout<<"writer():frameCount:"<<frameCount<<std::endl;);

	// Count meshes and cameras
	unsigned int meshCount = 0;
	unsigned int cameraCount = 0;
	unsigned int spotlightCount = 0;
	std::vector<DRNTDagNode *> meshNodes;
	std::vector<DRNTDagNode *> cameraNodes;
	std::vector<DRNTDagNode *> spotlightNodes;
	for (unsigned int i = 0; i < dagTable.nodes.size(); ++i)
	{
		if (dagTable.nodes[i].type == DRN_SCENE_MESH )
		{
			meshNodes.push_back(& dagTable.nodes[i]);
			meshCount++;
		}
		else if (dagTable.nodes[i].type == DRN_SCENE_CAMERA )
		{
			cameraNodes.push_back(& dagTable.nodes[i]);
			cameraCount++;
		}
		else if (dagTable.nodes[i].type == DRN_SCENE_SPOTLIGHT )
		{
			spotlightNodes.push_back(& dagTable.nodes[i]);
			spotlightCount++;
		}
	}
	// Allocate mesh containers
	drn_scene::MeshContainer * meshContainers = new drn_scene::MeshContainer[meshCount];
	DRNTHardEdge * hePerMesh = new DRNTHardEdge[meshCount];
	drn_scene::MeshDynamicDataContainer ** meshDynContainers = new drn_scene::MeshDynamicDataContainer*[meshCount];
	for (unsigned int i = 0; i < meshCount; ++i)
	{
		meshDynContainers[i] = new drn_scene::MeshDynamicDataContainer[frameCount];
	}
	// Allocate Cameras containers
	drn_scene::CameraContainer * cameraContainers = new drn_scene::CameraContainer[cameraCount];
	drn_scene::CameraDynamicDataContainer ** cameraDynContainers = new drn_scene::CameraDynamicDataContainer*[cameraCount];
	for (unsigned int i = 0; i < cameraCount; ++i)
	{
		cameraDynContainers[i] = new drn_scene::CameraDynamicDataContainer[frameCount];
	}
	// Allocate Spotlights containers
	drn_scene::SpotlightContainer * spotlightContainers = new drn_scene::SpotlightContainer[spotlightCount];
	drn_scene::SpotlightDynamicDataContainer ** spotlightDynContainers = new drn_scene::SpotlightDynamicDataContainer*[spotlightCount];
	for (unsigned int i = 0; i < spotlightCount; ++i)
	{
		spotlightDynContainers[i] = new drn_scene::SpotlightDynamicDataContainer[frameCount];
	}

	// Allocate dag node states
	drn_scene::DagNodeStateContainer * states = new drn_scene::DagNodeStateContainer[dagTable.nodes.size() * frameCount];
	// Loop over frames to write
	for (unsigned int i = 0; i < frameCount; ++i)
	{
		double time = drn_scene::frameToTime(i, params.startFrame, params.endFrame, params.subFrames);
		MGlobal::viewFrame(time);
		DRN_DBG_LVL2(std::cout<<"writer():time:"<<time<<std::endl;);
		// Build states
		buildDagStates(dagTable, states, i);
		// Loop over all meshes
		for (unsigned int j = 0; j < meshCount; ++j)
		{
			// Write static data only for frame 0
			if (i == 0)
			{
				writeMeshStaticDataInCache(&cacheWriter, *meshNodes[j], &meshContainers[j], hePerMesh[j], params.meshExportMode);
			}
			writeMeshDynamicDataInCache(&cacheWriter, *meshNodes[j], &meshContainers[j], hePerMesh[j], meshDynContainers[j], i, params.meshExportMode);
		}
		// Loop over all Cameras
		for (unsigned int j = 0; j < cameraCount; ++j)
		{
			// Write static data only for frame 0
			if (i == 0)
			{
				writeCameraStaticDataInCache(&cacheWriter, *cameraNodes[j], &cameraContainers[j]);
			}
			writeCameraDynamicDataInCache(&cacheWriter, *cameraNodes[j], &cameraContainers[j], cameraDynContainers[j], i);
		}
		// Loop over all Spotlights
		for (unsigned int j = 0; j < spotlightCount; ++j)
		{
			// Write static data only for frame 0
			if (i == 0)
			{
				writeSpotlightStaticDataInCache(&cacheWriter, *spotlightNodes[j], &spotlightContainers[j]);
			}
			writeSpotlightDynamicDataInCache(&cacheWriter, *spotlightNodes[j], &spotlightContainers[j], spotlightDynContainers[j], i);
		}
	}

	// Write mesh containers
	for (unsigned int i = 0; i < meshCount; ++i)
	{
		writeMeshContainerInCache(& cacheWriter, *meshNodes[i], &meshContainers[i], meshDynContainers[i], frameCount, params.meshExportMode);
	}
	// Write camera containers
	for (unsigned int i = 0; i < cameraCount; ++i)
	{
		writeCameraContainerInCache(& cacheWriter, *cameraNodes[i], &cameraContainers[i], cameraDynContainers[i], frameCount);
	}
	// Write spotlight containers
	for (unsigned int i = 0; i < spotlightCount; ++i)
	{
		writeSpotlightContainerInCache(& cacheWriter, *spotlightNodes[i], &spotlightContainers[i], spotlightDynContainers[i], frameCount);
	}

	// Write base scene objects
	drnStatus = drn_writer_add_chunk(&cacheWriter, states, sizeof(drn_scene::DagNodeStateContainer) * frameCount * scene.numDagNodes);
	scene.dagStates = drn_writer_get_last_chunk_id(&cacheWriter);
	writeMaterialsInCache(&cacheWriter, dagTable, scene);
	writeDagTableInCache(&cacheWriter, dagTable, scene);
	drnStatus = drn_writer_add_chunk(&cacheWriter, &scene, sizeof(drn_scene::SceneContainer));
	drn_chunk_id_t sceneEntryId = drn_writer_get_last_chunk_id(&cacheWriter);
	const char * tag_value = DRN_SCENE_CHUNK_TAG_TYPE_VALUE;
	drn_writer_map_chunk(&cacheWriter, sceneEntryId, 1, &chunkTypeTagId, &tag_value);
	drnStatus = drn_close_writer(&cacheWriter);

	// Free memory
	delete[] states;
	for (unsigned int i = 0; i < meshCount; ++i)
		delete[] meshDynContainers[i];
	delete[] meshDynContainers;
	delete[] hePerMesh;
	delete[] meshContainers;

	return MS::kSuccess;
}

bool buildDagPathArrayFromSelection(const MSelectionList & selection,
									MDagPathArray & dagPathArray)
{
	MStatus status;
	// Selection list loop
	for (MItSelectionList iter( selection ) ; !iter.isDone(); iter.next() )
	{
		MItDag dagIterator( MItDag::kDepthFirst, MFn::kInvalid, &status);
		MDagPath objectPath;
		status = iter.getDagPath(objectPath);
		status = dagIterator.reset(objectPath.node(),
								   MItDag::kDepthFirst, MFn::kInvalid);
		do
		{
			MDagPath dagPath;
			MObject component = MObject::kNullObj;
			status = dagIterator.getPath(dagPath);
			MFnDagNode dagNode( dagPath, &status );
			if (dagNode.isIntermediateObject())
			{

			}
			else if (dagPath.hasFn(MFn::kMesh))
			{
				if(!dagPath.hasFn(MFn::kTransform))
				{
					dagPathArray.append(dagPath);
				}
			}
			else if (dagPath.hasFn(MFn::kCamera))
			{
				if(!dagPath.hasFn(MFn::kTransform))
				{
					dagPathArray.append(dagPath);
				}
			}
			else if (dagPath.hasFn(MFn::kSpotLight))
			{
				if(!dagPath.hasFn(MFn::kTransform))
				{
					dagPathArray.append(dagPath);
				}
			}
			dagIterator.next();
		} while (!dagIterator.isDone());
	}
	return true;
}

bool buildDagTableFromSelection(const MSelectionList & selection, DRNTDagTable & dagTable)
{
	MDagPathArray dagPathArray;
	buildDagPathArrayFromSelection(selection, dagPathArray);
	dagTable.nodes.resize(dagPathArray.length());
	for (unsigned int i = 0; i < dagTable.nodes.size(); ++i)
	{
		DRN_DBG_LVL1(std::cout << "node:"<<i<<":fullpath:"<<dagPathArray[i].fullPathName()<<std::endl;);
		dagTable.nodes[i].dagId = i;
		if (dagPathArray[i].hasFn(MFn::kMesh) &&  !dagPathArray[i].hasFn(MFn::kTransform))
		{
			DRN_DBG_LVL1(std::cout << "node:"<<i<<":type:mesh"<<std::endl;;);
			dagTable.nodes[i].type = DRN_SCENE_MESH;
		}
		else if (dagPathArray[i].hasFn(MFn::kCamera) &&  !dagPathArray[i].hasFn(MFn::kTransform))
		{
			DRN_DBG_LVL1(std::cout << "node:"<<i<<":type:camera"<<std::endl;;);
			dagTable.nodes[i].type = DRN_SCENE_CAMERA;
		}
		else if (dagPathArray[i].hasFn(MFn::kSpotLight) &&  !dagPathArray[i].hasFn(MFn::kTransform))
		{
			DRN_DBG_LVL1(std::cout << "node:"<<i<<":type:camera"<<std::endl;;);
			dagTable.nodes[i].type = DRN_SCENE_SPOTLIGHT;
		}
		else
		{
			DRN_DBG_LVL1(std::cout << "node:"<<i<<":type:NONE"<<std::endl;;);
			dagTable.nodes[i].type = DRN_SCENE_NONE;
		}
		dagTable.nodes[i].dagPath = dagPathArray[i];
	}
	return true;
}

bool writeDagTableInCache(drn_writer_t * cache, const DRNTDagTable & dagTable, drn_scene::SceneContainer & scene)
{
	int32_t status;
	drn_scene::DagNodeContainer * dtDesc = new drn_scene::DagNodeContainer[dagTable.nodes.size()];
	for (unsigned int i = 0 ; i < dagTable.nodes.size(); ++i)
	{
		dtDesc[i].type = dagTable.nodes[i].type;
		dtDesc[i].materialId = dagTable.nodes[i].materialId;
		dtDesc[i].container = dagTable.nodes[i].container;
		dtDesc[i].fullPath = dagTable.nodes[i].fullPath;
	}
	status = drn_writer_add_chunk(cache, dtDesc, dagTable.nodes.size() * sizeof(drn_scene::DagNodeContainer));
	scene.dagNodes = drn_writer_get_last_chunk_id(cache);
	delete[] dtDesc;
	return status;
}

bool writeMaterialsInCache(drn_writer_t * cache, DRNTDagTable & dagTable, drn_scene::SceneContainer & scene)
{
	int32_t drnStatus;
	MStringArray shaderNodeNames;
	MSelectionList shaderList;
	MGlobal::executeCommand("ls -long -typ lambert", shaderNodeNames);
	DRN_DBG_LVL1(std::cout<<"shaders:"<<shaderNodeNames<<std::endl;);
	for (unsigned int i = 0; i < shaderNodeNames.length(); ++i)
	{
		shaderList.add(shaderNodeNames[i]);
	}
	drn_scene::MaterialContainer * materials = new drn_scene::MaterialContainer[shaderList.length()];
	std::map<std::string, uint32_t> materialById;
	for (unsigned int i = 0; i < shaderList.length(); ++i)
	{
		MStatus status;
		MObject nodeObject;
		shaderList.getDependNode(i, nodeObject);
		MFnDependencyNode shaderNode(nodeObject);
		std::cout <<"shader:"<<shaderNode.name();
		float diffuseColor[3] = {0.4f, 0.4f, 0.4f};
		float specularColor[3] = {0.0f, 0.0f, 0.0f};
		float ambientColor[3] = {0.0f, 0.0f, 0.0f};
		float transparency[3] = {0.0f, 0.0f, 0.0f};
		float cosinePower = 0.f;
		MPlug colorPlug = shaderNode.findPlug("color", &status);
		if (status)
		{
			MPlugArray plugArray;
			colorPlug.connectedTo(plugArray, true, false);
			if (plugArray.length() == 1)
			{
				MFnDependencyNode fileNode(plugArray[0].node());
				MPlug filePlug = fileNode.findPlug("fileTextureName", &status);
				if (!status)
				{
					materials[i].diffuseTexture = 0;
				}
				else
				{
					MString fileName = filePlug.asString();
					int x,y,n;
					unsigned char * data = stbi_load(fileName.asChar(), &x, &y, &n, 0);
					drn_scene::TextureContainer d;
					d.width = x;
					d.height = y;
					d.ncomp = n;
					d.bytes = sizeof(unsigned char);
					drnStatus = drn_writer_add_chunk(cache, data, x * y * n * d.bytes);
					d.data = drn_writer_get_last_chunk_id(cache);
					stbi_image_free(data);
					drnStatus = drn_writer_add_chunk(cache, &d, sizeof(drn_scene::TextureContainer));
					materials[i].diffuseTexture = drn_writer_get_last_chunk_id(cache);
				}
			}
			else
			{
				materials[i].diffuseTexture = 0;
			}

			MObject colorObject;
			if (colorPlug.getValue(colorObject))
			{
				float color[3];
				MFnNumericData colorData(colorObject);
				if (colorData.getData(color[0], color[1], color[2]))
				{
					MPlug diffusePlug = shaderNode.findPlug("diffuse", &status);
					if (status)
					{
						float diffuse = diffusePlug.asFloat();
						diffuseColor[0] = color[0] * diffuse;
						diffuseColor[1] = color[1] * diffuse;
						diffuseColor[2] = color[2] * diffuse;
					}
				}
			}
		}
		MPlug specularColorPlug = shaderNode.findPlug("specularColor", &status);
		if (status)
		{
			MObject colorObject;
			if (colorPlug.getValue(colorObject))
			{
				float color[3];
				MFnNumericData colorData(colorObject);
				if (colorData.getData(color[0], color[1], color[2]))
				{
					specularColor[0] = color[0];
					specularColor[1] = color[1];
					specularColor[2] = color[2];
				}
			}
		}
		MPlug ambientColorPlug = shaderNode.findPlug("ambientColor", &status);
		if (status)
		{
			MObject colorObject;
			if (colorPlug.getValue(colorObject))
			{
				float color[3];
				MFnNumericData colorData(colorObject);
				if (colorData.getData(color[0], color[1], color[2]))
				{
					ambientColor[0] = color[0];
					ambientColor[1] = color[1];
					ambientColor[2] = color[2];
				}
			}
		}
		MPlug transparencyPlug = shaderNode.findPlug("transparency", &status);
		if (status)
		{
			MObject transparencyObject;
			if (transparencyPlug.getValue(transparencyObject))
			{
				float t[3];
				MFnNumericData transparencyData(transparencyObject);
				if (transparencyData.getData(t[0], t[1], t[2]))
				{
					transparency[0] = t[0];
					transparency[0] = t[0];
					transparency[0] = t[0];
				}
			}
		}
		MPlug cosinePowerPlug = shaderNode.findPlug("cosinePower", &status);
		if (status)
		{
			cosinePower = cosinePowerPlug.asFloat();
		}
		DRN_DBG_LVL2(
			std::cout <<":diffuseColor:"<<diffuseColor[0]<<";"<<diffuseColor[1]<<";"<<diffuseColor[2];
			std::cout <<":specularColor:"<<specularColor[0]<<";"<<specularColor[1]<<";"<<specularColor[2];
			std::cout <<":ambientColor:"<<ambientColor[0]<<";"<<ambientColor[1]<<";"<<ambientColor[2];
			std::cout <<":transparency:"<<transparency[0]<<";"<<transparency[1]<<";"<<transparency[2];
			std::cout <<":cosinePower:"<<cosinePower;
			std::cout<<std::endl;
			);
		drnStatus = drn_writer_add_chunk(cache, shaderNode.name().asChar(), shaderNode.name().length() + 1);
		materials[i].name = drn_writer_get_last_chunk_id(cache);
		materials[i].diffuseColor[0] = diffuseColor[0];
		materials[i].diffuseColor[1] = diffuseColor[1];
		materials[i].diffuseColor[2] = diffuseColor[2];
		materials[i].diffuseColor[3] = 1.f;
		materials[i].specularColor[0] = specularColor[0];
		materials[i].specularColor[1] = specularColor[1];
		materials[i].specularColor[2] = specularColor[2];
		materials[i].specularColor[3] = 1.f;
		materials[i].ambientColor[0] = ambientColor[0];
		materials[i].ambientColor[1] = ambientColor[1];
		materials[i].ambientColor[2] = ambientColor[2];
		materials[i].ambientColor[3] = 1.f;
		materials[i].transparency[0] = transparency[0];
		materials[i].transparency[1] = transparency[1];
		materials[i].transparency[2] = transparency[2];
		materials[i].transparency[3] = 1.f;
		materials[i].specularPower = cosinePower;
		materialById[shaderNode.name().asChar()] = i;
	}

	for (unsigned int i = 0; i < dagTable.nodes.size(); ++i)
	{
		dagTable.nodes[i].materialId = 0;
		if (dagTable.nodes[i].dagPath.hasFn(MFn::kMesh))
		{
			MFnMesh mesh(dagTable.nodes[i].dagPath);
			int instanceNum = 0;
			if (dagTable.nodes[i].dagPath.isInstanced())
				instanceNum = dagTable.nodes[i].dagPath.instanceNumber();
			MObjectArray shaders;
			MIntArray indices;
			mesh.getConnectedShaders(instanceNum, shaders, indices);
			//DRN_DBG_LVL1(std::cout<<"mesh:"<<dagTable.nodes[i].dagPath.fullPathName()<<":instance:"<<instanceNum<<":shaders:"<<shaders.length()<<":indices:"<<indices<<std::endl;);
			if (shaders.length() > 0)
			{
				MStatus status;
				MFnDependencyNode shaderNodeSG(shaders[0]);
				MPlug surfaceShaderPlug = shaderNodeSG.findPlug("surfaceShader", &status);
				if (status)
				{
					MPlugArray connectedPlugs;
					bool asSrc = false;
					bool asDst = true;
					surfaceShaderPlug.connectedTo( connectedPlugs, asDst, asSrc );
					if (connectedPlugs.length() > 0)
					{
						MFnDependencyNode shaderNode = connectedPlugs[0].node();
						dagTable.nodes[i].materialId = materialById[shaderNode.name().asChar()];
						DRN_DBG_LVL1(std::cout<<"shader:"<<shaderNode.name()<<" "<<dagTable.nodes[i].materialId<<std::endl;);
					}
				}
			}
		}
	}

	drnStatus = drn_writer_add_chunk(cache, materials, sizeof(drn_scene::MaterialContainer) * shaderList.length());
	scene.numMaterials = shaderList.length();
	scene.materials = drn_writer_get_last_chunk_id(cache);
	delete[] materials;
	return drnStatus;
}


bool buildDagStates(const DRNTDagTable & dagTable,
					drn_scene::DagNodeStateContainer * states,
					uint32_t frame)
{
	for (unsigned int i = 0; i < dagTable.nodes.size(); ++i)
	{
		MDagPath path = dagTable.nodes[i].dagPath;
		mmatrixToArray(path.inclusiveMatrix(), states[dagTable.nodes.size()*frame+i].otwTransform);
		if (path.hasFn(MFn::kMesh) || path.hasFn(MFn::kCamera) || path.hasFn(MFn::kSpotLight) )
		{
			MFnDagNode node(path);
			MBoundingBox bbox = node.boundingBox();
			bbox.min().get(states[dagTable.nodes.size()*frame+i].aabb);
			bbox.max().get(states[dagTable.nodes.size()*frame+i].aabb + 4);
		}
	}
	return true;
}

DRNTParameters defaultParameters()
{
	DRNTParameters params;
	params.startFrame = 1;
	params.endFrame = 1;
	params.subFrames = 0;
	params.meshExportMode = MESH_EXPORT_FULL_TOPOLOGY;
	return params;
}

bool parseOptions( const MString & optionString, DRNTParameters & params)
{
	// Parsing
	MStringArray optionList;
	optionString.split(';', optionList);
	for (unsigned int i = 0; i < optionList.length(); ++i)
	{
		MStringArray option;
		optionList[i].split('=', option);
		if (option[0] == MString("startFrame"))
		{
			params.startFrame = option[1].asInt();
			DRN_DBG_LVL1(std::cout<<"parseOptions:startFrame:"<<params.startFrame<<std::endl;);
		}
		else if (option[0] == MString("endFrame"))
		{
			params.endFrame = option[1].asInt();
			DRN_DBG_LVL1(std::cout<<"parseOptions:endFrame:"<<params.endFrame<<std::endl;);
		}
		else if (option[0] == MString("subFrames"))
		{
			params.subFrames = option[1].asInt();
			DRN_DBG_LVL1(std::cout<<"parseOptions:subFrames:"<<params.subFrames<<std::endl;);
		}
		else if (option[0] == MString("meshExport"))
		{
			MString mode(option[1]);
			if (mode ==  "Full")
			{
				params.meshExportMode = MESH_EXPORT_FULL_TOPOLOGY;
			}
			else if (mode == "Triangles")
			{
				params.meshExportMode = MESH_EXPORT_TRIANGLE_TOPOLOGY;
			}
			DRN_DBG_LVL1(std::cout<<"parseOptions:meshExport:"<<params.meshExportMode<<std::endl;);
		}
	}
	return true;
}

bool DRNTranslator::haveReadMethod () const
{
	return false;
}

bool DRNTranslator::haveWriteMethod () const
{
	return true;
}

MString DRNTranslator::defaultExtension () const
{
	return MString("drn");
}

MPxFileTranslator::MFileKind DRNTranslator::identifyFile ( const MFileObject& fileName,
														  const char* buffer,
														  short size) const
{
	return MPxFileTranslator::kNotMyFileType;
}

void * DRNTranslator::creator()
{
	return new DRNTranslator();
}
