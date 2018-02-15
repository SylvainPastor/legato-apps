#include "legato.h"
#include "interfaces.h"
#include "worker.h"
#include "factoryCmd.h"

#define MAX_COMMANDS (20)

// A map of safe references to command objects
static le_ref_MapRef_t CommandRefMap;

//--------------------------------------------------------------------------------------------------
/**
 * A handler for client disconnects which frees all resources associated with the client.
 */
//--------------------------------------------------------------------------------------------------
static void _clientSessionClosedHandler(
    le_msg_SessionRef_t clientSessionRef,
    void* context
)
{
    Command_t *cmdPtr = NULL;
    le_ref_IterRef_t iterRef = le_ref_GetIterator(CommandRefMap);
    while (LE_OK == le_ref_NextNode(iterRef))
    {
        cmdPtr = (Command_t*) le_ref_GetValue(iterRef);
        if (cmdPtr)
        {
            if (clientSessionRef == cmdPtr->owningSession)
            {
                LE_INFO("Deleting Command <ref=%p, id=%d>", cmdPtr->cmdRef, cmdPtr->id);

                // Release Command.
                le_mem_Release(cmdPtr);

                // Delete the command safeRef.
                le_ref_DeleteRef(CommandRefMap, (void*)le_ref_GetSafeRef(iterRef));
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function created a command and register it.
 *
 * @return
 *      - Reference to the command.
 *      - NULL if an error occurs.
 */
//--------------------------------------------------------------------------------------------------
command_CmdRef_t command_Create(
    command_Type_t type)
{
    command_CmdRef_t cmdRef = NULL;
       
    Command_t *newCmdPtr = factoryCmd_Create(type);
    if (newCmdPtr)
    {
        cmdRef = le_ref_CreateRef(CommandRefMap, newCmdPtr);
        newCmdPtr->cmdRef = cmdRef;
        LE_INFO("Command <ref=%p, id=%d> created", cmdRef, newCmdPtr->id);
    }

    return cmdRef;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function deletes a command.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to delete the command.
 *      - LE_BUSY          Command is in progress.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t command_Delete(
    command_CmdRef_t cmdRef ///< Command reference
)
{
    Command_t* cmdPtr = le_ref_Lookup(CommandRefMap, cmdRef);
    if (!cmdPtr) 
    {
        LE_ERROR("Invalid commande reference (%p) provided!", cmdRef);
        return LE_FAULT;
    }

    if (cmdPtr->processing) 
    {
        LE_ERROR("Command <%d> currently in processing", cmdPtr->id);
        return LE_BUSY;
    }

    // Delete the Command safeRef.
    le_ref_DeleteRef(CommandRefMap, cmdRef);

    // Release Command.
    le_mem_Release(cmdPtr);
    
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Add handler function for EVENT 'command_Command'
 *
 * This event provides information when the command is detected.
 *
 */
//--------------------------------------------------------------------------------------------------
command_CommandHandlerRef_t command_AddCommandHandler(
    command_CmdRef_t cmdRef, ///< [IN] Command reference
    command_CommandHandlerFunc_t handlerPtr, ///< [IN] Handler to called when the command is detected
    void* contextPtr ///< [IN] Context pointer
)
{
    Command_t* cmdPtr = le_ref_Lookup(CommandRefMap, cmdRef);
    if (!cmdPtr)
    {
        LE_ERROR("Invalid commande reference (%p) provided!", cmdRef);
        return NULL;
    }

    if (cmdPtr->handlerFunc)
    {
        LE_INFO("Command <%d>: Handler already exists", cmdPtr->id);
        return NULL;
    }

    cmdPtr->handlerFunc = handlerPtr;
    cmdPtr->handlerContextPtr = contextPtr;
    return (command_CommandHandlerRef_t)(cmdRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * Remove handler function for EVENT 'command_Command'
 */
//--------------------------------------------------------------------------------------------------
void command_RemoveCommandHandler(
    command_CommandHandlerRef_t handlerRef
)
{
    if (handlerRef)
    {
        Command_t* cmdPtr = le_ref_Lookup(CommandRefMap, handlerRef);
        if (cmdPtr)
        {
            cmdPtr->handlerFunc = NULL;
            cmdPtr->handlerContextPtr = NULL;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Execute a commande.
 */
//--------------------------------------------------------------------------------------------------
le_result_t command_Execute(
    command_CmdRef_t cmdRef ///< [IN] Command reference
)
{
    Command_t* cmdPtr = le_ref_Lookup(CommandRefMap, cmdRef);
    if (cmdPtr == NULL)
    {
        LE_ERROR("Invalid commande reference (%p) provided!", cmdRef);
        return LE_FAULT;
    }

    if (cmdPtr->processing) 
    {
        LE_ERROR("Command <%d> currently in processing", cmdPtr->id);
        return LE_BUSY;
    }

    worker_QueueCommand(cmdPtr);
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Component initialisation
 */
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
    LE_INFO("Server starting...");

    // Command factory initialisation
    factoryCmd_Init();

    // Worker initialisation
    worker_Init();

    CommandRefMap = le_ref_CreateMap("PendingCommands", MAX_COMMANDS);
    LE_ASSERT(CommandRefMap);

    // Register a handler to be notified when clients disconnect
    le_msg_AddServiceCloseHandler(
        command_GetServiceRef(), _clientSessionClosedHandler, NULL);

    LE_INFO("Server started");
}
