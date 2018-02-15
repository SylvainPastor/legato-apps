#include "legato.h"
#include "interfaces.h"

#ifndef COMMAND_H
#define COMMAND_H

typedef struct _Command Command_t;

//--------------------------------------------------------------------------------------------------
/**
 * \brief Processing function.
 *
 * \return The result of command processing.
 */
 //--------------------------------------------------------------------------------------------------
typedef le_result_t (*cmdProcessingFunc_t)(Command_t* cmdPtr);

//--------------------------------------------------------------------------------------------------
/**
 * Command object.
 */
//--------------------------------------------------------------------------------------------------
typedef struct _Command
{
    uint32_t id;                              ///< Command identifier
    command_CmdRef_t cmdRef;                  ///< Command refrence.
    le_msg_SessionRef_t owningSession;        ///< Client session identifier.
    bool processing;                          ///< is command processing
    cmdProcessingFunc_t ProcessFunc;          ///< Internal processing function.
    le_result_t result;                       ///< Command result.
    command_CommandHandlerFunc_t handlerFunc; ///< Handler associated with the command.
    void* handlerContextPtr;                  ///< client handler context.
} Command_t;

#endif /* COMMAND_H */