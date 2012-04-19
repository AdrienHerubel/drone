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

enum MeshExportMode
{
	MESH_EXPORT_BASE_TOPOLOGY,
	MESH_EXPORT_TRIANGLE_TOPOLOGY,
	MESH_EXPORT_FULL_TOPOLOGY,
};


bool writeMeshContainerInCache(drn_writer_t * cache,
								DRNTDagNode & node,
								drn_scene::MeshContainer * meshContainer,
								drn_scene::MeshDynamicDataContainer * dynContainers,
								unsigned int frameCount, MeshExportMode mode);
bool writeMeshStaticDataInCache(drn_writer_t * cache,
								DRNTDagNode & node,
								drn_scene::MeshContainer * meshContainer,
								DRNTHardEdge & he, MeshExportMode mode);
bool writeMeshDynamicDataInCache(drn_writer_t * cache,
								 DRNTDagNode & node,
								 drn_scene::MeshContainer * meshContainer,
								 DRNTHardEdge & he,
								 drn_scene::MeshDynamicDataContainer * meshDynContainers,
								 unsigned int frame, MeshExportMode mode);

#endif // __DRNT_MESH_EXPORT_HPP__
