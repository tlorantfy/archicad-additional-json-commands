// *****************************************************************************
// Contact person:	Tibor Lorántfy (lorantfyt@gmail.com)
// *****************************************************************************

#include	"APIEnvir.h"
#include	"ACAPinc.h"		// also includes APIdefs.h

#include	"PublishCommand.hpp"
#include	"PublishCommand.hpp"
#include	"TeamworkReceiveCommand.hpp"
#include	"GetProjectInfoCommand.hpp"
#include	"GetArchicadLocationCommand.hpp"
#include	"QuitCommand.hpp"
#include	"ReloadLibrariesCommand.hpp"
#include	"MoveElementsCommand.hpp"
#include	"CreateColumnsCommand.hpp"
#include	"GetHotlinksCommand.hpp"
#include	"GetGDLParametersOfElementsCommand.hpp"
#include	"ChangeGDLParametersOfElementsCommand.hpp"


// =============================================================================
//
// Required functions
//
// =============================================================================

// -----------------------------------------------------------------------------
// Dependency definitions
// -----------------------------------------------------------------------------

API_AddonType	__ACDLL_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	RSGetIndString (&envir->addOnInfo.name, 32000, 1, ACAPI_GetOwnResModule ());
	RSGetIndString (&envir->addOnInfo.description, 32000, 2, ACAPI_GetOwnResModule ());

	return APIAddon_Preload;
}		// CheckEnvironment


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------

GSErrCode	__ACDLL_CALL	RegisterInterface (void)
{
	return NoError;
}		// RegisterInterface


// -----------------------------------------------------------------------------
// Initialize
//		called after the Add-On has been loaded into memory
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	Initialize (void)
{
	GSErrCode err = NoError;
	
	err |= ACAPI_Install_AddOnCommandHandler (GS::NewOwned<PublishCommand> ());
	err |= ACAPI_Install_AddOnCommandHandler (GS::NewOwned<TeamworkReceiveCommand> ());
	err |= ACAPI_Install_AddOnCommandHandler (GS::NewOwned<GetProjectInfoCommand> ());
	err |= ACAPI_Install_AddOnCommandHandler (GS::NewOwned<GetArchicadLocationCommand> ());
	err |= ACAPI_Install_AddOnCommandHandler (GS::NewOwned<QuitCommand> ());
	err |= ACAPI_Install_AddOnCommandHandler (GS::NewOwned<ReloadLibrariesCommand> ());
	err |= ACAPI_Install_AddOnCommandHandler (GS::NewOwned<MoveElementsCommand> ());
	err |= ACAPI_Install_AddOnCommandHandler (GS::NewOwned<CreateColumnsCommand> ());
	err |= ACAPI_Install_AddOnCommandHandler (GS::NewOwned<GetHotlinksCommand> ());
	err |= ACAPI_Install_AddOnCommandHandler (GS::NewOwned<GetGDLParametersOfElementsCommand> ());
	err |= ACAPI_Install_AddOnCommandHandler (GS::NewOwned<ChangeGDLParametersOfElementsCommand> ());

	return err;
}		// Initialize


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)
{
	return NoError;
}		// FreeData
