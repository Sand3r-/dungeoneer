internal EntityID
EntityIDInit(u16 type, u16 id)
{
    EntityID result = { type, id };
    return result;
}

internal EntityID
PlayerEntityID(void)
{
    EntityID id = { 0, 1, };
    return id;
}

internal b32
EntityIDEqual(EntityID id1, EntityID id2)
{
    return (id1.type == id2.type &&
            id1.instance_id == id2.instance_id);
}