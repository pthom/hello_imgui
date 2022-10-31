#include "hello_imgui/backend_api/backend_api.h"


namespace BackendApi
{
    bool EventCallback_NoOp(void * /*backendEvent*/)
    {
        return false;
    }

}