#include "drnMayaPlugin.hpp"
#include "drnTranslator.hpp"

#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

MStatus initializePlugin( MObject obj )
{
	MStatus			status;
	MFnPlugin plugin( obj, "Drone", "1.0", "Any");
	status = plugin.registerFileTranslator( "drnTranslator", "none",
											DRNTranslator::creator,
											"drnTranslator",
											NULL,
											false);
	if (!status) 
	{
		status.perror("Impossible to register drone exporter");
		return status;
	}
	return status;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus			status;
	MFnPlugin plugin( obj );
	status	= plugin.deregisterFileTranslator( "drnTranslator");
	if (!status)
	{
		status.perror("Impossible to deregister drone exporter");
		return status;
	}
	return status;
}
