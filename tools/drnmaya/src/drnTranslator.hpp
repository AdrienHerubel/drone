#ifndef __DRN_TRANSLATOR_HPP__
#define __DRN_TRANSLATOR_HPP__

#include <maya/MPxFileTranslator.h>
#include <maya/MDagPath.h>
#include <drone/drn_writer.h>

class MString;
class MFileObject;
class MSelectionList;
class MDagPathArray;

struct DRNTDagNode
{
	uint64_t dagId;
	uint32_t type;
	MDagPath  dagPath;
	uint32_t materialId;
	drn_chunk_id_t container;
	drn_chunk_id_t fullPath;
};

class DRNTranslator : public MPxFileTranslator
{
public :
	DRNTranslator();
	virtual ~DRNTranslator();
	MStatus reader(const MFileObject& file,
				   const MString& optionsString,
				   FileAccessMode mode);
	MStatus writer(const MFileObject& file,
				   const MString& optionsString,
				   FileAccessMode mode);
	bool haveReadMethod() const;
	bool haveWriteMethod() const;
	MString defaultExtension() const;
	MFileKind identifyFile(const MFileObject& fileName,
						   const char* buffer,
						   short size) const;
	static void * creator();
private :
	DRNTranslator(const DRNTranslator &);
	DRNTranslator & operator=(const DRNTranslator &);
};

#endif // __DRN_TRANSLATOR_HPP__
