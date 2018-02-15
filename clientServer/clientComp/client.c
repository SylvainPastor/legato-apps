#include "legato.h"
#include "interfaces.h"

static command_CmdRef_t CmdARef = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * Handler for command.
 */
//--------------------------------------------------------------------------------------------------
static void CommandHandler(
    command_CmdRef_t cmdRef, ///< Received command reference
    le_result_t result, ///> Command result
    void* contextPtr ///< Command context
)
{
	int errCode = EXIT_FAILURE;

	LE_INFO("Command <%p> result: %s", cmdRef, LE_RESULT_TXT(result));
	
	if (LE_OK == result)
	{
		errCode = EXIT_SUCCESS;
	}

	exit(errCode);
}

COMPONENT_INIT
{
	CmdARef = command_Create(COMMAND_A);
	LE_ASSERT(CmdARef);

	LE_ASSERT(
		command_AddCommandHandler(CmdARef, CommandHandler, NULL));

	command_Execute(CmdARef); // The result in 5 seconds
}
