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


}