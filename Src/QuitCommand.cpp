#include	"QuitCommand.hpp"
#include	"ObjectState.hpp"


GS::String QuitCommand::GetName () const
{
	return "Quit";
}


GS::ObjectState	QuitCommand::Execute (const GS::ObjectState& /*parameters*/, GS::ProcessControl& /*processControl*/) const
{
	Int64 magicCode = 1234;
	GSErrCode err = ACAPI_Automate (APIDo_QuitID, reinterpret_cast<void*> (magicCode));

	if (err != NoError) {
		return CreateErrorResponse (APIERR_COMMANDFAILED, "Failed to quit Archicad!");
	}

	return {};
}