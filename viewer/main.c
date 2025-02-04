#include <stdlib.h>

#include "raylib.h"

#define KDB_IMPLEMENTATION
#include "../kdb.h"

#define SMA_SHORT 15
#define SMA_LONG  30

int main()
{
  KDB_INITIALIZE(db, "test");

  if (!db)
  {
    return 1;
  }

  KDB_DATA data;
  float* values = malloc(db->header.count * sizeof(float));
  float* sma1 = malloc(db->header.count * sizeof(float));
  float* sma2 = malloc(db->header.count * sizeof(float));
  char* buffer1 = malloc(256 * sizeof(char));
  char* buffer2 = malloc(256 * sizeof(char));
  char* buffer3 = malloc(256 * sizeof(char));
  char* buffer4 = malloc(256 * sizeof(char));

  for (size_t i = 0; i < db->header.count; ++i)
  {
    kdb_get_data(db, i, &data);

    values[i] = data.value;

    sma1[i] = kdb_sma(db, i, SMA_SHORT);
    sma2[i] = kdb_sma(db, i, SMA_LONG);
  }

  const int screenWidth = 1024;
  const int screenHeight = 768;

  InitWindow(screenWidth, screenHeight, "KrakluniaDB Visualizer");

  SetTargetFPS(60);

  while (!WindowShouldClose())
  {
    BeginDrawing();
      ClearBackground(CLITERAL(Color){ 51, 51, 51, 255 });

      float previous_x = 0.0f;
      float previous_y1 = 1.0f;
      float previous_y2 = 1.0f;
      float previous_y3 = 1.0f;

      float x = 0.0f;
      float y1 = 1.0f;
      float y2 = 1.0f;
      float y3 = 1.0f;

      for (size_t i = 0; i < db->header.count; ++i)
      {
        x = ((float)i / db->header.count);

        y1 = (1.0f - values[i]);
        y2 = (1.0f - sma1[i]);
        y3 = (1.0f - sma2[i]);

        DrawLine(
          (int)(previous_x * screenWidth), (int)(previous_y1 * screenHeight),
          (int)(x * screenWidth), (int)(y1 * screenHeight),
          CLITERAL(Color){ 0, 0, 255, 255 }
        );

        DrawLine(
          (int)(previous_x * screenWidth), (int)(previous_y2 * screenHeight),
          (int)(x * screenWidth), (int)(y2 * screenHeight),
          CLITERAL(Color){ 0, 255, 0, 255 }
        );

        DrawLine(
          (int)(previous_x * screenWidth), (int)(previous_y3 * screenHeight),
          (int)(x * screenWidth), (int)(y3 * screenHeight),
          CLITERAL(Color){ 255, 0, 0, 255 }
        );

        previous_x = x;
        previous_y1 = y1;
        previous_y2 = y2;
        previous_y3 = y3;
      }

      int mouse_x = GetMouseX();
      int mouse_y = GetMouseY();

      DrawLine(
        mouse_x, 0,
        mouse_x, screenHeight,
        CLITERAL(Color){ 255, 255, 255, 255 }
      );

      DrawLine(
        0, mouse_y,
        screenWidth, mouse_y,
        CLITERAL(Color){ 255, 255, 255, 255 }
      );

      size_t index = ((float)mouse_x / screenWidth) * db->header.count;

      sprintf(buffer1, "Index %lu", index);
      sprintf(buffer2, "%f", values[index]);
      sprintf(buffer3, "%f", sma1[index]);
      sprintf(buffer4, "%f", sma2[index]);

      int font_size = screenHeight * 0.03f;

      int buffer1_width = MeasureText(buffer1, font_size);
      int buffer2_width = MeasureText(buffer2, font_size);
      int buffer3_width = MeasureText(buffer3, font_size);
      int buffer4_width = MeasureText(buffer4, font_size);

      int max_width = fmax(fmax(fmax(buffer1_width, buffer2_width), buffer3_width), buffer4_width);

      int origin_x = mouse_x;
      int origin_y = mouse_y;

      if (origin_x > screenWidth - max_width - 10)
      {
        origin_x -= max_width + 10;
      }

      if (origin_y < font_size * 4 + 5)
      {
        origin_y += font_size * 4 + 5;
      }

      DrawText(buffer1, origin_x + 5, origin_y - font_size * 4, font_size, CLITERAL(Color){ 255, 255, 255, 255 });
      DrawText(buffer2, origin_x + 5, origin_y - font_size * 3, font_size, CLITERAL(Color){ 0, 0, 255, 255 });
      DrawText(buffer3, origin_x + 5, origin_y - font_size * 2, font_size, CLITERAL(Color){ 0, 255, 0, 255 });
      DrawText(buffer4, origin_x + 5, origin_y - font_size, font_size, CLITERAL(Color){ 255, 0, 0, 255 });
    EndDrawing();
  }

  CloseWindow();

  free(buffer4);
  free(buffer3);
  free(buffer2);
  free(buffer1);
  free(sma2);
  free(sma1);
  free(values);

  KDB_FINALIZE(db);

  return 0;
}
