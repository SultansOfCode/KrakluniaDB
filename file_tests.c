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

  KDB db = { 0 };

  if (!kdb_initialize(&db, "test.kdb", "TEST"))
  {
    return 1;
  }

  srand(time(NULL));

  for (size_t i = 0; i < 1000; ++i)
  {
    kdb_add(&db, (KDB_VALUE_TYPE)rand() / INT32_MAX * 2 - 1);
    // kdb_add(&db, (KDB_VALUE_TYPE)(rand() % 10));
    // kdb_add(&db, i);
  }

  printf("\n");

  kdb_dump(&db, false);

  printf("\n");

  printf("Variance:\t%f\n", kdb_variance(&db));
  printf("Stddev:\t\t%f\n", kdb_stddev(&db));
  printf("Median:\t\t%f\n", kdb_median(&db));

  printf("\n");

  kdb_dump(&db, false);

  if (!kdb_finalize(&db))
  {
    return 1;
  }

  // kdb_hashmap_dbs_get("Test", NULL);
  // kdb_hashmap_dbs_set("Test", NULL);

  kdb_hashmap_cache_get(10, -1.0f);
  kdb_hashmap_cache_get(15, -2.0f);
  kdb_hashmap_cache_get(20, -3.0f);
  // kdb_hashmap_cache_set(0, 1.0f);

  return 0;
}
