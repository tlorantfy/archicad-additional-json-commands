#include	"AdditionalJSONCommands.hpp"
#include	"ObjectState.hpp"
#include	"FileSystem.hpp"


constexpr const char* AdditionalJSONCommandsNamespace = "AdditionalJSONCommands";


static GS::HashTable<GS::UniString, API_Guid> GetPublisherSetNameGuidTable ()
{
	GS::HashTable<GS::UniString, API_Guid> table;

	Int32 numberOfPublisherSets = 0;
	ACAPI_Navigator (APINavigator_GetNavigatorSetNumID, &numberOfPublisherSets);

	API_NavigatorSet set = {};
	for (Int32 ii = 0; ii < numberOfPublisherSets; ++ii) {
		set.mapId = API_PublisherSets;
		if (ACAPI_Navigator (APINavigator_GetNavigatorSetID, &set, &ii) == NoError) {
			table.Add (set.name, set.rootGuid);
		}
	}

	return table;
}


static GS::Optional<IO::Location>	GetApplicationLocation ()
{
	IO::Location applicationFileLocation;

	GSErrCode error = IO::fileSystem.GetSpecialLocation (IO::FileSystem::ApplicationFile, &applicationFileLocation);
	if (error != NoError) {
		return GS::NoValue;
	}

	return applicationFileLocation;
}


constexpr const char* ErrorResponseField = "error";
constexpr const char* ErrorCodeResponseField = "code";
constexpr const char* ErrorMessageResponseField = "message";


static GS::ObjectState CreateErrorResponse (APIErrCodes errorCode, const char* errorMessage)
{
	GS::ObjectState error;
	error.Add (ErrorCodeResponseField, errorCode);
	error.Add (ErrorMessageResponseField, errorMessage);
	return GS::ObjectState (ErrorResponseField, error);
}


// --- AdditionalJSONCommand ----------------------------------------------------------------------------------

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


// --- PublishCommand ----------------------------------------------------------------------------------

GS::String PublishCommand::GetName () const
{
	return "Publish";
}


constexpr const char* PublisherSetNameParameterField = "publisherSetName";
constexpr const char* OutputPathParameterField = "outputPath";


GS::Optional<GS::UniString> PublishCommand::GetInputParametersSchema () const
{
	return GS::UniString::Printf (R"({
	"type": "object",
	"properties": {
		"%s": {
			"type": "string",
			"description": "The name of the publisher set.",
			"minLength": 1
		},
		"%s": {
			"type": "string",
			"description": "Full local or LAN path for publishing. Optional, by default the path set in the settings of the publiser set will be used.",
			"minLength": 1
		}
	},
	"additionalProperties": false,
	"required": [
		"%s"
	]
})",
PublisherSetNameParameterField, OutputPathParameterField,
PublisherSetNameParameterField);
}


GS::ObjectState	PublishCommand::Execute (const GS::ObjectState& parameters, GS::ProcessControl& /*processControl*/) const
{
	GS::UniString publisherSetName;
	parameters.Get (PublisherSetNameParameterField, publisherSetName);

	const auto publisherSetNameGuidTable = GetPublisherSetNameGuidTable ();
	if (!publisherSetNameGuidTable.ContainsKey (publisherSetName)) {
		return CreateErrorResponse (APIERR_BADNAME, "Not valid publisher set name.");
	}

	API_PublishPars	publishPars = {};
	publishPars.guid = publisherSetNameGuidTable.Get (publisherSetName);

	if (parameters.Contains (OutputPathParameterField)) {
		GS::UniString outputPath;
		parameters.Get (OutputPathParameterField, outputPath);
		publishPars.path = new IO::Location (outputPath);
	}

	GSErrCode err = ACAPI_Automate (APIDo_PublishID, &publishPars);

	delete publishPars.path;

	if (err != NoError) {
		return CreateErrorResponse (APIERR_COMMANDFAILED, "Publishing failed. Check output path!");
	}

	return {};
}


// --- TeamworkReceiveCommand ----------------------------------------------------------------------------------

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


// --- GetProjectInfoCommand ----------------------------------------------------------------------------------

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


// --- GetArchicadLocationCommand ----------------------------------------------------------------------------------

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


// --- QuitCommand ----------------------------------------------------------------------------------

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