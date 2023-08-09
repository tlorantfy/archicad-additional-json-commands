#include	"TeamworkReceiveCommand.hpp"
#include	"ObjectState.hpp"


GS::String TeamworkReceiveCommand::GetName () const
{
	return "TeamworkReceive";
}


GS::ObjectState	TeamworkReceiveCommand::Execute (const GS::ObjectState& /*parameters*/, GS::ProcessControl& /*processControl*/) const
{
	GSErrCode err = ACAPI_TeamworkControl_ReceiveChanges ();

	if (err != NoError) {
		return CreateErrorResponse (APIERR_SERVICEFAILED, "Receive failed. Check internet connection!");
	}

	return {};
}