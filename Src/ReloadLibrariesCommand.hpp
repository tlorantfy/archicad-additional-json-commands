#if !defined (RELOADLIBRARIESCOMMAND_HPP)
#define RELOADLIBRARIESCOMMAND_HPP

#pragma once

#include	"AdditionalJSONCommand.hpp"


class ReloadLibrariesCommand : public AdditionalJSONCommand {
public:
	virtual GS::String							GetName () const override;
	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;
};


#endif