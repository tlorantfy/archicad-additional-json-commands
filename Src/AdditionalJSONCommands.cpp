#include	"AdditionalJSONCommands.hpp"
#include	"ObjectState.hpp"
#include	"FileSystem.hpp"
#include	"OnExit.hpp"


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

#ifdef ServerMainVers_2600
bool AdditionalJSONCommand::IsProcessWindowVisible () const
{
	return false;
}
#endif


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


// --- ReloadLibrariesCommand ----------------------------------------------------------------------------------

GS::String ReloadLibrariesCommand::GetName () const
{
	return "ReloadLibraries";
}


GS::ObjectState	ReloadLibrariesCommand::Execute (const GS::ObjectState& /*parameters*/, GS::ProcessControl& /*processControl*/) const
{
	GSErrCode err = ACAPI_Automate (APIDo_ReloadLibrariesID);

	if (err != NoError) {
		return CreateErrorResponse (APIERR_COMMANDFAILED, "Reloading Libraries failed. Check internet connection if you have active libraries from BIMcloud!");
	}

	return {};
}


// --- MoveElementsCommand ----------------------------------------------------------------------------------

GS::String MoveElementsCommand::GetName () const
{
	return "MoveElements";
}


constexpr const char* ElementsWithMoveVectorsParameterField = "elementsWithMoveVectors";
constexpr const char* ElementIdField = "elementId";
constexpr const char* GuidField = "guid";
constexpr const char* MoveVectorField = "moveVector";
constexpr const char* CopyField = "copy";


GS::Optional<GS::UniString> MoveElementsCommand::GetInputParametersSchema () const
{
	return GS::UniString::Printf (R"({
	"type": "object",
	"properties": {
		"%s": {
			"type": "array",
			"description": "The elements with move vector pairs.",
			"items": {
				"type": "object",
				"properties": {
					"%s": {
						"$ref": "APITypes.json#/definitions/ElementId"
					},
					"%s": {
						"type": "object",
						"description" : "Move vector of a 3D point.",
						"properties" : {
							"x": {
								"type": "number",
								"description" : "X value of the vector."
							},
							"y" : {
								"type": "number",
								"description" : "Y value of the vector."
							},
							"z" : {
								"type": "number",
								"description" : "Z value of the vector."
							}
						},
						"additionalProperties": false,
						"required" : [
							"x",
							"y",
							"z"
						]
					},
					"%s": {
						"type": "boolean",
						"description" : "Optional parameter. If true, then a copy of the element will be moved. By default it's false."
					}
				},
				"additionalProperties": false,
				"required": [
					"%s",
					"%s"
				]
			}
		}
	},
	"additionalProperties": false,
	"required": [
	"%s"
	]
})",
ElementsWithMoveVectorsParameterField,
ElementIdField,
MoveVectorField,
CopyField,
ElementIdField, MoveVectorField,
ElementsWithMoveVectorsParameterField);
}


static API_Guid GetGuidFromElementIdField (const GS::ObjectState& os)
{
	GS::String guid;
	os.Get (GuidField, guid);
	return APIGuidFromString (guid.ToCStr ());
}


static API_Vector3D GetVector3DFromMoveVectorField (const GS::ObjectState& os)
{
	API_Vector3D v;
	os.Get ("x", v.x);
	os.Get ("y", v.y);
	os.Get ("z", v.z);
	return v;
}


static GSErrCode MoveElement (const API_Guid& elemGuid, const API_Vector3D& moveVector, bool withCopy)
{
	GS::Array<API_Neig> elementsToEdit = { API_Neig (elemGuid) };

	API_EditPars editPars = {};
	editPars.typeID = APIEdit_Drag;
	editPars.endC = moveVector;
	editPars.withDelete = !withCopy;

	return ACAPI_Element_Edit (&elementsToEdit, editPars);
}


