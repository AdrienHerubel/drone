#ifndef __DRNT_MESH_EXPORT_HPP__
#define __DRNT_MESH_EXPORT_HPP__

#include <drone/drn_writer.h>
#include <vector>

class DRNTDagNode;
namespace drn_scene { class MeshContainer; class MeshDynamicDataContainer; }

struct triplet
{
	inline triplet() : first(0), second(0), third(0) {}
	inline triplet(int x, int y, int z) : first(x), second(y), third(z) {}
	inline bool operator<(const triplet & t) const { return first < t.first || (!(first < t.first) && second < t.second) || (!(first < t.first) && !(second < t.second) && third < t.third); 
	}
	int first;
	int second;
	int third;	
};

struct DRNTHardEdge
{
	std::vector<int> tlist;
	std::vector<triplet > itlist;
	int idmax;
};

bool writeMeshContainerInCache(drn_writer_t * cache, 
								DRNTDagNode & node, 
								drn_scene::MeshContainer * meshContainer, 
								drn_scene::MeshDynamicDataContainer * dynContainers, 
								unsigned int frameCount);
bool writeMeshStaticDataInCache(drn_writer_t * cache, 
								DRNTDagNode & node, 
								drn_scene::MeshContainer * meshContainer, 
								DRNTHardEdge & he);
bool writeMeshDynamicDataInCache(drn_writer_t * cache,
								 DRNTDagNode & node,
								 drn_scene::MeshContainer * meshContainer,
								 DRNTHardEdge & he,
								 drn_scene::MeshDynamicDataContainer * meshDynContainers,
								 unsigned int frame);

#endif // __DRNT_MESH_EXPORT_HPP__
