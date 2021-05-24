#if !defined (ADDITIONALJSONCOMMANDS_HPP)
#define ADDITIONALJSONCOMMANDS_HPP

#pragma once

#include	"APIEnvir.h"
#include	"ACAPinc.h"


class AdditionalJSONCommand : public API_AddOnCommand {
public:
	virtual GS::String							GetNamespace () const override;
	virtual GS::Optional<GS::UniString>			GetSchemaDefinitions () const override;
	virtual GS::Optional<GS::UniString>			GetInputParametersSchema () const override;
	virtual GS::Optional<GS::UniString>			GetResponseSchema () const override;
	virtual API_AddOnCommandExecutionPolicy		GetExecutionPolicy () const override			{ return API_AddOnCommandExecutionPolicy::ScheduleForExecutionOnMainThread; }
	virtual void								OnResponseValidationFailed (const GS::ObjectState& response) const override;
};


class PublishCommand : public AdditionalJSONCommand {
public:
	virtual GS::String							GetName () const override;
	virtual GS::Optional<GS::UniString>			GetInputParametersSchema () const override;
	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;
};


class TeamworkReceiveCommand : public AdditionalJSONCommand {
public:
	virtual GS::String							GetName () const override;
	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;
};


class GetProjectInfoCommand : public AdditionalJSONCommand {
public:
	virtual GS::String							GetName () const override;
	virtual GS::Optional<GS::UniString>			GetResponseSchema () const override;
	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;
};


class GetArchicadLocationCommand : public AdditionalJSONCommand {
public:
	virtual GS::String							GetName () const override;
	virtual GS::Optional<GS::UniString>			GetResponseSchema () const override;
	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;
};


class QuitCommand : public AdditionalJSONCommand {
public:
	virtual GS::String							GetName () const override;
	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;
};


#endif