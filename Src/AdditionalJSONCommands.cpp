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


// --- PublishCommand ----------------------------------------------------------------------------------

GS::String PublishCommand::GetName () const
{
	return "Publish";
}


GS::String PublishCommand::GetNamespace () const
{
	return AdditionalJSONCommandsNamespace;
}


GS::Optional<GS::UniString> PublishCommand::GetSchemaDefinitions () const
{
	return {};
}


constexpr const char* PublisherSetNameParameterField = "publisherSetName";
constexpr const char* OutputPathParameterField = "outputPath";
constexpr const char* ErrorMessageResponseField = "errorMessage";


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


GS::Optional<GS::UniString> PublishCommand::GetResponseSchema () const
{
	return {};
}


GS::ObjectState	PublishCommand::Execute (const GS::ObjectState& parameters, GS::ProcessControl& /*processControl*/) const
{
	GS::UniString publisherSetName;
	parameters.Get (PublisherSetNameParameterField, publisherSetName);

	const auto publisherSetNameGuidTable = GetPublisherSetNameGuidTable ();
	if (!publisherSetNameGuidTable.ContainsKey (publisherSetName)) {
		return GS::ObjectState (ErrorMessageResponseField, "Not valid publisher set name.");
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
		return GS::ObjectState (ErrorMessageResponseField, "Publishing failed. Check output path!");
	}

	return {};
}


void PublishCommand::OnResponseValidationFailed (const GS::ObjectState& /*response*/) const
{
}


// --- TeamworkReceiveCommand ----------------------------------------------------------------------------------

GS::String TeamworkReceiveCommand::GetName () const
{
	return "TeamworkReceive";
}


GS::String TeamworkReceiveCommand::GetNamespace () const
{
	return AdditionalJSONCommandsNamespace;
}


GS::Optional<GS::UniString> TeamworkReceiveCommand::GetSchemaDefinitions () const
{
	return {};
}


GS::Optional<GS::UniString> TeamworkReceiveCommand::GetInputParametersSchema () const
{
	return {};
}


GS::Optional<GS::UniString> TeamworkReceiveCommand::GetResponseSchema () const
{
	return {};
}


GS::ObjectState	TeamworkReceiveCommand::Execute (const GS::ObjectState& /*parameters*/, GS::ProcessControl& /*processControl*/) const
{
	GSErrCode err = ACAPI_TeamworkControl_ReceiveChanges ();

	if (err != NoError) {
		return GS::ObjectState (ErrorMessageResponseField, "Receive failed. Check internet connection!");
	}

	return {};
}


void TeamworkReceiveCommand::OnResponseValidationFailed (const GS::ObjectState& /*response*/) const
{
}


// --- GetProjectInfoCommand ----------------------------------------------------------------------------------

GS::String GetProjectInfoCommand::GetName () const
{
	return "GetProjectInfo";
}


GS::String GetProjectInfoCommand::GetNamespace () const
{
	return AdditionalJSONCommandsNamespace;
}


GS::Optional<GS::UniString> GetProjectInfoCommand::GetSchemaDefinitions () const
{
	return {};
}


constexpr const char* IsUntitledResponseField = "isUntitled";
constexpr const char* IsTeamworkResponseField = "isTeamwork";
constexpr const char* ProjectLocationResponseField = "projectLocation";
constexpr const char* ProjectPathResponseField = "projectPath";
constexpr const char* ProjectNameResponseField = "projectName";


GS::Optional<GS::UniString> GetProjectInfoCommand::GetInputParametersSchema () const
{
	return {};
}


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
		return GS::ObjectState (ErrorMessageResponseField, "Failed to retrieve project information. Check the opened project!");
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


void GetProjectInfoCommand::OnResponseValidationFailed (const GS::ObjectState& /*response*/) const
{
}


// --- GetArchicadLocationCommand ----------------------------------------------------------------------------------

GS::String GetArchicadLocationCommand::GetName () const
{
	return "GetArchicadLocation";
}


GS::String GetArchicadLocationCommand::GetNamespace () const
{
	return AdditionalJSONCommandsNamespace;
}


GS::Optional<GS::UniString> GetArchicadLocationCommand::GetSchemaDefinitions () const
{
	return {};
}


constexpr const char* ArchicadLocationResponseField = "archicadLocation";


GS::Optional<GS::UniString> GetArchicadLocationCommand::GetInputParametersSchema () const
{
	return {};
}


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
		return GS::ObjectState (ErrorMessageResponseField, "Failed to get the location of the Archicad application!");
	}

	return GS::ObjectState (ArchicadLocationResponseField, applicationFileLocation.Get ().ToDisplayText ());
}


void GetArchicadLocationCommand::OnResponseValidationFailed (const GS::ObjectState& /*response*/) const
{
}


// --- QuitCommand ----------------------------------------------------------------------------------

GS::String QuitCommand::GetName () const
{
	return "Quit";
}


GS::String QuitCommand::GetNamespace () const
{
	return AdditionalJSONCommandsNamespace;
}


GS::Optional<GS::UniString> QuitCommand::GetSchemaDefinitions () const
{
	return {};
}


GS::Optional<GS::UniString> QuitCommand::GetInputParametersSchema () const
{
	return {};
}


GS::Optional<GS::UniString> QuitCommand::GetResponseSchema () const
{
	return {};
}


GS::ObjectState	QuitCommand::Execute (const GS::ObjectState& /*parameters*/, GS::ProcessControl& /*processControl*/) const
{
	Int64 magicCode = 1234;
	GSErrCode err = ACAPI_Automate (APIDo_QuitID, reinterpret_cast<void*> (magicCode));

	if (err != NoError) {
		return GS::ObjectState (ErrorMessageResponseField, "Failed to quit Archicad!");
	}

	return {};
}


void QuitCommand::OnResponseValidationFailed (const GS::ObjectState& /*response*/) const
{
}