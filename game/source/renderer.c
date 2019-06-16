internal void
_RendererInternalFinishActiveRequest(Renderer *renderer)
{
    if(renderer->active_request.type > 0)
    {
        Assert(renderer->request_count < RENDERER_MAX_REQUESTS);
        renderer->requests[renderer->request_count++] = renderer->active_request;
        renderer->active_request.type = 0;
    }
}

#if RENDERER_BACKEND == RENDERER_OPENGL
#include "renderer_opengl.c"
#endif