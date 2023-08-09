#include	"ReloadLibrariesCommand.hpp"
#include	"ObjectState.hpp"


GS::String ReloadLibrariesCommand::GetName () const
{
	return "ReloadLibraries";
}


GS::ObjectState	ReloadLibrariesCommand::Execute (const GS::ObjectState& /*parameters*/, GS::ProcessControl& /*processControl*/) const
{
	GSErrCode err = ACAPI_Automate (APIDo_ReloadLibrariesID);

	if (err != NoError) {
		return CreateErrorResponse (APIERR_COMMANDFAILED, "Reloading Libraries failed. Check internet connection if you have active libraries from BIMcloud!");
	}

	return {};
}