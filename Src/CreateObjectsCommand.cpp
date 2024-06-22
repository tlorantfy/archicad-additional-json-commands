#include	"CreateObjectsCommand.hpp"
#include	"ObjectState.hpp"
#include	"OnExit.hpp"


GS::String CreateObjectsCommand::GetName () const
{
	return "CreateObjects";
}


constexpr const char* ObjectsParameterField = "objects";
constexpr const char* ObjectNameParameterField = "name";
constexpr const char* ObjectCoordinateParameterField = "coordinate";
constexpr const char* ObjectDimensionsParameterField = "dimensions";


GS::Optional<GS::UniString> CreateObjectsCommand::GetInputParametersSchema () const
{
	return GS::UniString::Printf (R"({
	"type": "object",
	"properties": {
		"%s": {
			"type": "array",
			"description": "The parameters of the new Objects.",
			"items": {
				"type": "object",
				"description" : "The parameters of the new Object.",
				"properties" : {
					"%s": {
						"type": "string",
						"description" : "The name of the object."	
					},
					"%s": {
						"type": "object",
						"description" : "3D coordinate.",
						"properties" : {
							"x": {
								"type": "number",
								"description" : "X value of the coordinate."
							},
							"y" : {
								"type": "number",
								"description" : "Y value of the coordinate."
							},
							"z" : {
								"type": "number",
								"description" : "Z value of the coordinate."
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
						"type": "object",
						"description" : "3D size.",
						"properties" : {
							"x": {
								"type": "number",
								"description" : "X dimension."
							},
							"y" : {
								"type": "number",
								"description" : "Y dimension."
							},
							"z" : {
								"type": "number",
								"description" : "Z dimension."
							}
						},
						"additionalProperties": false,
						"required" : [
							"x",
							"y",
							"z"
						]
					}
				},
				"additionalProperties": true,
				"required" : [
					"%s",
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
ObjectsParameterField,
ObjectNameParameterField,
ObjectCoordinateParameterField,
ObjectDimensionsParameterField,
ObjectNameParameterField,
ObjectCoordinateParameterField,
ObjectDimensionsParameterField,
ObjectsParameterField);
}


GS::ObjectState	CreateObjectsCommand::Execute (const GS::ObjectState& parameters, GS::ProcessControl& /*processControl*/) const
{
	GS::Array<GS::ObjectState> objects;
	parameters.Get (ObjectsParameterField, objects);

	GS::UniString name;
	API_Coord3D   apiCoordinate;
	const APIErrCodes err = (APIErrCodes) ACAPI_CallUndoableCommand ("Create Objects", [&] () -> GSErrCode {
		API_Element element = {};
		API_ElementMemo memo = {};
		const GS::OnExit guard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });

#ifdef ServerMainVers_2600
		element.header.type   = API_ObjectID;
#else
		element.header.typeID = API_ObjectID;
#endif
		APIErrCodes err = (APIErrCodes) ACAPI_Element_GetDefaults (&element, &memo);

		const GS::Array<GS::Pair<short, double>> storyLevels = Utilities::GetStoryLevels ();

		for (const GS::ObjectState& object : objects) {
			if (!object.Contains (ObjectNameParameterField) ||
				!object.Contains (ObjectCoordinateParameterField)) {
				continue;
			}

			API_LibPart libPart = {};
			GS::UniString uName;
			object.Get (ObjectNameParameterField, uName);
			GS::ucscpy (libPart.docu_UName, uName.ToUStr ());

			err = (APIErrCodes) ACAPI_LibPart_Search (&libPart, false);
			delete libPart.location;

			if (err != NoError) {
				return err;
			}

			element.object.libInd = libPart.index;

			GS::ObjectState coordinate;
			object.Get (ObjectCoordinateParameterField, coordinate);
			apiCoordinate = Utilities::Get3DCoordinateFromObjectState (coordinate);

			object.Get (ObjectDimensionsParameterField, coordinate);
			API_Coord3D dimensions = Utilities::Get3DCoordinateFromObjectState (coordinate);

			element.object.pos.x = apiCoordinate.x;
			element.object.pos.y = apiCoordinate.y;
			element.header.floorInd = Utilities::GetFloorIndexAndOffset (apiCoordinate.z, storyLevels, element.object.level);

			element.object.xRatio = dimensions.x;
			element.object.yRatio = dimensions.y;
			GS::ObjectState os;
			os.Add("value", dimensions.z);
			Utilities::ChangeParams(memo.params, {{"ZZYZX", os}});

			err = (APIErrCodes) ACAPI_Element_Create (&element, &memo);

			if (err != NoError) {
				return err;
			}
		}

		return NoError;
	});

	if (err != NoError) {
		const GS::UniString errorMsg = GS::UniString::Printf ("Failed to create object with name {%T} to coordinate: {%.2f, %.2f, %.2f}!", name.ToPrintf(), apiCoordinate.x, apiCoordinate.y, apiCoordinate.z);
		return CreateErrorResponse (err, errorMsg);
	}

	return {};
}