#ifndef __DRNT_CAMERA_EXPORT_HPP__
#define __DRNT_CAMERA_EXPORT_HPP__

#include <drone/drn_writer.h>

class DRNTDagNode;
namespace drn_scene { class CameraContainer; class CameraDynamicDataContainer; }

bool writeCameraContainerInCache(drn_writer_t * cache, 
								  DRNTDagNode & node, 
								  drn_scene::CameraContainer * cameraContainer, 
								  drn_scene::CameraDynamicDataContainer * dynContainers, 
								  unsigned int frameCount);
bool writeCameraStaticDataInCache(drn_writer_t * cache, 
								  DRNTDagNode & node, 
								  drn_scene::CameraContainer * cameraContainer);
bool writeCameraDynamicDataInCache(drn_writer_t * cache,
								 DRNTDagNode & node,
								 drn_scene::CameraContainer * cameraContainer,
								 drn_scene::CameraDynamicDataContainer * cameraDynContainers,
								 unsigned int frame);

#endif // __DRNT_CAMERA_EXPORT_HPP__
