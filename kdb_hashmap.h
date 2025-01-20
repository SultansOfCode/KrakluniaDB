#include <stdbool.h>
#include <string.h>

#ifndef KDB_HASHMAP_NAME
  #error "KDB_HASHMAP_NAME is not defined"
#endif

#ifndef KDB_HASHMAP_CAPACITY
  #error "KDB_HASHMAP_CAPACITY is not defined"
#endif

#ifndef KDB_HASHMAP_KEY_TYPE
  #error "KDB_HASHMAP_KEY_TYPE is not defined"
#endif

#ifndef KDB_HASHMAP_VALUE_TYPE
  #error "KDB_HASHMAP_VALUE_TYPE is not defined"
#endif

#define KDB_HASHMAP_GLUE_HELPER(a, b) a##b
#define KDB_HASHMAP_GLUE(a, b)        KDB_HASHMAP_GLUE_HELPER(a, b)
#define KDB_HASHMAP_ENTRY             KDB_HASHMAP_GLUE(KDB_HASHMAP_GLUE(KDB_HASHMAP_, KDB_HASHMAP_NAME), _ENTRY)
#define KDB_HASHMAP                   KDB_HASHMAP_GLUE(KDB_HASHMAP_, KDB_HASHMAP_NAME)
#define KDB_HASHMAP_FUNCTION_BASE     KDB_HASHMAP_GLUE(KDB_HASHMAP_GLUE(kdb_hashmap_, KDB_HASHMAP_NAME), _)
#define KDB_HASHMAP_FUNCTION_GET      KDB_HASHMAP_GLUE(KDB_HASHMAP_FUNCTION_BASE, get)
#define KDB_HASHMAP_FUNCTION_SET      KDB_HASHMAP_GLUE(KDB_HASHMAP_FUNCTION_BASE, set)
#define KDB_HASHMAP_BUFFER            KDB_HASHMAP_GLUE(KDB_HASHMAP_FUNCTION_BASE, buffer)

typedef struct
{
  KDB_HASHMAP_KEY_TYPE   key;
  KDB_HASHMAP_VALUE_TYPE value;
  bool                   occupied;
} KDB_HASHMAP_ENTRY;

KDB_HASHMAP_ENTRY KDB_HASHMAP_BUFFER[KDB_HASHMAP_CAPACITY] = { 0 };

#ifndef KDB_HASHMAP_HASH_FUNCTIONS
  #define KDB_HASHMAP_HASH_FUNCTIONS 1

  uint64_t kdb_hashmap_hash(void* key)
  {
    uint64_t hash = 0;

    while (*((char*)key) != 0)
    {
      hash += *((char*)key);

      ++key;
    }

    hash %= KDB_HASHMAP_CAPACITY;

    return hash;
  }
#endif

KDB_HASHMAP_VALUE_TYPE KDB_HASHMAP_FUNCTION_GET(KDB_HASHMAP_KEY_TYPE key, KDB_HASHMAP_VALUE_TYPE default_value)
{
  uint64_t hash = 0;

  if (__builtin_types_compatible_p(typeof(KDB_HASHMAP_KEY_TYPE), char*) == 1)
  {
    hash = kdb_hashmap_hash((void*)(uintptr_t)key);
  }
  else
  {
    hash = (uint64_t)key % KDB_HASHMAP_CAPACITY;
  }

  for (uint64_t tries = 0; tries < KDB_HASHMAP_CAPACITY; ++tries)
  {
    if (!KDB_HASHMAP_BUFFER[hash].occupied)
    {
      break;
    }

    if (KDB_HASHMAP_BUFFER[hash].key != key)
    {
      hash = (hash + 1) % KDB_HASHMAP_CAPACITY;

      continue;
    }

    return KDB_HASHMAP_BUFFER[hash].value;
  }

  return default_value;
}

bool KDB_HASHMAP_FUNCTION_SET(KDB_HASHMAP_KEY_TYPE key, KDB_HASHMAP_VALUE_TYPE value)
{
  uint64_t hash  = 0;

  if (__builtin_types_compatible_p(typeof(key), char*) == 1)
  {
    hash = kdb_hashmap_hash((void*)(uintptr_t)key);
  }
  else
  {
    hash = (uint64_t)key % KDB_HASHMAP_CAPACITY;
  }

  uint64_t tries = 0;

  for (tries = 0; tries < KDB_HASHMAP_CAPACITY; ++tries)
  {
    if (!KDB_HASHMAP_BUFFER[hash].occupied)
    {
      break;
    }

    if (KDB_HASHMAP_BUFFER[hash].key == key)
    {
      break;
    }

    hash = (hash + 1) % KDB_HASHMAP_CAPACITY;
  }

  if (tries == KDB_HASHMAP_CAPACITY)
  {
    return false;
  }

  KDB_HASHMAP_BUFFER[hash].key      = key;
  KDB_HASHMAP_BUFFER[hash].value    = value;
  KDB_HASHMAP_BUFFER[hash].occupied = true;

  return true;
}

// #undef KDB_HASHMAP_BUFFER
// #undef KDB_HASHMAP_FUNCTION_SET
// #undef KDB_HASHMAP_FUNCTION_GET
// #undef KDB_HASHMAP_FUNCTION_BASE
// #undef KDB_HASHMAP
// #undef KDB_HASHMAP_ENTRY
// #undef KDB_HASHMAP_GLUE
// #undef KDB_HASHMAP_GLUE_HELPER

#undef KDB_HASHMAP_VALUE_TYPE
#undef KDB_HASHMAP_KEY_TYPE
#undef KDB_HASHMAP_CAPACITY
#undef KDB_HASHMAP_NAME
