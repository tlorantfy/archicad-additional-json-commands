#include	"GetHotlinksCommand.hpp"
#include	"ObjectState.hpp"


GS::String GetHotlinksCommand::GetName () const
{
	return "GetHotlinks";
}


constexpr const char* HotlinksSchemaName = "Hotlinks";
constexpr const char* HotlinkSchemaName = "Hotlink";
constexpr const char* LocationResponseField = "location";
constexpr const char* ChildrenResponseField = "children";
constexpr const char* HotlinksResponseField = "hotlinks";


GS::Optional<GS::UniString> GetHotlinksCommand::GetSchemaDefinitions () const
{
	return GS::UniString::Printf (R"({
    "%s": {
      "type": "array",
      "description": "A list of hotlink nodes.",
      "items": {
        "$ref": "#/%s"
      }
    },
    "%s": {
      "type": "object",
      "description": "The details of a hotlink node.",
      "properties": {
        "%s": {
          "type": "string",
          "description": "The path of the hotlink file."
        },
        "%s": {
          "$ref": "#/%s",
          "description": "The children of the hotlink node if it has any."
        }
      },
      "additionalProperties": false,
      "required": [
        "%s"
      ]
    }
})",
HotlinksSchemaName,
HotlinkSchemaName, HotlinkSchemaName,
LocationResponseField, ChildrenResponseField,
HotlinksSchemaName,
LocationResponseField
);
}


GS::Optional<GS::UniString> GetHotlinksCommand::GetResponseSchema () const
{
	return GS::UniString::Printf (R"({
	"type": "object",
	"properties": {
		"%s": {
        	"$ref": "#/%s"
		}
	},
	"additionalProperties": false,
	"required": [
		"%s"
	]
})",
HotlinksResponseField,
HotlinkSchemaName,
HotlinksResponseField);
}


static GS::Optional<GS::UniString>	GetLocationOfHotlink (const API_Guid& hotlinkGuid)
{
	API_HotlinkNode hotlinkNode = {};
	hotlinkNode.guid = hotlinkGuid;

	ACAPI_Database (APIDb_GetHotlinkNodeID, &hotlinkNode);

	if (hotlinkNode.sourceLocation == nullptr) {
		return GS::NoValue;
	}
	
	return hotlinkNode.sourceLocation->ToDisplayText ();
}


static GS::ObjectState	DumpHotlinkWithChildren (const API_Guid&								hotlinkGuid,
												 GS::HashTable<API_Guid, GS::Array<API_Guid>>&	hotlinkTree)
{
	GS::ObjectState hotlinkNodeOS;

	const auto& location = GetLocationOfHotlink (hotlinkGuid);
	if (location.HasValue ()) {
		hotlinkNodeOS.Add (LocationResponseField, location.Get ());
	}

	const auto& children = hotlinkTree.Retrieve (hotlinkGuid);
	if (!children.IsEmpty ()) {
		const auto& listAdder = hotlinkNodeOS.AddList<GS::ObjectState> (ChildrenResponseField);
		for (const API_Guid& childNodeGuid : hotlinkTree.Retrieve (hotlinkGuid)) {
			listAdder (DumpHotlinkWithChildren (childNodeGuid, hotlinkTree));
		}
	}

	return hotlinkNodeOS;
}


GS::ObjectState	GetHotlinksCommand::Execute (const GS::ObjectState& /*parameters*/, GS::ProcessControl& /*processControl*/) const
{
	GS::ObjectState response;
	const auto& listAdder = response.AddList<GS::ObjectState> (HotlinksResponseField);

	for (API_HotlinkTypeID type : {APIHotlink_Module, APIHotlink_XRef}) {
		API_Guid hotlinkRootNodeGuid = APINULLGuid;
		if (ACAPI_Database (APIDb_GetHotlinkRootNodeGuidID, &type, &hotlinkRootNodeGuid) == NoError) {
			GS::HashTable<API_Guid, GS::Array<API_Guid>> hotlinkTree;
			if (ACAPI_Database (APIDb_GetHotlinkNodeTreeID, &hotlinkRootNodeGuid, &hotlinkTree) == NoError) {
				for (const API_Guid& childNodeGuid : hotlinkTree.Retrieve (hotlinkRootNodeGuid)) {
					listAdder (DumpHotlinkWithChildren (childNodeGuid, hotlinkTree));
				}
			}
		}
	}

	return response;
}