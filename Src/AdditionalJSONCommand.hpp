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

	void SetValueInteger (API_ChangeParamType& changeParam,
					  	  const GS::ObjectState& parameterDetails);
	void SetValueDouble (API_ChangeParamType& changeParam,
					 	 const GS::ObjectState&	parameterDetails);
	void SetValueOnOff (API_ChangeParamType& changeParam,
						const GS::ObjectState&	parameterDetails);
	void SetValueBool (API_ChangeParamType& changeParam,
				   	   const GS::ObjectState&	parameterDetails);
	void SetValueString (API_ChangeParamType& changeParam,
					 	 const GS::ObjectState&	parameterDetails);
	void SetValueInteger (API_AddParType& addPar,
					  	  const GS::ObjectState& parameterDetails);
	void SetValueDouble (API_AddParType& addPar,
					 	 const GS::ObjectState&	parameterDetails);
	void SetValueOnOff (API_AddParType& addPar,
						const GS::ObjectState&	parameterDetails);
	void SetValueBool (API_AddParType& addPar,
				   	   const GS::ObjectState&	parameterDetails);
	void SetValueString (API_AddParType& addPar,
					 	 const GS::ObjectState&	parameterDetails);

	void ChangeParams (API_AddParType**& params, const GS::HashTable<GS::String, GS::ObjectState>& changeParamsDictionary);
}

#endif