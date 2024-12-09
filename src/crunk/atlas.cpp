
internal Texture_Region *find_texture_region(Texture_Atlas *atlas, String8 name) {
    Texture_Region *result = NULL;
    u64 hash = djb2_hash_string(name);
    Texture_Region_Bucket *region_bucket = atlas->region_hash_table + hash % atlas->region_hash_table_size;
    for (Texture_Region *region = region_bucket->first; region; region = region->hash_next) {
        if (str8_match(region->name, name, StringMatchFlag_Nil)) {
            result = region;
        }
    }
    return result;
}
