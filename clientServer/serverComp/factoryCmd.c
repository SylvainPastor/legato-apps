#include "factoryCmd.h"

// Memory pool for allocating commands
static le_mem_PoolRef_t CommandPool;

//--------------------------------------------------------------------------------------------------
/**
 * Returns an auto increment id.
 */
//--------------------------------------------------------------------------------------------------
static uint32_t _Id()
{
    static uint32_t Id = 0;

    if (UINT32_MAX == Id)
        Id = 0;
    return ++Id;
}

//--------------------------------------------------------------------------------------------------
/**
 * Processing functions.
 */
//--------------------------------------------------------------------------------------------------

static le_result_t CmdA_Processing(Command_t* CmdAPtr)
{
    LE_INFO("Processing command <%d>", CmdAPtr->id);
    sleep(5);
    return LE_OK;
}

static le_result_t CmdB_Processing(Command_t* CmdBPtr)
{
    LE_INFO("Processing command <%d>", CmdBPtr->id);
    sleep(5);
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Create a new command object.
 */
//--------------------------------------------------------------------------------------------------
static Command_t* Create()
{
    Command_t* newCmdPtr = le_mem_ForceAlloc(CommandPool);
    newCmdPtr->id = _Id();
    newCmdPtr->owningSession = command_GetClientSessionRef();
    newCmdPtr->handlerFunc = NULL;
    newCmdPtr->handlerContextPtr = NULL;
    return newCmdPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Create a command
 */
//--------------------------------------------------------------------------------------------------
Command_t* factoryCmd_Create(
    command_Type_t type)
{
    Command_t* CmdPtr = Create(); 
    if (CmdPtr)
    {
        switch(type)
        {
            case COMMAND_A:
                CmdPtr->ProcessFunc = CmdA_Processing;
                break;

            case COMMAND_B:
                CmdPtr->ProcessFunc = CmdB_Processing;
                break;
        }

    }
    return CmdPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Factory initialization
 */
//--------------------------------------------------------------------------------------------------
void factoryCmd_Init()
{
    CommandPool = le_mem_CreatePool("Command Pool", sizeof(Command_t));
    LE_ASSERT(CommandPool);
}