#include <limits.h>
#include <stdlib.h>
#include <time.h>

#define KDB_IMPLEMENTATION
#include "kdb.h"

int main(void)
{
  printf("sizeof(KDB):\t\t\t%lu\n", sizeof(KDB));
  printf("sizeof(KDB_FLAGS_TYPE):\t%lu\n", sizeof(KDB_FLAGS_TYPE));
  printf("sizeof(KDB_VALUE_TYPE):\t%lu\n", sizeof(KDB_VALUE_TYPE));
  printf("sizeof(KDB_HEADER):\t\t%lu\n", sizeof(KDB_HEADER));
  printf("sizeof(KDB_DATA):\t\t%lu\n", sizeof(KDB_DATA));

  KDB* db = kdb_initialize("test");

  if (!db)
  {
    return 1;
  }

  kdb_hashmap_dbs_set("test", NULL);

  srand(time(NULL));

  for (size_t i = 0; i < 1000; ++i)
  {
    kdb_add(db, (KDB_VALUE_TYPE)rand() / INT32_MAX * 2 - 1);
    // kdb_add(&db, (KDB_VALUE_TYPE)(rand() % 10));
    // kdb_add(&db, i);
  }

  printf("\n");

  kdb_dump(db, false);

  printf("\n");

  printf("Variance:\t%f\n", kdb_variance(db));
  printf("Stddev:\t\t%f\n", kdb_stddev(db));
  printf("Median:\t\t%f\n", kdb_median(db));

  printf("\n");

  kdb_dump(db, false);

  KDB* db2 = kdb_initialize("test");

  printf("BEFORE\n");
  printf("db.initialized  = %d\n", db->initialized);
  printf("db2.initialized = %d\n", db2->initialized);

  if (!kdb_finalize(db))
  {
    return 1;
  }

  printf("AFTER\n");
  printf("db.initialized  = %d\n", db->initialized);
  printf("db2.initialized = %d\n", db2->initialized);

  // kdb_hashmap_dbs_get("Test", NULL);
  // kdb_hashmap_dbs_set("Test", NULL);

  // kdb_hashmap_cache_get(10, -1.0f);
  // kdb_hashmap_cache_get(15, -2.0f);
  // kdb_hashmap_cache_get(20, -3.0f);
  // kdb_hashmap_cache_set(0, 1.0f);

  return 0;
}
