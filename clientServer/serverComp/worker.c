#include "worker.h"
#include "cmd.h"

//--------------------------------------------------------------------------------------------------
/**
 *  Bridge context structure.
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    le_thread_Ref_t                             threadRef;          ///< worker thread reference
    le_thread_Ref_t                             mainThreadRef;      ///< main thread reference
}
Worker_t;

static Worker_t* workerPtr;

//--------------------------------------------------------------------------------------------------
/**
 * Treat Response
 * This function is called in the main thread
 */
//--------------------------------------------------------------------------------------------------
static void TreatResponse(
    void* param1Ptr,
    void* param2Ptr
)
{
    Command_t* cmdPtr = param1Ptr;
    LE_ASSERT(cmdPtr);

    cmdPtr->processing = false;
    if (cmdPtr->handlerFunc)
    {
        // Notify the result of the command.
        (cmdPtr->handlerFunc)(
            cmdPtr->cmdRef, 
            cmdPtr->result, 
            cmdPtr->handlerContextPtr);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Execute a command
 */
//--------------------------------------------------------------------------------------------------
static void ProcessCommand(
    void* param1Ptr,
    void* param2Ptr
)
{
    Command_t* cmdPtr = param1Ptr;
    if (NULL == cmdPtr)
    {
        LE_ERROR("Bad parameter");
        return;
    }

    cmdPtr->result = cmdPtr->ProcessFunc(cmdPtr);
    LE_ERROR_IF(cmdPtr->result, 
        "Error in processing command <%d>: %s", cmdPtr->id,
        LE_RESULT_TXT(cmdPtr->result));

    // Treat the send of responses in the main thread
    le_event_QueueFunctionToThread(workerPtr->mainThreadRef,
                                   TreatResponse,
                                   cmdPtr,
                                   NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Add command in the worker queue. 
 */
//--------------------------------------------------------------------------------------------------
void worker_QueueCommand(
    Command_t* cmdPtr
)
{
    cmdPtr->processing = true;

    // Treat the commands in the Worker thread in order to not lock the main thread
    le_event_QueueFunctionToThread(workerPtr->threadRef,
                                   ProcessCommand,
                                   cmdPtr,
                                   NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Thread used for the bridge
 * This thread is used to send the AT command to the modem (called function is synchronous, and
 * may take a lot of time).
 */
//--------------------------------------------------------------------------------------------------
static void *WorkerThreadLoop(
    void* context
)
{
    le_event_RunLoop();
    return NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * Worker initialization
 */
//--------------------------------------------------------------------------------------------------
void worker_Init(
    void
)
{
    workerPtr = (Worker_t*)malloc(sizeof(Worker_t));
    LE_ASSERT(workerPtr);
    memset(workerPtr, 0, sizeof(Worker_t));

    workerPtr->threadRef = le_thread_Create("worker", WorkerThreadLoop, NULL);
    LE_ASSERT(workerPtr->threadRef);
    le_thread_SetJoinable(workerPtr->threadRef);

    le_thread_Start(workerPtr->threadRef);
    workerPtr->mainThreadRef = le_thread_GetCurrent();
}
