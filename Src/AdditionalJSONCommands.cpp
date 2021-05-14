#include	"AdditionalJSONCommands.hpp"
#include	"ObjectState.hpp"


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


GS::Optional<GS::UniString> PublishCommand::GetInputParametersSchema () const
{
	return R"({
	"type": "object",
	"properties": {
		"publisherSetName": {
			"type": "string",
			"description": "The name of the publisher set.",
			"minLength": 1
		},
		"outputPath": {
			"type": "string",
			"description": "Full local or LAN path for publishing. Optional, by default the path set in the settings of the publiser set will be used.",
			"minLength": 1
		}
	},
	"additionalProperties": false,
	"required": [
		"publisherSetName"
	]
})";
}


GS::Optional<GS::UniString> PublishCommand::GetResponseSchema () const
{
	return {};
}


constexpr const char* PublisherSetNameParameterField = "publisherSetName";
constexpr const char* OutputPathParameterField = "outputPath";
constexpr const char* ErrorMessageResponseField = "errorMessage";


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