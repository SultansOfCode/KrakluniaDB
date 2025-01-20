#ifndef KDB_H_
#define KDB_H_

#include <errno.h>
#include <math.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#if defined(KDB_USE_LONG_DOUBLE) && defined(KDB_USE_DOUBLE)
  #error "You can't define KDB_USE_LONG_DOUBLE and KDB_USE_DOUBLE at the same time"
#endif

#define KDB_VERSION_SIZE      4
#define KDB_NAME_SIZE         10
#define KDB_VERSION           "KDB\1"
#define KDB_FLAGS_TYPE        uint16_t
#define KDB_FLAGS_TYPE_FORMAT "%04hX"

#ifdef KDB_USE_LONG_DOUBLE
  #define KDB_VALUE_TYPE        long double
  #define KDB_VALUE_TYPE_FORMAT "%Lf"
#else
  #define KDB_VALUE_TYPE_FORMAT "%f"

  #ifdef KDB_USE_DOUBLE
    #define KDB_VALUE_TYPE double
  #else
    #define KDB_VALUE_TYPE float
  #endif
#endif

#define KDB_ERROR(message, ...) \
  do \
  { \
    fprintf(stderr, "%s:%d [%s] "message, __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
  } while (0)

#define KDB_CHECK_INITIALIZED_VOID(db) \
  do \
  { \
    if (!(db) || !(db)->initialized) \
    { \
      KDB_ERROR("Database not initialized\n"); \
      \
      return; \
    } \
  } while (0)

#define KDB_CHECK_INITIALIZED(db, ret) \
  do \
  { \
    if (!(db) || !(db)->initialized) \
    { \
      KDB_ERROR("Database not initialized\n"); \
      \
      return (ret); \
    } \
  } while (0)

#define KDB_PUSH_HEADER \
  KDB_HEADER old_header = { 0 }; \
  \
  do \
  { \
    memcpy(&old_header, &db->header, sizeof(KDB_HEADER)); \
  } while (0)

#define KDB_POP_HEADER \
  do \
  { \
    memcpy(&db->header, &old_header, sizeof(KDB_HEADER)); \
  } while (0)

typedef enum
{
  KDB_FLAGS_USE_DOUBLE          = 0b0001,
  KDB_FLAGS_USE_LONG_DOUBLE     = 0b0010,
  KDB_FLAGS_VARIANCE_CALCULATED = 0b0100,
  KDB_FLAGS_MEDIAN_CALCULATED   = 0b1000
} KDB_FLAGS;

typedef struct
{
  char           version[KDB_VERSION_SIZE];
  char           name[KDB_NAME_SIZE];
  KDB_FLAGS_TYPE flags;
  uint32_t       count;
  KDB_VALUE_TYPE sum;
  KDB_VALUE_TYPE average;
  KDB_VALUE_TYPE min;
  KDB_VALUE_TYPE max;
  KDB_VALUE_TYPE variance;
  KDB_VALUE_TYPE median;
} KDB_HEADER;

typedef struct
{
  uint64_t       timestamp;
  KDB_VALUE_TYPE value;
  KDB_VALUE_TYPE sum;
} KDB_DATA;

typedef struct
{
  bool       initialized;
  char*      filename;
  FILE*      file;
  KDB_HEADER header;
} KDB;

#define KDB_HASHMAP_NAME       dbs
#define KDB_HASHMAP_CAPACITY   32
#define KDB_HASHMAP_KEY_TYPE   char*
#define KDB_HASHMAP_VALUE_TYPE KDB*
#include "kdb_hashmap.h"

#define KDB_HASHMAP_NAME       cache
#define KDB_HASHMAP_CAPACITY   64
#define KDB_HASHMAP_KEY_TYPE   unsigned int
#define KDB_HASHMAP_VALUE_TYPE KDB_VALUE_TYPE
#include "kdb_hashmap.h"

int            kdb_compare_values(const void* a, const void* b);
KDB_VALUE_TYPE kdb_map_value(KDB_VALUE_TYPE value, KDB_VALUE_TYPE min_a, KDB_VALUE_TYPE max_a, KDB_VALUE_TYPE min_b, KDB_VALUE_TYPE max_b);
void           kdb_dump_flags_binary(KDB* db);
void           kdb_dump_flags_name(KDB* db);
void           kdb_dump_header(KDB* db);
void           kdb_dump_data(KDB_DATA* data);
void           kdb_dump(KDB* db, bool include_all_data);
bool           kdb_write_header(KDB* db);
bool           kdb_write_data(KDB* db, KDB_DATA* data);
bool           kdb_initialize(KDB* db, char* filename, char* name);
bool           kdb_finalize(KDB* db);
bool           kdb_get_data(KDB* db, int64_t index, KDB_DATA* data);
bool           kdb_get_data_normalized(KDB* db, int64_t index, KDB_DATA* data);
bool           kdb_get_data_normalized_neg(KDB* db, int64_t index, KDB_DATA* data);
bool           kdb_add_ts(KDB* db, uint64_t timestamp, KDB_VALUE_TYPE value);
bool           kdb_add(KDB* db, KDB_VALUE_TYPE value);
uint32_t       kdb_count(KDB* db);
KDB_VALUE_TYPE kdb_sum(KDB* db);
KDB_VALUE_TYPE kdb_average(KDB* db);
KDB_VALUE_TYPE kdb_min(KDB* db);
KDB_VALUE_TYPE kdb_max(KDB* db);
KDB_VALUE_TYPE kdb_variance(KDB* db);
KDB_VALUE_TYPE kdb_stddev(KDB* db);
KDB_VALUE_TYPE kdb_median(KDB* db);
KDB_VALUE_TYPE kdb_sma(KDB* db, uint32_t index, uint32_t frame);
#endif // KDB_H_

#ifdef KDB_IMPLEMENTATION
int kdb_compare_values(const void* a, const void* b)
{
  KDB_VALUE_TYPE first = *(const KDB_VALUE_TYPE*)a;
  KDB_VALUE_TYPE second = *(const KDB_VALUE_TYPE*)b;

  if (first < second)
  {
    return -1;
  }

  if (second < first)
  {
    return 1;
  }

  return 0;
}

KDB_VALUE_TYPE kdb_map_value(KDB_VALUE_TYPE value, KDB_VALUE_TYPE min_a, KDB_VALUE_TYPE max_a, KDB_VALUE_TYPE min_b, KDB_VALUE_TYPE max_b)
{
  KDB_VALUE_TYPE range_a = max_a - min_a;
  KDB_VALUE_TYPE range_b = max_b - min_b;

  return (value - min_a) * range_b / range_a + min_b;
}

void kdb_dump_flags_binary(KDB* db)
{
  KDB_FLAGS_TYPE flags = 0;

  if (db && db->initialized && db->header.flags != 0)
  {
    flags = db->header.flags;
  }

  KDB_FLAGS_TYPE flag = (KDB_FLAGS_TYPE)1 << (sizeof(KDB_FLAGS_TYPE) * 8 - 1);

  while (flag > 0)
  {
    printf("%d", (flags & flag) != 0);

    flag >>= 1;
  }
}

void kdb_dump_flags_name(KDB* db)
{
  if (!db || !db->initialized || db->header.flags == 0)
  {
    printf("None");

    return;
  }

  bool printed_flag = false;

  if ((db->header.flags & KDB_FLAGS_MEDIAN_CALCULATED) != 0)
  {
    printf("%s%s", printed_flag ? " | " : "", "MEDIAN_CALCULATED");

    if (!printed_flag)
    {
      printed_flag = true;
    }
  }

  if ((db->header.flags & KDB_FLAGS_VARIANCE_CALCULATED) != 0)
  {
    printf("%s%s", printed_flag ? " | " : "", "VARIANCE_CALCULATED");

    if (!printed_flag)
    {
      printed_flag = true;
    }
  }

  if ((db->header.flags & KDB_FLAGS_USE_DOUBLE) != 0)
  {
    printf("%s%s", printed_flag ? " | " : "", "USE_DOUBLE");

    if (!printed_flag)
    {
      printed_flag = true;
    }
  }

  if ((db->header.flags & KDB_FLAGS_USE_LONG_DOUBLE) != 0)
  {
    printf("%s%s", printed_flag ? " | " : "", "USE_LONG_DOUBLE");

    if (!printed_flag)
    {
      printed_flag = true;
    }
  }
}
void kdb_dump_header(KDB* db)
{
  KDB_CHECK_INITIALIZED_VOID(db);

  printf("File:\t\t%s\n", db->filename);

  printf("File version:\t");

  for (size_t i = 0; i < KDB_VERSION_SIZE; ++i)
  {
    printf(i < (KDB_VERSION_SIZE - 1) ? "%c" : "%d", db->header.version[i]);
  }

  printf("\n");

  printf("Indicator name:\t%s\n", db->header.name);
  printf("Flags:\t\t"KDB_FLAGS_TYPE_FORMAT" (", db->header.flags);

  kdb_dump_flags_binary(db);

  printf(") (");

  kdb_dump_flags_name(db);

  printf(")\n");
  printf("Records' count:\t%u\n",                db->header.count);
  printf("Sum:\t\t"KDB_VALUE_TYPE_FORMAT"\n",    db->header.sum);
  printf("Average:\t"KDB_VALUE_TYPE_FORMAT"\n",  db->header.average);
  printf("Minimum:\t"KDB_VALUE_TYPE_FORMAT"\n",  db->header.min);
  printf("Maximum:\t"KDB_VALUE_TYPE_FORMAT"\n",  db->header.max);
  printf("Variance:\t"KDB_VALUE_TYPE_FORMAT"\n", db->header.variance);
  printf("StdDev:\t\t"KDB_VALUE_TYPE_FORMAT"\n", (db->header.flags & KDB_FLAGS_VARIANCE_CALCULATED) != 0 ? kdb_stddev(db) : INFINITY);
  printf("Median:\t\t"KDB_VALUE_TYPE_FORMAT"\n", (db->header.flags & KDB_FLAGS_MEDIAN_CALCULATED) != 0 ? kdb_median(db) : INFINITY);
}

void kdb_dump_data(KDB_DATA* data)
{
  if (!data)
  {
    return;
  }

  printf("Timestamp:\t%llu\n", data->timestamp);
  printf("Value:\t\t"KDB_VALUE_TYPE_FORMAT"\n", data->value);
  printf("Sum:\t\t"KDB_VALUE_TYPE_FORMAT"\n", data->sum);
}

void kdb_dump(KDB* db, bool include_all_data)
{
  KDB_CHECK_INITIALIZED_VOID(db);

  kdb_dump_header(db);

  if (!include_all_data)
  {
    return;
  }

  KDB_DATA data;

  for (uint32_t i = 0; i < db->header.count; ++i)
  {
    if (!kdb_get_data(db, i, &data))
    {
      return;
    }

    printf("\n");

    kdb_dump_data(&data);
  }
}

bool kdb_write_header(KDB* db)
{
  if (!db)
  {
    KDB_ERROR("Database pointer is NULL\n");

    return false;
  }

  if (!db->file)
  {
    KDB_ERROR("File handler is not set\n");

    return false;
  }

  if (fseek(db->file, 0, SEEK_SET) != 0)
  {
    KDB_ERROR("Error seeking for the start of the file\n");

    return false;
  }

  if (fwrite(&db->header, sizeof(KDB_HEADER), 1, db->file) != 1)
  {
    KDB_ERROR("Error while trying to write the file header\n");

    return false;
  }

  if (fflush(db->file) != 0)
  {
    KDB_ERROR("Error writing file to disk\n");

    return false;
  }

  return true;
}

bool kdb_write_data(KDB* db, KDB_DATA* data)
{
  KDB_CHECK_INITIALIZED(db, false);

  if (!db->file)
  {
    KDB_ERROR("File handler is not set\n");

    return false;
  }

  if (fseek(db->file, 0, SEEK_END) != 0)
  {
    KDB_ERROR("Error seeking for the end of the file\n");

    return false;
  }

  if (fwrite(data, sizeof(KDB_DATA), 1, db->file) != 1)
  {
    KDB_ERROR("Error while trying to write the data to file\n");

    return false;
  }

  if (fflush(db->file) != 0)
  {
    KDB_ERROR("Error writing file to disk\n");

    return false;
  }

  return true;
}

// Initialize the database's structure
bool kdb_initialize(KDB* db, char* filename, char* name)
{
  if (!db)
  {
    return false;
  }

  if (db->initialized)
  {
    return true;
  }

  // Ensure everything is clean
  kdb_finalize(db);

  // Validate name limit
  size_t name_size = strlen(name);

  if (name_size > KDB_NAME_SIZE)
  {
    KDB_ERROR("Database name is above the %d characters limit\n", KDB_NAME_SIZE);

    return false;
  }

  // Initialize filename
  db->filename = filename;

  // Try to open the file to read/update
  db->file = fopen(filename, "r+b");

  if (!db->file)
  {
    // If file does not exist, try to create it
    if (errno == ENOENT)
    {
      // Try to open the file to write/read
      db->file = fopen(filename, "w+b");

      if (!db->file)
      {
        KDB_ERROR("Failed to create the file \"%s\"\n", filename);

        goto file_error;
      }

      // Initialize data
      memcpy(&db->header.version, &KDB_VERSION, KDB_VERSION_SIZE);
      memcpy(&db->header.name, name, name_size);

      #ifdef KDB_USE_LONG_DOUBLE
        db->header.flags |= KDB_FLAGS_USE_LONG_DOUBLE;
      #else
        #ifdef KDB_USE_DOUBLE
          db->header.flags |= KDB_FLAGS_USE_DOUBLE;
        #endif
      #endif

      // Try to write the header
      if (!kdb_write_header(db))
      {
        goto file_error;
      }

      // All good
      db->initialized = true;

      return true;
    }

    KDB_ERROR("Failed to open \"%s\"\n", filename);

    file_error:
      kdb_finalize(db);

      return false;
  }

  // Read data from file
  char             f_version[KDB_VERSION_SIZE] = { 0 };
  char             f_name[KDB_NAME_SIZE]       = { 0 };
  size_t           f_name_size                 = 0;
  KDB_FLAGS_TYPE f_flags                       = 0;
  uint32_t         f_count                     = 0;
  KDB_VALUE_TYPE f_sum                         = 0.0f;
  KDB_VALUE_TYPE f_average                     = 0.0f;
  KDB_VALUE_TYPE f_min                         = INFINITY;
  KDB_VALUE_TYPE f_max                         = -INFINITY;
  KDB_VALUE_TYPE f_variance                    = INFINITY;
  KDB_VALUE_TYPE f_median                      = INFINITY;

  // Try to parse the version
  if (fread(&f_version, sizeof(char), KDB_VERSION_SIZE, db->file) != KDB_VERSION_SIZE)
  {
    KDB_ERROR("Failed to read the database version\n");

    goto parsing_error;
  }

  // Check the magic string
  if (strncmp(f_version, "KDB", 3) != 0)
  {
    KDB_ERROR("File has no magic string in header\n");

    goto parsing_error;
  }

  // Check if the version is right/supported
  if (f_version[KDB_VERSION_SIZE - 1] != 1)
  {
    KDB_ERROR("Unknown database version\n");

    goto parsing_error;
  }

  // Try to parse the name
  if (fread(&f_name, sizeof(char), KDB_NAME_SIZE, db->file) != KDB_NAME_SIZE)
  {
    KDB_ERROR("Failed to read the database name\n");

    goto parsing_error;
  }

  // Validate the name
  f_name_size = strlen(f_name);

  if (f_name_size != name_size || strcmp(f_name, name) != 0)
  {
    KDB_ERROR("Wrong name for the database\n");

    goto parsing_error;
  }

  // Try to parse the flags
  if (fread(&f_flags, sizeof(KDB_FLAGS_TYPE), 1, db->file) != 1)
  {
    KDB_ERROR("Failed to read the flags from database\n");

    goto parsing_error;
  }

  // Check compatibility between the library and the file
  #ifdef KDB_USE_LONG_DOUBLE
    if ((f_flags & KDB_FLAGS_USE_LONG_DOUBLE) == 0)
    {
      KDB_ERROR("KDB is set to use long double and this file is not compatible\n");

      goto parsing_error;
    }
  #else
    #ifdef KDB_USE_DOUBLE
      if ((f_flags & KDB_FLAGS_USE_DOUBLE) == 0)
      {
        KDB_ERROR("KDB is set to use double and this file is not compatible\n");

        goto parsing_error;
      }
    #endif
  #endif

  // Try to parse the records count
  if (fread(&f_count, sizeof(uint32_t), 1, db->file) != 1)
  {
    KDB_ERROR("Failed to read the records' count from database\n");

    goto parsing_error;
  }

  // Try to parse the sum
  if (fread(&f_sum, sizeof(KDB_VALUE_TYPE), 1, db->file) != 1)
  {
    KDB_ERROR("Failed to read the sum from database\n");

    goto parsing_error;
  }

  // Try to parse the average
  if (fread(&f_average, sizeof(KDB_VALUE_TYPE), 1, db->file) != 1)
  {
    KDB_ERROR("Failed to read the average from database\n");

    goto parsing_error;
  }

  // Try to parse the minimum
  if (fread(&f_min, sizeof(KDB_VALUE_TYPE), 1, db->file) != 1)
  {
    KDB_ERROR("Failed to read the minimum from database\n");

    goto parsing_error;
  }

  // Try to parse the maximum
  if (fread(&f_max, sizeof(KDB_VALUE_TYPE), 1, db->file) != 1)
  {
    KDB_ERROR("Failed to read the maximum from database\n");

    goto parsing_error;
  }

  // Try to parse the variance
  if (fread(&f_variance, sizeof(KDB_VALUE_TYPE), 1, db->file) != 1)
  {
    KDB_ERROR("Failed to read the variance from database\n");

    goto parsing_error;
  }

  // Try to parse the median
  if (fread(&f_median, sizeof(KDB_VALUE_TYPE), 1, db->file) != 1)
  {
    KDB_ERROR("Failed to read the median from database\n");

    goto parsing_error;
  }

  // Initialize the database with the read/parsed data
  memcpy(&db->header.version, &f_version, KDB_VERSION_SIZE);
  memcpy(&db->header.name, &f_name, f_name_size);

  db->header.flags    = f_flags;
  db->header.count    = f_count;
  db->header.sum      = f_sum;
  db->header.average  = f_average;
  db->header.min      = f_min;
  db->header.max      = f_max;
  db->header.variance = f_variance;
  db->header.median   = f_median;

  // All good
  db->initialized = true;

  return true;

  // Close the file
  parsing_error:
    kdb_finalize(db);

    return false;
}

// Finalize the database's structure
bool kdb_finalize(KDB* db)
{
  if (!db)
  {
    return true;
  }

  // Close the file
  if (db->file)
  {
    if (fclose(db->file) != 0)
    {
      KDB_ERROR("Failed to close file handler\n");

      return false;
    }

    db->file = NULL;
  }

  // Zero all data
  if (db->filename)
  {
    db->filename = NULL;
  }

  memset(&db->header.version, 0, KDB_VERSION_SIZE);
  memset(&db->header.name, 0, KDB_NAME_SIZE);

  db->header.flags    = 0;
  db->header.count    = 0;
  db->header.sum      = 0.0f;
  db->header.average  = 0.0f;
  db->header.min      = INFINITY;
  db->header.max      = -INFINITY;
  db->header.variance = INFINITY;
  db->header.median   = INFINITY;

  db->initialized     = false;

  return true;
}

bool kdb_get_data(KDB* db, int64_t index, KDB_DATA* data)
{
  data->timestamp = 0;
  data->value     = 0.0f;
  data->sum       = 0.0f;

  KDB_CHECK_INITIALIZED(db, false);

  if (index < 0 || index >= db->header.count)
  {
    return true;
  }

  if (fseek(db->file, sizeof(KDB_HEADER) + sizeof(KDB_DATA) * index, SEEK_SET) != 0)
  {
    KDB_ERROR("Error seeking for the index's data\n");

    return false;
  }

  if (fread(data, sizeof(KDB_DATA), 1, db->file) != 1)
  {
    KDB_ERROR("Error reading the index's data\n");

    return false;
  }

  return true;
}

bool kdb_get_data_normalized(KDB* db, int64_t index, KDB_DATA* data)
{
  if (!kdb_get_data(db, index, data))
  {
    return false;
  }

  data->value = kdb_map_value(data->value, db->header.min, db->header.max, 0.0f, 1.0f);

  return true;
}

bool kdb_get_data_normalized_neg(KDB* db, int64_t index, KDB_DATA* data)
{
  if (!kdb_get_data(db, index, data))
  {
    return false;
  }

  data->value = kdb_map_value(data->value, db->header.min, db->header.max, -1.0f, 1.0f);

  return true;
}

bool kdb_add_ts(KDB* db, uint64_t timestamp, KDB_VALUE_TYPE value)
{
  KDB_CHECK_INITIALIZED(db, false);

  KDB_PUSH_HEADER;

  db->header.flags &= ~KDB_FLAGS_VARIANCE_CALCULATED;
  db->header.flags &= ~KDB_FLAGS_MEDIAN_CALCULATED;

  ++db->header.count;

  db->header.sum     += value;
  db->header.average  = db->header.sum / db->header.count;

  if (value < db->header.min)
  {
    db->header.min = value;
  }

  if (value > db->header.max)
  {
    db->header.max = value;
  }

  db->header.variance = INFINITY;
  db->header.median   = INFINITY;

  if (!kdb_write_header(db))
  {
    goto save_error;
  }

  KDB_DATA data = {
    .timestamp = timestamp,
    .value     = value,
    .sum       = db->header.sum
  };

  if (!kdb_write_data(db, &data))
  {
    goto save_error;
  }

  return true;

  save_error:
    KDB_POP_HEADER;

    return false;
}

bool kdb_add(KDB* db, KDB_VALUE_TYPE value)
{
  uint64_t timestamp = time(NULL);

  return kdb_add_ts(db, timestamp, value);
}

uint32_t kdb_count(KDB* db)
{
  KDB_CHECK_INITIALIZED(db, 0);

  return db->header.count;
}

KDB_VALUE_TYPE kdb_sum(KDB* db)
{
  KDB_CHECK_INITIALIZED(db, 0.0f);

  return db->header.sum;
}

KDB_VALUE_TYPE kdb_average(KDB* db)
{
  KDB_CHECK_INITIALIZED(db, 0.0f);

  return db->header.average;
}

KDB_VALUE_TYPE kdb_min(KDB* db)
{
  KDB_CHECK_INITIALIZED(db, INFINITY);

  return db->header.min;
}

KDB_VALUE_TYPE kdb_max(KDB* db)
{
  KDB_CHECK_INITIALIZED(db, -INFINITY);

  return db->header.max;
}

KDB_VALUE_TYPE kdb_variance(KDB* db)
{
  KDB_CHECK_INITIALIZED(db, INFINITY);

  if (db->header.count == 0)
  {
    return INFINITY;
  }

  if ((db->header.flags & KDB_FLAGS_VARIANCE_CALCULATED) != 0)
  {
    return db->header.variance;
  }

  KDB_VALUE_TYPE summation = 0.0f;

  KDB_DATA       data;
  KDB_VALUE_TYPE difference;

  for (uint32_t i = 0; i < db->header.count; ++i)
  {
    if (!kdb_get_data(db, i, &data))
    {
      return INFINITY;
    }

    difference = data.value - db->header.average;

    summation += difference * difference;
  }

  KDB_VALUE_TYPE variance = summation / db->header.count;

  KDB_PUSH_HEADER;

  db->header.flags    |= KDB_FLAGS_VARIANCE_CALCULATED;
  db->header.variance  = variance;

  if (!kdb_write_header(db))
  {
    KDB_POP_HEADER;

    return INFINITY;
  }

  return variance;
}

KDB_VALUE_TYPE kdb_stddev(KDB* db)
{
  KDB_CHECK_INITIALIZED(db, INFINITY);

  if (db->header.count == 0)
  {
    return INFINITY;
  }

  KDB_VALUE_TYPE variance = kdb_variance(db);

  if (variance == INFINITY)
  {
    return INFINITY;
  }

  #ifdef KDB_USE_LONG_DOUBLE
    return sqrtl(variance);
  #else
    #ifdef KDB_USE_DOUBLE
      return sqrt(variance);
    #else
      return sqrtf(variance);
    #endif
  #endif
}

KDB_VALUE_TYPE kdb_median(KDB* db)
{
  KDB_CHECK_INITIALIZED(db, INFINITY);

  if (db->header.count == 0)
  {
    return INFINITY;
  }

  if ((db->header.flags & KDB_FLAGS_MEDIAN_CALCULATED) != 0)
  {
    return db->header.median;
  }

  KDB_VALUE_TYPE* values = (KDB_VALUE_TYPE*)malloc(sizeof(KDB_VALUE_TYPE) * db->header.count);

  if (!values)
  {
    KDB_ERROR("Could not allocate the memory to calculate the median\n");

    return INFINITY;
  }

  KDB_VALUE_TYPE median = INFINITY;
  KDB_DATA       data;

  for (uint32_t i = 0; i < db->header.count; ++i)
  {
    if (!kdb_get_data(db, i, &data))
    {
      goto defer;
    }

    values[i] = data.value;
  }

  qsort(values, db->header.count, sizeof(KDB_VALUE_TYPE), &kdb_compare_values);

  if ((db->header.count & 1) == 0)
  {
    int second_index = db->header.count / 2;
    int first_index  = second_index - 1;

    median = (values[first_index] + values[second_index]) / 2.0f;
  }
  else
  {
    int index = db->header.count / 2;

    median = values[index];
  }

  KDB_PUSH_HEADER;

  db->header.flags  |= KDB_FLAGS_MEDIAN_CALCULATED;
  db->header.median  = median;

  if (!kdb_write_header(db))
  {
    KDB_POP_HEADER;

    median = INFINITY;

    goto defer;
  }

  defer:
    free(values);

    return median;
}

KDB_VALUE_TYPE kdb_sma(KDB* db, uint32_t index, uint32_t frame)
{
  KDB_CHECK_INITIALIZED(db, 0.0f);

  if (frame == 0)
  {
    return 0.0f;
  }

  KDB_DATA initial = { 0 };
  KDB_DATA final   = { 0 };

  if (!kdb_get_data(db, index - frame + 1, &initial))
  {
    return 0.0f;
  }

  if (!kdb_get_data(db, index, &final))
  {
    return 0.0f;
  }

  KDB_VALUE_TYPE difference = final.sum - initial.sum;
  KDB_VALUE_TYPE sma        = difference / frame;

  return sma;
}
#endif // KDB_IMPLEMENTATION

/* TODO
 * Add comments
 * Create the hashmap
 * Test removing sum from records
 *   Add a cache for optimizing moving averages
 * Add a hashmap for kdb*, so each file has only one file handler
 */
