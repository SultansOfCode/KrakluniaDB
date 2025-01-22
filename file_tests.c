#include <limits.h>
#include <stdlib.h>
#include <time.h>

#define KDB_IMPLEMENTATION
#include "kdb.h"

#define DB_NAME "test"

int main(void)
{
  printf("sizeof(KDB):\t\t\t%lu\n", sizeof(KDB));
  printf("sizeof(KDB_FLAGS_TYPE):\t%lu\n", sizeof(KDB_FLAGS_TYPE));
  printf("sizeof(KDB_VALUE_TYPE):\t%lu\n", sizeof(KDB_VALUE_TYPE));
  printf("sizeof(KDB_HEADER):\t\t%lu\n", sizeof(KDB_HEADER));
  printf("sizeof(KDB_DATA):\t\t%lu\n", sizeof(KDB_DATA));

  printf("HASHMAP DUMP 1\n");
  kdb_hashmap_dbs_references_dump();
  
  KDB* db = kdb_initialize(DB_NAME);

  if (!db)
  {
    return 1;
  }

  printf("HASHMAP DUMP 2\n");
  kdb_hashmap_dbs_references_dump();

  srand(time(NULL));

  for (size_t i = 0; i < 1000; ++i)
  {
    kdb_add(db, (KDB_VALUE_TYPE)rand() / INT32_MAX * 2 - 1);
  }

  printf("\n");

  kdb_dump(db, false);

  printf("\n");

  printf("Variance:\t%f\n", kdb_variance(db));
  printf("Stddev:\t\t%f\n", kdb_stddev(db));
  printf("Median:\t\t%f\n", kdb_median(db));

  printf("\n");

  kdb_dump(db, false);

  printf("HASHMAP DUMP 3\n");
  kdb_hashmap_dbs_references_dump();

  KDB* db2 = kdb_initialize(DB_NAME);

  printf("BEFORE\n");
  printf("db points to: %p\n", db);
  printf("db.initialized  = %d\n", db->initialized);
  printf("db2.initialized = %d\n", db2->initialized);

  printf("HASHMAP DUMP 4\n");
  kdb_hashmap_dbs_references_dump();

  if (!kdb_finalize(db))
  {
    return 1;
  }

  printf("AFTER\n");
  printf("db points to: %p\n", db);
  printf("db.initialized  = %d\n", db->initialized);
  printf("db2.initialized = %d\n", db2->initialized);

  printf("HASHMAP DUMP 5\n");
  kdb_hashmap_dbs_references_dump();

  if (!kdb_finalize(db2))
  {
    return 1;
  }

  printf("VERY AFTER\n");
  printf("db points to: %p\n", db);
  printf("db.initialized  = %d\n", db->initialized);
  printf("db2.initialized = %d\n", db2->initialized);

  printf("HASHMAP DUMP 6\n");
  kdb_hashmap_dbs_references_dump();

  return 0;
}
