#include	"CreateSlabsCommand.hpp"
#include	"ObjectState.hpp"
#include	"OnExit.hpp"


GS::String CreateSlabsCommand::GetName () const
{
	return "CreateSlabs";
}


constexpr const char* SlabsParameterField = "slabs";
constexpr const char* LevelParameterField = "level";
constexpr const char* PolygonCoordinatesParameterField = "polygonCoordinates";
constexpr const char* HolesParameterField = "holes";


GS::Optional<GS::UniString> CreateSlabsCommand::GetInputParametersSchema () const
{
	return GS::UniString::Printf (R"({
	"type": "object",
	"properties": {
		"%s": {
			"type": "array",
			"description": "The parameters of the new Slabs.",
			"items": {
				"type": "object",
				"description" : "The parameters of the new Slab.",
				"properties" : {
					"%s": {
						"type": "number",
						"description" : "The Z coordinate value of the reference line of the slab."	
					},
					"%s": { 
						"type": "array",
						"description": "The 2D coordinates of the edge of the slab.",
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
SlabsParameterField,
LevelParameterField,
PolygonCoordinatesParameterField,
LevelParameterField,
PolygonCoordinatesParameterField,
SlabsParameterField);
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


GS::ObjectState	CreateSlabsCommand::Execute (const GS::ObjectState& parameters, GS::ProcessControl& /*processControl*/) const
{
	GS::Array<GS::ObjectState> slabs;
	parameters.Get (SlabsParameterField, slabs);

	GSIndex iSlab = 0;
	const APIErrCodes err = (APIErrCodes) ACAPI_CallUndoableCommand ("Create Slabs", [&] () -> GSErrCode {
		APIErrCodes err = (APIErrCodes) NoError;
		const GS::Array<GS::Pair<short, double>> storyLevels = Utilities::GetStoryLevels ();
		for (const GS::ObjectState& slab : slabs) {
			if (!slab.Contains (LevelParameterField) ||
				!slab.Contains (PolygonCoordinatesParameterField)) {
				continue;
			}

			API_Element element = {};
			API_ElementMemo memo = {};
			const GS::OnExit guard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });

#ifdef ServerMainVers_2600
			element.header.type   = API_SlabID;
#else
			element.header.typeID = API_SlabID;
#endif
			err = (APIErrCodes) ACAPI_Element_GetDefaults (&element, &memo);

			slab.Get(LevelParameterField, element.slab.level);
			element.header.floorInd = Utilities::GetFloorIndexAndOffset (element.slab.level, storyLevels, element.slab.level);

			GS::Array<GS::ObjectState> polygonCoordinates;
			GS::Array<GS::ObjectState> holes;
			slab.Get (PolygonCoordinatesParameterField, polygonCoordinates);
			if (slab.Contains (HolesParameterField)) {
				slab.Get (HolesParameterField, holes);
			}
			element.slab.poly.nCoords	= polygonCoordinates.GetSize() + 1;
			element.slab.poly.nSubPolys	= 1;
			element.slab.poly.nArcs		= 0; // Curved edges are not supported yet by my code

			for (const GS::ObjectState& hole : holes) {
				if (!hole.Contains (PolygonCoordinatesParameterField)) {
					continue;
				}
				GS::Array<GS::ObjectState> holePolygonCoordinates;
				hole.Get (PolygonCoordinatesParameterField, holePolygonCoordinates);
				element.slab.poly.nCoords += holePolygonCoordinates.GetSize() + 1;
				++element.slab.poly.nSubPolys;
			}

			memo.coords = reinterpret_cast<API_Coord**> (BMAllocateHandle ((element.slab.poly.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0));
			memo.edgeTrims = reinterpret_cast<API_EdgeTrim**> (BMAllocateHandle ((element.slab.poly.nCoords + 1) * sizeof (API_EdgeTrim), ALLOCATE_CLEAR, 0));
			memo.sideMaterials = reinterpret_cast<API_OverriddenAttribute*> (BMAllocatePtr ((element.slab.poly.nCoords + 1) * sizeof (API_OverriddenAttribute), ALLOCATE_CLEAR, 0));
			memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle ((element.slab.poly.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0));

			Int32 iCoord = 1;
			Int32 iPends = 1;
			AddPolyToMemo(polygonCoordinates,
						  element.slab.sideMat,
						  iCoord,
						  iPends,
						  memo);

			for (const GS::ObjectState& hole : holes) {
				if (!hole.Contains (PolygonCoordinatesParameterField)) {
					continue;
				}
				GS::Array<GS::ObjectState> holePolygonCoordinates;
				hole.Get (PolygonCoordinatesParameterField, holePolygonCoordinates);

				AddPolyToMemo(holePolygonCoordinates,
							element.slab.sideMat,
							iCoord,
							iPends,
							memo);
			}

			err = (APIErrCodes) ACAPI_Element_Create (&element, &memo);

			if (err != NoError) {
				break;
			}

			++iSlab;
		}

		return err;
	});

	if (err != NoError) {
		const GS::UniString errorMsg = GS::UniString::Printf ("Failed to create slab #%d!", iSlab);
		return CreateErrorResponse (err, errorMsg);
	}

	return {};
}