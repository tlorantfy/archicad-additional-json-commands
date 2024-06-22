#include	"CreateRailingsCommand.hpp"
#include	"ObjectState.hpp"
#include	"OnExit.hpp"


GS::String CreateRailingsCommand::GetName () const
{
	return "CreateRailings";
}


constexpr const char* RailingsParameterField = "railings";
constexpr const char* LevelParameterField = "level";
constexpr const char* PolygonCoordinatesParameterField = "polygonCoordinates";
constexpr const char* HolesParameterField = "holes";


GS::Optional<GS::UniString> CreateRailingsCommand::GetInputParametersSchema () const
{
	return GS::UniString::Printf (R"({
	"type": "object",
	"properties": {
		"%s": {
			"type": "array",
			"description": "The parameters of the new Railings.",
			"items": {
				"type": "object",
				"description" : "The parameters of the new Railing.",
				"properties" : {
					"%s": {
						"type": "number",
						"description" : "The Z coordinate value of the reference line of the railing."	
					},
					"%s": { 
						"type": "array",
						"description": "The 2D coordinates of the edge of the railing.",
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
				"additionalProperties": true,
				"required" : [
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
RailingsParameterField,
LevelParameterField,
PolygonCoordinatesParameterField,
LevelParameterField,
PolygonCoordinatesParameterField,
RailingsParameterField);
}


static void AddPolyToMemo (const GS::Array<GS::ObjectState>& polygonCoordinates,
						   const API_OverriddenAttribute&	 sideMat,
						   Int32& 							 iCoord,
						   Int32& 							 iPends,
						   API_ElementMemo& 				 memo)
{
	Int32 iStart = iCoord;
	for (const GS::ObjectState& polygonCoordinate : polygonCoordinates) {
		(*memo.coords)[iCoord] = Utilities::Get2DCoordinateFromObjectState (polygonCoordinate);
		(*memo.edgeTrims)[iCoord].sideType = APIEdgeTrim_Vertical; // Only vertical trim is supported yet by my code
		memo.sideMaterials[iCoord] = sideMat;
		++iCoord;
	}
	(*memo.coords)[iCoord] = (*memo.coords)[iStart];
	(*memo.pends)[iPends++] = iCoord;
	(*memo.edgeTrims)[iCoord].sideType = (*memo.edgeTrims)[iStart].sideType;
	(*memo.edgeTrims)[iCoord].sideAngle = (*memo.edgeTrims)[iStart].sideAngle;
	memo.sideMaterials[iCoord] = memo.sideMaterials[iStart];
	++iCoord;
}


GS::ObjectState	CreateRailingsCommand::Execute (const GS::ObjectState& parameters, GS::ProcessControl& /*processControl*/) const
{
	GS::Array<GS::ObjectState> railings;
	parameters.Get (RailingsParameterField, railings);

	GSIndex iRailing = 0;
	const APIErrCodes err = (APIErrCodes) ACAPI_CallUndoableCommand ("Create Railings", [&] () -> GSErrCode {
		APIErrCodes err = (APIErrCodes) NoError;
		const GS::Array<GS::Pair<short, double>> storyLevels = Utilities::GetStoryLevels ();
		for (const GS::ObjectState& railing : railings) {
			if (!railing.Contains (LevelParameterField) ||
				!railing.Contains (PolygonCoordinatesParameterField)) {
				continue;
			}

			API_Element element = {};
			API_ElementMemo memo = {};
			const GS::OnExit guard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });

#ifdef ServerMainVers_2600
			element.header.type   = API_RailingID;
#else
			element.header.typeID = API_RailingID;
#endif
			err = (APIErrCodes) ACAPI_Element_GetDefaults (&element, &memo);

			railing.Get(LevelParameterField, element.railing.bottomOffset);
			element.header.floorInd = Utilities::GetFloorIndexAndOffset (element.railing.bottomOffset, storyLevels, element.railing.bottomOffset);

			GS::Array<GS::ObjectState> polygonCoordinates;
			GS::Array<GS::ObjectState> holes;
			railing.Get (PolygonCoordinatesParameterField, polygonCoordinates);
			if (railing.Contains (HolesParameterField)) {
				railing.Get (HolesParameterField, holes);
			}
			API_Polygon poly = {};
			poly.nCoords	= polygonCoordinates.GetSize() + 1;
			poly.nSubPolys	= 1;

			memo.coords = reinterpret_cast<API_Coord**> (BMAllocateHandle ((poly.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0));
			memo.edgeTrims = reinterpret_cast<API_EdgeTrim**> (BMAllocateHandle ((poly.nCoords + 1) * sizeof (API_EdgeTrim), ALLOCATE_CLEAR, 0));
			memo.sideMaterials = reinterpret_cast<API_OverriddenAttribute*> (BMAllocatePtr ((poly.nCoords + 1) * sizeof (API_OverriddenAttribute), ALLOCATE_CLEAR, 0));
			memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle ((poly.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0));

			element.railing.nVertices = poly.nCoords;

			Int32 iCoord = 1;
			Int32 iPends = 1;
			AddPolyToMemo(polygonCoordinates,
						  API_OverriddenAttribute(),
						  iCoord,
						  iPends,
						  memo);

			err = (APIErrCodes) ACAPI_Element_Create (&element, &memo);

			if (err != NoError) {
				break;
			}

			++iRailing;
		}

		return err;
	});

	if (err != NoError) {
		const GS::UniString errorMsg = GS::UniString::Printf ("Failed to create railing #%d!", iRailing);
		return CreateErrorResponse (err, errorMsg);
	}

	return {};
}