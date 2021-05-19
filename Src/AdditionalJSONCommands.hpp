#if !defined (ADDITIONALJSONCOMMANDS_HPP)
#define ADDITIONALJSONCOMMANDS_HPP

#pragma once

#include	"APIEnvir.h"
#include	"ACAPinc.h"


class PublishCommand : public API_AddOnCommand {
public:

	virtual GS::String							GetName () const override;
	virtual GS::String							GetNamespace () const override;
	virtual GS::Optional<GS::UniString>			GetSchemaDefinitions () const override;
	virtual GS::Optional<GS::UniString>			GetInputParametersSchema () const override;
	virtual GS::Optional<GS::UniString>			GetResponseSchema () const override;

	virtual API_AddOnCommandExecutionPolicy		GetExecutionPolicy () const override			{ return API_AddOnCommandExecutionPolicy::ScheduleForExecutionOnMainThread; }

	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;

	virtual void								OnResponseValidationFailed (const GS::ObjectState& response) const override;
};


class TeamworkReceiveCommand : public API_AddOnCommand {
public:

	virtual GS::String							GetName () const override;
	virtual GS::String							GetNamespace () const override;
	virtual GS::Optional<GS::UniString>			GetSchemaDefinitions () const override;
	virtual GS::Optional<GS::UniString>			GetInputParametersSchema () const override;
	virtual GS::Optional<GS::UniString>			GetResponseSchema () const override;

	virtual API_AddOnCommandExecutionPolicy		GetExecutionPolicy () const override { return API_AddOnCommandExecutionPolicy::ScheduleForExecutionOnMainThread; }

	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;

	virtual void								OnResponseValidationFailed (const GS::ObjectState& response) const override;
};


class GetProjectInfoCommand : public API_AddOnCommand {
public:

	virtual GS::String							GetName () const override;
	virtual GS::String							GetNamespace () const override;
	virtual GS::Optional<GS::UniString>			GetSchemaDefinitions () const override;
	virtual GS::Optional<GS::UniString>			GetInputParametersSchema () const override;
	virtual GS::Optional<GS::UniString>			GetResponseSchema () const override;

	virtual API_AddOnCommandExecutionPolicy		GetExecutionPolicy () const override { return API_AddOnCommandExecutionPolicy::ScheduleForExecutionOnMainThread; }

	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;

	virtual void								OnResponseValidationFailed (const GS::ObjectState& response) const override;
};


class GetArchicadLocationCommand : public API_AddOnCommand {
public:

	virtual GS::String							GetName () const override;
	virtual GS::String							GetNamespace () const override;
	virtual GS::Optional<GS::UniString>			GetSchemaDefinitions () const override;
	virtual GS::Optional<GS::UniString>			GetInputParametersSchema () const override;
	virtual GS::Optional<GS::UniString>			GetResponseSchema () const override;

	virtual API_AddOnCommandExecutionPolicy		GetExecutionPolicy () const override { return API_AddOnCommandExecutionPolicy::ScheduleForExecutionOnMainThread; }

	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;

	virtual void								OnResponseValidationFailed (const GS::ObjectState& response) const override;
};


class QuitCommand : public API_AddOnCommand {
public:

	virtual GS::String							GetName () const override;
	virtual GS::String							GetNamespace () const override;
	virtual GS::Optional<GS::UniString>			GetSchemaDefinitions () const override;
	virtual GS::Optional<GS::UniString>			GetInputParametersSchema () const override;
	virtual GS::Optional<GS::UniString>			GetResponseSchema () const override;

	virtual API_AddOnCommandExecutionPolicy		GetExecutionPolicy () const override { return API_AddOnCommandExecutionPolicy::ScheduleForExecutionOnMainThread; }

	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;

	virtual void								OnResponseValidationFailed (const GS::ObjectState& response) const override;
};


#endif