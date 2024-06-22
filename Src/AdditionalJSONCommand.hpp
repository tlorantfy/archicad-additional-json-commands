#if !defined (ADDITIONALJSONCOMMAND_HPP)
#define ADDITIONALJSONCOMMAND_HPP

#pragma once

#include	"APIEnvir.h"
#include	"ACAPinc.h"


class AdditionalJSONCommand : public API_AddOnCommand {
public:
	virtual GS::String							GetNamespace () const override;
	virtual GS::Optional<GS::UniString>			GetSchemaDefinitions () const override;
	virtual GS::Optional<GS::UniString>			GetInputParametersSchema () const override;
	virtual GS::Optional<GS::UniString>			GetResponseSchema () const override;
	virtual API_AddOnCommandExecutionPolicy		GetExecutionPolicy () const override			{ return API_AddOnCommandExecutionPolicy::ScheduleForExecutionOnMainThread; }
	virtual void								OnResponseValidationFailed (const GS::ObjectState& response) const override;
	#ifdef ServerMainVers_2600
		virtual bool IsProcessWindowVisible () const override;
	#endif

    static GS::ObjectState  CreateErrorResponse (APIErrCodes errorCode, const GS::UniString& errorMessage);
    static API_Guid         GetGuidFromElementIdField (const GS::ObjectState& os);
};

constexpr const char* ElementIdField = "elementId";

namespace Utilities {
	GS::Array<GS::Pair<short, double>> GetStoryLevels ();
	short GetFloorIndexAndOffset (double zPos, const GS::Array<GS::Pair<short, double>>& storyLevels, double& zOffset);

	API_Coord 	Get2DCoordinateFromObjectState (const GS::ObjectState& objectState);
	API_Coord3D Get3DCoordinateFromObjectState (const GS::ObjectState& objectState);
}

#endif