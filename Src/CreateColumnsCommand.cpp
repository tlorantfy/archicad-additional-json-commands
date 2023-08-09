#include	"CreateColumnsCommand.hpp"
#include	"ObjectState.hpp"
#include	"OnExit.hpp"


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
		return CreateErrorResponse (err, errorMsg);
	}

	return {};
}