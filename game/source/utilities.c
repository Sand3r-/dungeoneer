
internal MessageTextDescriptorParseResult
MessageTextDescriptorParse(char *descriptor)
{
    MessageTextDescriptorParseResult result = {0};
    
    Assert(descriptor);
    
    u32 descriptor_length = 0;
    for(; descriptor[descriptor_length] != '}' && descriptor[descriptor_length];
        ++descriptor_length);
    
    char text_descriptor_type[8] = {0};
    CopySubstringToStringUntilCharN(text_descriptor_type, sizeof(text_descriptor_type),
                                    descriptor, ':');
    
    // NOTE(rjf): Parsing a color
    if(CStringMatchCaseInsensitiveN(text_descriptor_type, "c", 1) ||
       CStringMatchCaseInsensitiveN(text_descriptor_type, "color", 5))
    {
        char *num_read_str = descriptor + CStringIndexAfterSubstring(descriptor, ":");
        
        if(CStringMatchCaseInsensitiveN(num_read_str, "reset", 5))
        {
            result.type = MESSAGE_TEXT_DESCRIPTOR_PARSE_RESULT_reset_color;
        }
        else
        {
            result.type = MESSAGE_TEXT_DESCRIPTOR_PARSE_RESULT_color;
            
            result.color.color.r = GetFirstF32FromCString(num_read_str);
            num_read_str += CStringIndexAfterSubstring(num_read_str, ",");
            
            result.color.color.g = GetFirstF32FromCString(num_read_str);
            num_read_str += CStringIndexAfterSubstring(num_read_str, ",");
            
            result.color.color.b = GetFirstF32FromCString(num_read_str);
            num_read_str += CStringIndexAfterSubstring(num_read_str, ",");
        }
    }
    // NOTE(rjf): Parsing speed
    else if(CStringMatchCaseInsensitiveN(text_descriptor_type, "s", 1) ||
            CStringMatchCaseInsensitiveN(text_descriptor_type, "speed", 5))
    {
        
        char *num_read_str = descriptor + CStringIndexAfterSubstring(descriptor, ":");
        
        if(CStringMatchCaseInsensitiveN(num_read_str, "reset", 5))
        {
            result.type = MESSAGE_TEXT_DESCRIPTOR_PARSE_RESULT_reset_speed;
        }
        else
        {
            result.type = MESSAGE_TEXT_DESCRIPTOR_PARSE_RESULT_speed;
            result.speed.speed = GetFirstF32FromCString(num_read_str);
        }
    }
    // NOTE(rjf): Invalid
    else
    {
#if !BUILD_RELEASE
        platform->OutputError("Invalid Message Text Descriptor",
                              "\"%.*s\" is not a valid message text descriptor.",
                              descriptor_length,
                              descriptor);
#endif
    }
    
    return result;
}