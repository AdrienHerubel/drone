#ifndef __DRNT_SPOTLIGHT_EXPORT_HPP__
#define __DRNT_SPOTLIGHT_EXPORT_HPP__

#include <drone/drn_writer.h>

class DRNTDagNode;
namespace drn_scene { class SpotlightContainer; class SpotlightDynamicDataContainer; }

bool writeSpotlightContainerInCache(drn_writer_t * cache,
								  DRNTDagNode & node,
								  drn_scene::SpotlightContainer * spotlightContainer,
								  drn_scene::SpotlightDynamicDataContainer * dynContainers,
								  unsigned int frameCount);
bool writeSpotlightStaticDataInCache(drn_writer_t * cache,
								  DRNTDagNode & node,
								  drn_scene::SpotlightContainer * spotlightContainer);
bool writeSpotlightDynamicDataInCache(drn_writer_t * cache,
								 DRNTDagNode & node,
								 drn_scene::SpotlightContainer * spotlightContainer,
								 drn_scene::SpotlightDynamicDataContainer * spotlightDynContainers,
								 unsigned int frame);

#endif // __DRNT_SPOTLIGHT_EXPORT_HPP__
