#if !defined (GETGDLPARAMETERSOFELEMENTSCOMMAND_HPP)
#define GETGDLPARAMETERSOFELEMENTSCOMMAND_HPP

#pragma once

#include	"AdditionalJSONCommand.hpp"


class GetGDLParametersOfElementsCommand : public AdditionalJSONCommand {
public:
	virtual GS::String							GetName () const override;
	virtual GS::Optional<GS::UniString>			GetSchemaDefinitions () const override;
	virtual GS::Optional<GS::UniString>			GetInputParametersSchema () const override;
	virtual GS::Optional<GS::UniString>			GetResponseSchema () const override;
	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;
};


#endif