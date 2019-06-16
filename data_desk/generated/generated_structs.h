typedef struct EntityID EntityID;
struct EntityID
{
u16 type;
u16 instance_id;
};

typedef struct EntityIndex EntityIndex;
struct EntityIndex
{
u16 type;
u16 index;
};

enum
{
ATTACK_TYPE_melee,
ATTACK_TYPE_projectile_fire,
};

enum
{
ATTACK_STAGE_idle,
ATTACK_STAGE_windup,
ATTACK_STAGE_hot,
ATTACK_STAGE_cooldown,
};

enum
{
ENEMY_AI_TYPE_melee,
ENEMY_AI_TYPE_ranged,
};

enum
{
ENEMY_AI_STATE_calm,
ENEMY_AI_STATE_attacking,
};

typedef struct OpenGLShaderInput OpenGLShaderInput;
struct OpenGLShaderInput
{
int index;
char *name;
};

typedef struct OpenGLShaderOutput OpenGLShaderOutput;
struct OpenGLShaderOutput
{
int index;
char *name;
};

