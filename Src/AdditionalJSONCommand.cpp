#include	"AdditionalJSONCommand.hpp"
#include	"ObjectState.hpp"
#include	"FileSystem.hpp"
#include	"OnExit.hpp"


constexpr const char* AdditionalJSONCommandsNamespace = "AdditionalJSONCommands";
constexpr const char* ErrorResponseField = "error";
constexpr const char* ErrorCodeResponseField = "code";
constexpr const char* ErrorMessageResponseField = "message";
constexpr const char* GuidField = "guid";


GS::String AdditionalJSONCommand::GetNamespace () const
{
	return AdditionalJSONCommandsNamespace;
}


GS::Optional<GS::UniString> AdditionalJSONCommand::GetSchemaDefinitions () const
{
	return {};
}


GS::Optional<GS::UniString> AdditionalJSONCommand::GetInputParametersSchema () const
{
	return {};
}


GS::Optional<GS::UniString> AdditionalJSONCommand::GetResponseSchema () const
{
	return GS::UniString::Printf (R"({
	"type": "object",
	"properties": {
		"%s": {
			"$ref": "APITypes.json#/definitions/Error"
		}
	},
	"additionalProperties": false,
	"required": []
})",
ErrorResponseField);
}


void AdditionalJSONCommand::OnResponseValidationFailed (const GS::ObjectState& /*response*/) const
{
}

#ifdef ServerMainVers_2600
bool AdditionalJSONCommand::IsProcessWindowVisible () const
{
	return false;
}
#endif


GS::ObjectState AdditionalJSONCommand::CreateErrorResponse (APIErrCodes errorCode, const GS::UniString& errorMessage)
{
	GS::ObjectState error;
	error.Add (ErrorCodeResponseField, errorCode);
	error.Add (ErrorMessageResponseField, errorMessage.ToCStr ().Get ());
	return GS::ObjectState (ErrorResponseField, error);
}


API_Guid AdditionalJSONCommand::GetGuidFromElementIdField (const GS::ObjectState& os)
{
	GS::String guid;
	os.Get (GuidField, guid);
	return APIGuidFromString (guid.ToCStr ());
}


