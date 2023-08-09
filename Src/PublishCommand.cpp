#include	"PublishCommand.hpp"
#include	"ObjectState.hpp"


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