GS::ObjectState	MoveElementsCommand::Execute (const GS::ObjectState& parameters, GS::ProcessControl& /*processControl*/) const
{
	GS::Array<GS::ObjectState> elementsWithMoveVectors;
	parameters.Get (ElementsWithMoveVectorsParameterField, elementsWithMoveVectors);

	API_Guid elemGuid;
	const APIErrCodes err = (APIErrCodes) ACAPI_CallUndoableCommand ("Move Elements", [&] () -> GSErrCode {
		for (const GS::ObjectState& elementWithMoveVector : elementsWithMoveVectors) {
			const GS::ObjectState* elementId = elementWithMoveVector.Get (ElementIdField);
			const GS::ObjectState* moveVector = elementWithMoveVector.Get (MoveVectorField);
			if (elementId == nullptr || moveVector == nullptr) {
				continue;
			}

			elemGuid = GetGuidFromElementIdField (*elementId);

			bool copy = false;
			elementWithMoveVector.Get (CopyField, copy);

			const GSErrCode err = MoveElement (elemGuid,
											   GetVector3DFromMoveVectorField (*moveVector),
											   copy);
			if (err != NoError) {
				return err;
			}
		}

		return NoError;
	});

	if (err != NoError) {
		const GS::UniString errorMsg = GS::UniString::Printf ("Failed to move element with guid %T!", APIGuidToString (elemGuid).ToPrintf ());
		return CreateErrorResponse (err, errorMsg.ToCStr ().Get ());
	}

	return {};
}


// --- CreateColumnsCommand ----------------------------------------------------------------------------------

GS::String CreateColumnsCommand::GetName () const
{
	return "CreateColumns";
}


constexpr const char* CoordinatesParameterField = "coordinates";


GS::Optional<GS::UniString> CreateColumnsCommand::GetInputParametersSchema () const
{
	return GS::UniString::Printf (R"({
	"type": "object",
	"properties": {
		"%s": {
			"type": "array",
			"description": "The 2D coordinates of the new columns.",
			"items": {
				"type": "object",
				"description" : "Position of a 2D point.",
				"properties" : {
					"x": {
						"type": "number",
						"description" : "X value of the point."
					},
					"y" : {
						"type": "number",
						"description" : "Y value of the point."
					}
				},
				"additionalProperties": false,
				"required" : [
					"x",
					"y"
				]
			}
		}
	},
	"additionalProperties": false,
	"required": [
	"%s"
	]
})",
CoordinatesParameterField,
CoordinatesParameterField);
}


static API_Coord GetCoordinateFromObjectState (const GS::ObjectState& objectState)
{
	API_Coord coordinate = {};
	objectState.Get ("x", coordinate.x);
	objectState.Get ("y", coordinate.y);
	return coordinate;
}


GS::ObjectState	CreateColumnsCommand::Execute (const GS::ObjectState& parameters, GS::ProcessControl& /*processControl*/) const
{
	GS::Array<GS::ObjectState> coordinates;
	parameters.Get (CoordinatesParameterField, coordinates);

	API_Coord apiCoordinate = {};
	const APIErrCodes err = (APIErrCodes) ACAPI_CallUndoableCommand ("Create Columns", [&] () -> GSErrCode {
		API_Element element = {};
		API_ElementMemo memo = {};
		const GS::OnExit guard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });

		#ifdef ServerMainVers_2600
		element.header.type = API_ElemType(API_ColumnID); // AC26
		#else
		element.header.typeID = API_ColumnID; // AC25
		#endif
		APIErrCodes err = (APIErrCodes) ACAPI_Element_GetDefaults (&element, &memo);

		for (const GS::ObjectState& coordinate : coordinates) {
			apiCoordinate = GetCoordinateFromObjectState (coordinate);

			element.column.origoPos = apiCoordinate;
			err = (APIErrCodes) ACAPI_Element_Create (&element, &memo);

			if (err != NoError) {
				return err;
			}
		}

		return NoError;
	});

	if (err != NoError) {
		const GS::UniString errorMsg = GS::UniString::Printf ("Failed to create column to coordinate: {%.2f, %.2f}!", apiCoordinate.x, apiCoordinate.y);
		return CreateErrorResponse (err, errorMsg.ToCStr ().Get ());
	}

	return {};
}


// --- GetHotlinksCommand ----------------------------------------------------------------------------------

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