namespace Utilities {


GS::Array<GS::Pair<short, double>> GetStoryLevels ()
{
	GS::Array<GS::Pair<short, double>> storyLevels;
	API_StoryInfo storyInfo = {};

	GSErrCode err = ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	if (err == NoError) {
		const short numberOfStories = storyInfo.lastStory - storyInfo.firstStory + 1;
		for (short i = 0; i < numberOfStories; ++i) {
			storyLevels.PushNew ((*storyInfo.data)[i].index, (*storyInfo.data)[i].level);
		}
		BMKillHandle ((GSHandle*) &storyInfo.data);
	}
	return storyLevels;
}


short GetFloorIndexAndOffset (double zPos, const GS::Array<GS::Pair<short, double>>& storyLevels, double& zOffset)
{
	if (storyLevels.IsEmpty ()) {
		zOffset = zPos;
		return 0;
	}

	auto* lastStoryIndexAndLevel = &storyLevels[0];
	for (const auto& storyIndexAndLevel : storyLevels) {
		if (storyIndexAndLevel.second > zPos) {
			break;
		}
		lastStoryIndexAndLevel = &storyIndexAndLevel;
	}

	zOffset = zPos - lastStoryIndexAndLevel->second;
	return lastStoryIndexAndLevel->first;
}


API_Coord Get2DCoordinateFromObjectState (const GS::ObjectState& objectState)
{
	API_Coord coordinate = {};
	objectState.Get ("x", coordinate.x);
	objectState.Get ("y", coordinate.y);
	return coordinate;
}


API_Coord3D Get3DCoordinateFromObjectState (const GS::ObjectState& objectState)
{
	API_Coord3D coordinate = {};
	objectState.Get ("x", coordinate.x);
	objectState.Get ("y", coordinate.y);
	objectState.Get ("z", coordinate.z);
	return coordinate;
}


constexpr const char* ParameterValueFieldName = "value";


void SetValueInteger (API_ChangeParamType& changeParam,
					  const GS::ObjectState&	parameterDetails)
{
	Int32 value;
	parameterDetails.Get (ParameterValueFieldName, value);
	changeParam.realValue = value;
}


void SetValueDouble (API_ChangeParamType& changeParam,
					 const GS::ObjectState&	parameterDetails)
{
	double value;
	parameterDetails.Get (ParameterValueFieldName, value);
	changeParam.realValue = value;
}


void SetValueOnOff (API_ChangeParamType& changeParam,
					const GS::ObjectState&	parameterDetails)
{
	GS::String value;
	parameterDetails.Get (ParameterValueFieldName, value);
	changeParam.realValue = (value == "Off" ? 0 : 1);
}
 

void SetValueBool (API_ChangeParamType& changeParam,
				   const GS::ObjectState&	parameterDetails)
{
	bool value;
	parameterDetails.Get (ParameterValueFieldName, value);
	changeParam.realValue = (value ? 0 : 1);
}


void SetValueString (API_ChangeParamType& changeParam,
					 const GS::ObjectState&	parameterDetails)
{
	GS::UniString value;
	parameterDetails.Get (ParameterValueFieldName, value);

	constexpr USize MaxStrValueLength = 512;

	static GS::uchar_t strValuePtr[MaxStrValueLength];
	GS::ucscpy (strValuePtr, value.ToUStr (0, GS::Min(value.GetLength (), MaxStrValueLength)).Get ());

	changeParam.uStrValue = strValuePtr;
}


void SetValueInteger (API_AddParType& addPar,
					  const GS::ObjectState&	parameterDetails)
{
	Int32 value;
	parameterDetails.Get (ParameterValueFieldName, value);
	addPar.value.real = value;
}


void SetValueDouble (API_AddParType& addPar,
					 const GS::ObjectState&	parameterDetails)
{
	double value;
	parameterDetails.Get (ParameterValueFieldName, value);
	addPar.value.real = value;
}


void SetValueOnOff (API_AddParType& addPar,
					const GS::ObjectState&	parameterDetails)
{
	GS::String value;
	parameterDetails.Get (ParameterValueFieldName, value);
	addPar.value.real = (value == "Off" ? 0 : 1);
}
 

void SetValueBool (API_AddParType& addPar,
				   const GS::ObjectState&	parameterDetails)
{
	bool value;
	parameterDetails.Get (ParameterValueFieldName, value);
	addPar.value.real = (value ? 0 : 1);
}


void SetValueString (API_AddParType& addPar,
					 const GS::ObjectState&	parameterDetails)
{
	GS::UniString value;
	parameterDetails.Get (ParameterValueFieldName, value);

	GS::ucscpy (addPar.value.uStr, value.ToUStr (0, GS::Min(value.GetLength (), (USize)API_UAddParStrLen)).Get ());
}


void ChangeParams (API_AddParType**& params, const GS::HashTable<GS::String, GS::ObjectState>& changeParamsDictionary)
{
	const GSSize nParams = BMGetHandleSize ((GSHandle) params) / sizeof (API_AddParType);
	GS::HashTable<GS::String, API_AddParID> gdlParametersTypeDictionary;
	for (GSIndex ii = 0; ii < nParams; ++ii) {
		API_AddParType& actParam = (*params)[ii];

		const GS::String name(actParam.name);
		const auto* value = changeParamsDictionary.GetPtr (name);
		if (value == nullptr)
			continue;

		switch (actParam.typeID) {
			case APIParT_Integer:
			case APIParT_PenCol:				SetValueInteger (actParam, *value);	break;
			case APIParT_ColRGB:
			case APIParT_Intens:
			case APIParT_Length:
			case APIParT_RealNum:
			case APIParT_Angle:					SetValueDouble (actParam, *value);	break;
			case APIParT_LightSw:				SetValueOnOff (actParam, *value); 	break;
			case APIParT_Boolean: 				SetValueBool (actParam, *value);	break;
			case APIParT_LineTyp:
			case APIParT_Mater:
			case APIParT_FillPat:
			case APIParT_BuildingMaterial:
			case APIParT_Profile: 				SetValueInteger (actParam, *value);	break;
			case APIParT_CString:
			case APIParT_Title: 				SetValueString (actParam, *value);	break;
			default:
			case APIParT_Dictionary:
				// Not supported by the Archicad API yet
				break;
		}
	}
}


}