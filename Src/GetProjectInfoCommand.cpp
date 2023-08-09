#include	"GetProjectInfoCommand.hpp"
#include	"ObjectState.hpp"


GS::String GetProjectInfoCommand::GetName () const
{
	return "GetProjectInfo";
}


constexpr const char* IsUntitledResponseField = "isUntitled";
constexpr const char* IsTeamworkResponseField = "isTeamwork";
constexpr const char* ProjectLocationResponseField = "projectLocation";
constexpr const char* ProjectPathResponseField = "projectPath";
constexpr const char* ProjectNameResponseField = "projectName";


GS::Optional<GS::UniString> GetProjectInfoCommand::GetResponseSchema () const
{
	return GS::UniString::Printf (R"({
	"type": "object",
	"properties": {
		"%s": {
			"type": "boolean",
			"description": "True, if the project is not saved yet."
		},
		"%s": {
			"type": "boolean",
			"description": "True, if the project is a Teamwork (BIMcloud) project."
		},
		"%s": {
			"type": "string",
			"description": "The location of the project in the filesystem or a BIMcloud project reference.",
			"minLength": 1
		},
		"%s": {
			"type": "string",
			"description": "The path of the project. A filesystem path or a BIMcloud server relative path.",
			"minLength": 1
		},
		"%s": {
			"type": "string",
			"description": "The name of the project.",
			"minLength": 1
		}
	},
	"additionalProperties": false,
	"required": [
		"%s",
		"%s"
	]
})",
IsUntitledResponseField, IsTeamworkResponseField, ProjectLocationResponseField, ProjectPathResponseField, ProjectNameResponseField,
IsUntitledResponseField, IsTeamworkResponseField);
}


GS::ObjectState	GetProjectInfoCommand::Execute (const GS::ObjectState& /*parameters*/, GS::ProcessControl& /*processControl*/) const
{
	API_ProjectInfo projectInfo = {};
	GSErrCode err = ACAPI_Environment (APIEnv_ProjectID, &projectInfo);

	if (err != NoError) {
		return CreateErrorResponse (APIERR_NOPLAN, "Failed to retrieve project information. Check the opened project!");
	}

	GS::ObjectState response;
	response.Add (IsUntitledResponseField, projectInfo.untitled);
	if (!projectInfo.untitled) {
		response.Add (IsTeamworkResponseField, projectInfo.teamwork);
		if (projectInfo.location) {
			response.Add (ProjectLocationResponseField, projectInfo.location->ToDisplayText ());
		}
		if (projectInfo.projectPath) {
			response.Add (ProjectPathResponseField, *projectInfo.projectPath);
		}
		if (projectInfo.projectName) {
			response.Add (ProjectNameResponseField, *projectInfo.projectName);
		}
	}

	return response;
}