#if !defined (GETPROJECTINFOCOMMAND_HPP)
#define GETPROJECTINFOCOMMAND_HPP

#pragma once

#include	"AdditionalJSONCommand.hpp"


class GetProjectInfoCommand : public AdditionalJSONCommand {
public:
	virtual GS::String							GetName () const override;
	virtual GS::Optional<GS::UniString>			GetResponseSchema () const override;
	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;
};


#endif