#include	"GetArchicadLocationCommand.hpp"
#include	"ObjectState.hpp"
#include	"FileSystem.hpp"


static GS::Optional<IO::Location>	GetApplicationLocation ()
{
	IO::Location applicationFileLocation;

	GSErrCode error = IO::fileSystem.GetSpecialLocation (IO::FileSystem::ApplicationFile, &applicationFileLocation);
	if (error != NoError) {
		return GS::NoValue;
	}

	return applicationFileLocation;
}


GS::String GetArchicadLocationCommand::GetName () const
{
	return "GetArchicadLocation";
}


constexpr const char* ArchicadLocationResponseField = "archicadLocation";


GS::Optional<GS::UniString> GetArchicadLocationCommand::GetResponseSchema () const
{
	return GS::UniString::Printf (R"({
	"type": "object",
	"properties": {
		"%s": {
			"type": "string",
			"description": "The location of the Archicad executable in the filesystem.",
			"minLength": 1
		}
	},
	"additionalProperties": false,
	"required": [
		"%s"
	]
})",
ArchicadLocationResponseField,
ArchicadLocationResponseField);
}


GS::ObjectState	GetArchicadLocationCommand::Execute (const GS::ObjectState& /*parameters*/, GS::ProcessControl& /*processControl*/) const
{
	const GS::Optional<IO::Location> applicationFileLocation = GetApplicationLocation ();

	if (!applicationFileLocation.HasValue ()) {
		return CreateErrorResponse (APIERR_GENERAL, "Failed to get the location of the Archicad application!");
	}

	return GS::ObjectState (ArchicadLocationResponseField, applicationFileLocation.Get ().ToDisplayText ());
}