#include <limits.h>
#include <stdlib.h>
#include <time.h>

#define KDB_IMPLEMENTATION
#include "kdb.h"

#define DB_NAME         "test"
#define DB_RECORD_COUNT 1000
#define DB_SMA_FRAME    15

int main(void)
{
  printf("sizeof(KDB):\t\t\t%lu\n", sizeof(KDB));
  printf("sizeof(KDB_FLAGS_TYPE):\t%lu\n", sizeof(KDB_FLAGS_TYPE));
  printf("sizeof(KDB_VALUE_TYPE):\t%lu\n", sizeof(KDB_VALUE_TYPE));
  printf("sizeof(KDB_HEADER):\t\t%lu\n", sizeof(KDB_HEADER));
  printf("sizeof(KDB_DATA):\t\t%lu\n", sizeof(KDB_DATA));

  printf("HASHMAP DUMP 1\n");
  kdb_hashmap_dbs_references_dump();
  
  KDB_INITIALIZE(db, DB_NAME);

  if (!db)
  {
    return 1;
  }

  printf("HASHMAP DUMP 2\n");
  kdb_hashmap_dbs_references_dump();

  srand(time(NULL));

  for (size_t i = 0; i < DB_RECORD_COUNT; ++i)
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

  KDB_INITIALIZE(db2, DB_NAME);

  printf("BEFORE\n");
  printf("db points to: %p\n", db);
  printf("db2 points to: %p\n", db2);
  printf("db.initialized  = %d\n", db->initialized);
  printf("db2.initialized = %d\n", db2->initialized);

  printf("HASHMAP DUMP 4\n");
  kdb_hashmap_dbs_references_dump();

  KDB_FINALIZE(db);

  if (db)
  {
    return 1;
  }

  printf("AFTER\n");
  printf("db points to: %p\n", db);
  printf("db2 points to: %p\n", db2);
  printf("db.initialized  = %d\n", db  ? db->initialized  : 0);
  printf("db2.initialized = %d\n", db2 ? db2->initialized : 0);

  printf("HASHMAP DUMP 5\n");
  kdb_hashmap_dbs_references_dump();

  printf("SMA\n");

  KDB_VALUE_TYPE sma;

  for (size_t i = 0; i < DB_RECORD_COUNT; ++i)
  {
    sma = kdb_sma(db2, i, DB_SMA_FRAME);

    printf("Index: %d - SMA: %f\n", i, sma);
  }

  KDB_FINALIZE(db2);

  if (db2)
  {
    return 1;
  }

  printf("VERY AFTER\n");
  printf("db points to: %p\n", db);
  printf("db.initialized  = %d\n", db  ? db->initialized  : 0);
  printf("db2.initialized = %d\n", db2 ? db2->initialized : 0);

  printf("HASHMAP DUMP 6\n");
  kdb_hashmap_dbs_references_dump();

  return 0;
}
