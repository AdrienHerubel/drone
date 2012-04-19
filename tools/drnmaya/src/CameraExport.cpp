#include "CameraExport.hpp"
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

bool writeCameraContainerInCache(drn_writer_t * cache,
								  DRNTDagNode & node,
								  drn_scene::CameraContainer * cameraContainer,
								  drn_scene::CameraDynamicDataContainer * dynContainers,
								  unsigned int frameCount)
{
	int32_t status;
	status = drn_writer_add_chunk(cache, dynContainers, frameCount * sizeof (drn_scene::CameraDynamicDataContainer));
	cameraContainer->dynamicData = drn_writer_get_last_chunk_id(cache);
	status = drn_writer_add_chunk(cache, cameraContainer, sizeof(drn_scene::CameraContainer));
	node.container = drn_writer_get_last_chunk_id(cache);
	status = drn_writer_add_chunk(cache, node.dagPath.fullPathName().asChar(), node.dagPath.fullPathName().length() + 1);
	node.fullPath = drn_writer_get_last_chunk_id(cache);
	return status;
}

bool writeCameraStaticDataInCache(drn_writer_t * cache,
								  DRNTDagNode & node,
								  drn_scene::CameraContainer * cameraContainer)
{
	MStatus status = MS::kSuccess;
	MFnCamera camera(node.dagPath);
	return status;
}

bool writeCameraDynamicDataInCache(drn_writer_t * cache,
								   DRNTDagNode & node,
								   drn_scene::CameraContainer * cameraContainer,
								   drn_scene::CameraDynamicDataContainer * cameraDynContainers,
								   unsigned int frame)
{
	MStatus status;
	MFnCamera camera(node.dagPath);
	MPoint eye = camera.eyePoint(MSpace::kWorld);
	MVector dir = camera.viewDirection(MSpace::kWorld);
	MPoint center = camera.centerOfInterestPoint(MSpace::kWorld);
	MVector up = camera.upDirection(MSpace::kWorld);
	MVector right = camera.rightDirection(MSpace::kWorld);
	DRNT_DBG_LVL1(std::cout << "writeCameraDynamicDataInCache:"<<node.dagPath.fullPathName()
				<< ":frame:"<< frame << ":eye:"<<eye<<":up:"<<up<<":dir:"<<dir<<std::endl;);
	eye.get(cameraDynContainers[frame].eye);
	center.get(cameraDynContainers[frame].center);
	mvectorToArray(dir, cameraDynContainers[frame].direction);
	mvectorToArray(up, cameraDynContainers[frame].up);
	mvectorToArray(right, cameraDynContainers[frame].right);
	return true;
}
