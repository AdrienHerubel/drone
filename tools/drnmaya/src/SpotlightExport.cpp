#include "SpotlightExport.hpp"
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
#include <maya/MFnSpotLight.h>
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

bool writeSpotlightContainerInCache(drn_writer_t * cache, 
								  DRNTDagNode & node, 
								  drn_scene::SpotlightContainer * spotlightContainer, 
								  drn_scene::SpotlightDynamicDataContainer * dynContainers, 
								  unsigned int frameCount)
{
	int32_t status;
	status = drn_writer_add_chunk(cache, dynContainers, frameCount * sizeof (drn_scene::SpotlightDynamicDataContainer));
	spotlightContainer->dynamicData = drn_writer_get_last_chunk_id(cache);
	status = drn_writer_add_chunk(cache, spotlightContainer, sizeof(drn_scene::SpotlightContainer));
	node.container = drn_writer_get_last_chunk_id(cache);
	status = drn_writer_add_chunk(cache, node.dagPath.fullPathName().asChar(), node.dagPath.fullPathName().length() + 1);
	node.fullPath = drn_writer_get_last_chunk_id(cache);
	return status;
}

bool writeSpotlightStaticDataInCache(drn_writer_t * cache, 
								  DRNTDagNode & node, 
								  drn_scene::SpotlightContainer * spotlightContainer)
{
	MStatus status = MS::kSuccess;
	MFnSpotLight spotlight(node.dagPath);
	return status;
}

bool writeSpotlightDynamicDataInCache(drn_writer_t * cache,
								   DRNTDagNode & node,
								   drn_scene::SpotlightContainer * spotlightContainer,
								   drn_scene::SpotlightDynamicDataContainer * spotlightDynContainers,
								   unsigned int frame)
{
	MDagPath path(node.dagPath);
	MFnSpotLight spot(node.dagPath);
	drn_scene::SpotlightDynamicDataContainer * sd = spotlightDynContainers + frame;
	MPoint wPos = MPoint(0.0, 0.0, 0.0) * path.inclusiveMatrix();
	MVector wDir = (MVector(0.0, 0.0, -1.0) * path.inclusiveMatrix()).normal();
	mvectorToArray(wPos, sd->wPosition);
	mvectorToArray(wDir, sd->wDirection);
	mcolorToArray(spot.color(), sd->color);
	sd->intensity = spot.intensity();
	sd->angle = spot.coneAngle();
	short decay = spot.decayRate();
	if (decay == 0)
		sd->decayRate = drn_scene::SpotlightDynamicDataContainer::NO_DECAY;
	else if (decay == 1)
		sd->decayRate = drn_scene::SpotlightDynamicDataContainer::LINEAR;
	else if (decay == 2)
		sd->decayRate = drn_scene::SpotlightDynamicDataContainer::QUADRATIC;	
	sd->shadow = drn_scene::SpotlightDynamicDataContainer::NO_SHADOW;
	if (spot.useDepthMapShadows())
	{
		if (spot.castSoftShadows())
		{
			sd->shadow = drn_scene::SpotlightDynamicDataContainer::SOFT;
		}
		else
		{
			sd->shadow = drn_scene::SpotlightDynamicDataContainer::HARD;
		}	
	}	
	return true;
}
