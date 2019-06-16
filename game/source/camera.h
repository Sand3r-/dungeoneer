typedef struct Camera Camera;
struct Camera
{
    v3 look_at_position;
    v3 look_at_position_target;
    v3 eye;
};

internal void
CameraInit(Camera *camera, v3 target)
{
    camera->look_at_position = camera->look_at_position_target = target;
}

internal void
CameraUpdate(Camera *camera)
{
    camera->look_at_position.x += (camera->look_at_position_target.x - camera->look_at_position.x) * app->delta_t * 4.f;
    camera->look_at_position.y += (camera->look_at_position_target.y - camera->look_at_position.y) * app->delta_t * 4.f;
    camera->look_at_position.z += (camera->look_at_position_target.z - camera->look_at_position.z) * app->delta_t * 4.f;
    
    camera->eye.x = camera->look_at_position.x;
    camera->eye.y = camera->look_at_position.y + 20;
    camera->eye.z = camera->look_at_position.z + 15;
}