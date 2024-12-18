/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "img.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "stdio.h"
#include "stdlib.h" // Added for rand()

#include "ILI9341_GFX.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define SCREEN_WIDTH      320
#define SCREEN_HEIGHT     240
#define MAX_LENGTH        100
#define INITIAL_DELAY     200
#define SPEED_INCREMENT   10

#define UP                0
#define RIGHT             1
#define DOWN              2
#define LEFT              3
#define SNAKE_SIZE        16

#define SNOW_WIDTH        16
#define SNOW_HEIGHT       16
#define SNOW_SMALLEST_WIDTH  16
#define SNOW_SMALLEST_HEIGHT 16

#define MAX_XOFFSET 10



#define LETTER_WIDTH      16
#define LETTER_HEIGHT     16
#define LOGO_WIDTH        48
#define LOGO_HEIGHT       48

// Новые макросы для змейки
#define SNAKE_HEAD_WIDTH  16
#define SNAKE_HEAD_HEIGHT 16
#define SNAKE_BODY_WIDTH  16
#define SNAKE_BODY_HEIGHT 16
#define SNAKE_TAIL_WIDTH  16
#define SNAKE_TAIL_HEIGHT 16
/* USER CODE END PTD */

#define GAME_AREA_LEFT 0
#define GAME_AREA_RIGHT SCREEN_WIDTH
#define GAME_AREA_TOP 0 // Если вы используете область счета сверху, иначе установите в 0
#define GAME_AREA_BOTTOM SCREEN_HEIGHT

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

/* USER CODE BEGIN PV */
int16_t snakeX[MAX_LENGTH] = {0};
int16_t snakeY[MAX_LENGTH] = {0};
uint16_t snakeLength = 5;
int direction = UP; // 0 - Up, 1 - Right, 2 - Down, 3 - Left

uint16_t foodX = 0;
uint16_t foodY = 0;

uint16_t snakeSpeed = INITIAL_DELAY;

GPIO_PinState buttonRight;
GPIO_PinState buttonLeft;
GPIO_PinState buttonPrevRight = GPIO_PIN_SET;
GPIO_PinState buttonPrevLeft = GPIO_PIN_SET;

uint32_t lastMoveTime = 0;
uint8_t directionChanged = 0; // Флаг для предотвращения множественных смен направления

/* Добавьте следующие переменные для мигания светодиода */
uint32_t lastLedToggleTime = 0; // Время последнего переключения светодиода
const uint32_t ledBlinkInterval = 500; // Интервал мигания в миллисекундах (0.5 секунды)
/* USER CODE END PV */

/* Добавьте эти глобальные переменные для хранения предыдущих позиций головы и хвоста */
int16_t prevHeadX = -1, prevHeadY = -1;
int16_t prevTailX = -1, prevTailY = -1;

uint8_t snakeJustAteFood = 0;

int prevDirection = UP; // Начальное направление

uint32_t lastButtonPressTime = 0;
const uint32_t buttonDebounceDelay = 200; // Задержка в миллисекундах

uint8_t gameRunning = 0;
uint8_t snakeDead = 0;

// В начале файла, в разделе глобальных переменных
uint8_t isFirstFrame = 1;


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */
void InitSnakeGame(void);
void UpdateSnakePosition(void);
void DrawSnake(void);
void CheckCollision(void);
void GenerateFood(void);
void DrawFood(void);
void RunSnakeGame(void);
void EraseHead(void);
void EraseTail(void);
void DrawClippedFilledRectangle(int x, int y, int width, int height, uint16_t color);
int GetWrappedDelta(int pos1, int pos2, int maxPos);
void ShowStartScreen(void);
void ShowGameOverScreen(void);
void InitializeSnowflakes(void);
void InitializeSnowflakes(void);
void DrawClippedFilledRectangleSnow(int x, int y, int width, int height, uint16_t color);

/* USER CODE END PFP */

uint8_t IsButtonPressed(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState* prevState);





// void DrawCoordinates(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
typedef struct {
    int x;
    int y;
    int prev_x;
    int prev_y;
    int16_t initial_y;
    int speed;
    const uint8_t* image;
    size_t image_size;
    int width;
    int height;
} Snowflake;

// Определение констант для размеров сетки
// Количество колонок и строк для больших снежинок
#define BIG_COLS 5
#define BIG_ROWS 4

// Количество снежинок для каждого типа
#define NUM_LARGE_SNOWFLAKES (BIG_COLS * BIG_ROWS)

// Количество колонок и строк для средних и маленьких снежинок
#define MEDIUM_SMALL_COLS (BIG_COLS - 1)
#define MEDIUM_SMALL_ROWS (BIG_ROWS)
#define NUM_MEDIUM_SMALL_SNOWFLAKES (MEDIUM_SMALL_COLS * MEDIUM_SMALL_ROWS)

// Количество колонок и строк для самых маленьких снежинок
#define XSMALL_COLS 5
#define XSMALL_ROWS 5
#define NUM_XSMALL_SNOWFLAKES (XSMALL_COLS * XSMALL_ROWS)

// Общее количество снежинок
#define SNOWFLAKE_TOTAL (NUM_LARGE_SNOWFLAKES + NUM_MEDIUM_SMALL_SNOWFLAKES + NUM_XSMALL_SNOWFLAKES)
// Объявление массива снежинок
Snowflake snowflakes[SNOWFLAKE_TOTAL];


// Глобальные переменные для области текста и логотипа
int textAreaTop, textAreaBottom;
int logoX, logoY;
int startX, lettersY;
int num_letters;
char *letters;
int spacing;
/**
 * @brief Рисует заполненный прямоугольник с клиппингом по границам экрана.
 * @param x Начальная координата X
 * @param y Начальная координата Y
 * @param width Ширина прямоугольника
 * @param height Высота прямоугольника
 * @param color Цвет заливки
 */

//snake
void DrawClippedFilledRectangle(int x, int y, int width, int height, uint16_t color)
{
    // Ограничиваем координаты внутри экрана
    if (x < 0) {
        width += x; // Уменьшаем ширину на количество пикселей вне экрана слева
        x = 0;
    }
    if (y < 0) {
        height += y; // Уменьшаем высоту на количество пикселей вне экрана сверху
        y = 0;
    }
    if (x + width > SCREEN_WIDTH)
        width = SCREEN_WIDTH - x; // Уменьшаем ширину, если выходит справа
    if (y + height > SCREEN_HEIGHT)
        height = SCREEN_HEIGHT - y; // Уменьшаем высоту, если выходит снизу

    // Рисуем только если ширина и высота положительные
    if (width > 0 && height > 0)
        ILI9341_Draw_Filled_Rectangle_Coord(x, y, x + width, y + height, color);
}
void EraseHead(void)
{
    if (prevHeadX != -1 && prevHeadY != -1) {
        // Размеры головы
        int eraseWidth = SNAKE_HEAD_WIDTH;
        int eraseHeight = SNAKE_HEAD_HEIGHT;

        // Координаты для стирания
        int eraseX = prevHeadX - (SNAKE_HEAD_WIDTH - SNAKE_SIZE) / 2;
        int eraseY = prevHeadY - (SNAKE_HEAD_HEIGHT - SNAKE_SIZE) / 2;

        // Стираем прямоугольник головы с клиппингом
        DrawClippedFilledRectangle(eraseX, eraseY, eraseWidth, eraseHeight, BLACK);
    }
}



void EraseTail(void)
{
    if (prevTailX != -1 && prevTailY != -1) {
        // Размеры хвоста
        int eraseWidth = SNAKE_TAIL_WIDTH;
        int eraseHeight = SNAKE_TAIL_HEIGHT;

        // Координаты для стирания
        int eraseX = prevTailX - (SNAKE_TAIL_WIDTH - SNAKE_SIZE) / 2;
        int eraseY = prevTailY - (SNAKE_TAIL_HEIGHT - SNAKE_SIZE) / 2;

        // Стираем прямоугольник хвоста с клиппингом
        DrawClippedFilledRectangle(eraseX, eraseY, eraseWidth, eraseHeight, BLACK);
    }
}
int GetWrappedDelta(int pos1, int pos2, int maxPos)
{
    int delta = pos2 - pos1;
    if (delta > SNAKE_SIZE) {
        delta -= maxPos;
    } else if (delta < -SNAKE_SIZE) {
        delta += maxPos;
    }
    return delta;
}


void InitializeSnowflakes(void)
{
    int snowflakeIndex = 0;

    // --- Инициализация Больших Снежинок ---
    // Расчёт горизонтального и вертикального расстояния для больших снежинок
    float bigSpacingX = (float)(SCREEN_WIDTH - SNOW_WIDTH) / (BIG_COLS - 1); // (320 - 16) / 4 = 76
    float bigSpacingY = (float)(SCREEN_HEIGHT) / BIG_ROWS; // 240 / 4 = 60

    // Инициализация больших снежинок
    for(int row = 0; row < BIG_ROWS; row++)
    {
        for(int col = 0; col < BIG_COLS; col++)
        {
            if(snowflakeIndex >= SNOWFLAKE_TOTAL) break;

            Snowflake* sf = &snowflakes[snowflakeIndex++];
            sf->width = SNOW_WIDTH;
            sf->height = SNOW_HEIGHT;

            // Определение смещения по Y на основе столбца для шахматного чередования
            float yOffset = 0.0f;
            if(col % 2 == 1)
                yOffset = 10.0f; // Смещение на 10 пикселей для нечётных столбцов (1,3)
            else
                yOffset = -10.0f;  // Смещение на -10 пикселей для чётных столбцов (0,2,4)

            // Добавление случайного смещения по X для хаотичности
            float randXOffset = (rand() % (2 * MAX_XOFFSET + 1)) - MAX_XOFFSET; // Случайное смещение от -10 до +10 пикселей

            // Вычисление позиции снежинки с учётом смещений
            sf->x = (int)(col * bigSpacingX + randXOffset + 0.5f);
            sf->y = (int)(-SNOW_HEIGHT - row * bigSpacingY + yOffset + 0.5f);
            sf->initial_y = sf->y;

            // Проверка выхода за пределы экрана по X и корректировка позиции
            if (sf->x + sf->width > SCREEN_WIDTH)
                sf->x = SCREEN_WIDTH - sf->width;
            if (sf->x < 0)
                sf->x = 0;

            sf->speed = 4; // Скорость для больших снежинок
            sf->image = snow_large;
            sf->image_size = sizeof(snow_large);
            sf->prev_x = sf->x;
            sf->prev_y = sf->y;
        }
    }

    // --- Инициализация Средних и Маленьких Снежинок ---
    // Расчёт расстояния для средних и маленьких снежинок
    float msSpacingX = bigSpacingX; // 76
    float msSpacingY = bigSpacingY; // 60

    // Инициализация средних и маленьких снежинок
    for(int row = 0; row < MEDIUM_SMALL_ROWS; row++)
    {
        for(int col = 0; col < MEDIUM_SMALL_COLS; col++)
        {
            if(snowflakeIndex >= SNOWFLAKE_TOTAL) break;

            Snowflake* sf = &snowflakes[snowflakeIndex++];
            sf->width = SNOW_WIDTH;
            sf->height = SNOW_HEIGHT;

            // Определение смещения по Y на основе столбца для шахматного чередования
            float yOffset = 0.0f;
            if(col % 2 == 1)
                yOffset = 10.0f; // Смещение на 10 пикселей для нечётных столбцов (1,3)
            else
                yOffset = 0.0f;  // Без смещения для чётных столбцов (0,2)

            // Добавление случайного смещения по X для хаотичности
            float randXOffset = (rand() % (2 * MAX_XOFFSET + 1)) - MAX_XOFFSET; // Случайное смещение от -10 до +10 пикселей

            // Вычисление позиции снежинки с учётом смещений
            sf->x = (int)(col * msSpacingX + msSpacingX / 2 + randXOffset + 0.5f); // 38, 114, 190, 266 с рандомным смещением
            sf->y = (int)(-SNOW_HEIGHT - (row + 1) * msSpacingY + yOffset + 0.5f); // Смещаем по Y
            sf->initial_y = sf->y;

            // Проверка выхода за пределы экрана по X и корректировка позиции
            if (sf->x + sf->width > SCREEN_WIDTH)
                sf->x = SCREEN_WIDTH - sf->width;
            if (sf->x < 0)
                sf->x = 0;

            sf->speed = 2; // Скорость для средних и маленьких снежинок

            // Чередование средних и маленьких снежинок в шахматном порядке
            if((row + col) % 2 == 0)
            {
                sf->image = snow_medium;
                sf->image_size = sizeof(snow_medium);
            }
            else
            {
                sf->image = snow_small;
                sf->image_size = sizeof(snow_small);
            }

            sf->prev_x = sf->x;
            sf->prev_y = sf->y;
        }
    }

    // --- Инициализация Самых Маленьких Снежинок ---
    // Расчёт горизонтального и вертикального расстояния для самых маленьких снежинок
    float xsmallSpacingX = (float)(SCREEN_WIDTH - SNOW_SMALLEST_WIDTH) / (XSMALL_COLS - 1); // (320 - 8) / 4 = 78
    float xsmallSpacingY = (float)(SCREEN_HEIGHT) / XSMALL_ROWS; // 240 / 5 = 48

    for(int row = 0; row < XSMALL_ROWS; row++)
    {
        for(int col = 0; col < XSMALL_COLS; col++)
        {
            if(snowflakeIndex >= SNOWFLAKE_TOTAL) break;

            Snowflake* sf = &snowflakes[snowflakeIndex++];
            sf->width = SNOW_SMALLEST_WIDTH;  // Исправлено на 8
            sf->height = SNOW_SMALLEST_HEIGHT; // Исправлено на 8

            // Смещение по Y для равномерного распределения и предотвращения перекрытий
            float yOffset = 0.0f;
            if(row % 2 == 1)
                yOffset = 24.0f; // Смещение на 24 пикселя для нечётных строк
            else
                yOffset = -24.0f; // Смещение на -24 пикселя для чётных строк

            // Добавление случайных смещений для хаотичности
            float randXOffset = (rand() % (2 * MAX_XOFFSET + 1)) - MAX_XOFFSET; // Случайное смещение от -10 до +10 пикселей
            float randYOffset = (rand() % (2 * MAX_XOFFSET + 1)) - MAX_XOFFSET; // Случайное смещение от -10 до +10 пикселей

            // Вычисление позиции снежинки с учётом смещений
            sf->x = (int)(col * xsmallSpacingX + randXOffset + 0.5f); // 0, 78, 156, 234, 312 с рандомным смещением
            sf->y = (int)(-sf->height - row * xsmallSpacingY + yOffset + randYOffset + 0.5f);
            sf->initial_y = sf->y;

            // Проверка выхода за пределы экрана по X и корректировка позиции
            if (sf->x + sf->width > SCREEN_WIDTH)
                sf->x = SCREEN_WIDTH - sf->width;
            if (sf->x < 0)
                sf->x = 0;

            sf->speed = 1 + (rand() % 2); // Скорость 1 или 2

            sf->image = snow_xsmall; // Убедитесь, что это изображение определено
            sf->image_size = sizeof(snow_xsmall);
            sf->prev_x = sf->x;
            sf->prev_y = sf->y;
        }
    }

    // Если необходимо, можно добавить дополнительные проверки для избежания перекрытий
}




void DrawClippedFilledRectangleSnow(int x, int y, int width, int height, uint16_t color)
{
    int x0 = x;
    int y0 = y;
    int x1 = x + width;
    int y1 = y + height;

    // Ограничиваем координаты внутри экрана
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > SCREEN_WIDTH) x1 = SCREEN_WIDTH;
    if (y1 > SCREEN_HEIGHT) y1 = SCREEN_HEIGHT;

    // Вычисляем скорректированную ширину и высоту
    int adjusted_width = x1 - x0;
    int adjusted_height = y1 - y0;

    // Рисуем только если ширина и высота положительные
    if (adjusted_width > 0 && adjusted_height > 0)
    {
        ILI9341_Draw_Filled_Rectangle_Coord(x0, y0, x1 - 1, y1 - 1, color);
    }
}





void UpdateAndDrawSnowflakes(void)
{
    // 1. Сохраняем текущие позиции снежинок перед обновлением
    for (int i = 0; i < SNOWFLAKE_TOTAL; i++)
    {
        snowflakes[i].prev_x = snowflakes[i].x;
        snowflakes[i].prev_y = snowflakes[i].y;
    }

    // 2. Обновляем позиции снежинок
    for (int i = 0; i < SNOWFLAKE_TOTAL; i++)
    {
        snowflakes[i].y += snowflakes[i].speed;

        // Если снежинка выходит за нижний край экрана, перемещаем её наверх
        if (snowflakes[i].y > SCREEN_HEIGHT)
        {
            snowflakes[i].y = -snowflakes[i].height;
        }
    }

    // 3. Стираем предыдущие снежинки, если это не первый кадр
    if (!isFirstFrame)
    {
        for (int i = 0; i < SNOWFLAKE_TOTAL; i++)
        {
            int prev_x = snowflakes[i].prev_x;
            int prev_y = snowflakes[i].prev_y;
            int width = snowflakes[i].width;
            int height = snowflakes[i].height;

            // Проверяем, находится ли снежинка на экране
            if (prev_y + height > 0 && prev_y < SCREEN_HEIGHT)
            {
                DrawClippedFilledRectangleSnow(prev_x, prev_y, width, height, BLACK);
            }
        }
    }

    // 4. Отрисовываем снежинки на новых позициях
    for (int i = 0; i < SNOWFLAKE_TOTAL; i++)
    {
        int x = snowflakes[i].x;
        int y = snowflakes[i].y;
        int width = snowflakes[i].width;
        int height = snowflakes[i].height;

        // Проверяем, находится ли снежинка на экране
        if (y + height > 0 && y < SCREEN_HEIGHT)
        {
            ILI9341_Draw_Image(snowflakes[i].image, x, y, width, height, snowflakes[i].image_size);
        }
    }

    // 5. Перерисовываем логотип и текст, чтобы они были поверх снежинок
    ILI9341_Draw_Image(img_logo, logoX, logoY, LOGO_WIDTH, LOGO_HEIGHT, sizeof(img_logo));

    for (int i = 0; i < num_letters; i++)
    {
        const uint8_t *letter_image;
        size_t letter_size;
        switch (letters[i])
        {
            case 'A':
                letter_image = A;
                letter_size = sizeof(A);
                break;
            case 'P':
                letter_image = P;
                letter_size = sizeof(P);
                break;
            case 'L':
                letter_image = L;
                letter_size = sizeof(L);
                break;
            case 'I':
                letter_image = I;
                letter_size = sizeof(I);
                break;
            case 'N':
                letter_image = N;
                letter_size = sizeof(N);
                break;
            case 'E':
                letter_image = E;
                letter_size = sizeof(E);
                break;
            case 'R':
                letter_image = R;
                letter_size = sizeof(R);
                break;
            default:
                continue;
        }

        int x = startX + i * (LETTER_WIDTH + spacing);
        int y = lettersY;

        // Отображение буквы
        ILI9341_Draw_Image(letter_image, x, y, LETTER_WIDTH, LETTER_HEIGHT, letter_size);
    }


    // 6. Сбрасываем флаг isFirstFrame после первого кадра
    if (isFirstFrame)
    {
        isFirstFrame = 0;
    }
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  srand(HAL_GetTick()); // Инициализация генератора случайных чисел

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  __HAL_SPI_ENABLE(&hspi1); // Исправлено: DISP_SPI_PTR на &hspi1

  // DISP_CS_UNSELECT; // Предполагается, что DISP_CS_UNSELECT – это макрос для снятия выбора CS. Убедитесь, что он определён правильно.

  ILI9341_Init(); // Инициализация дисплея
  ILI9341_Set_Rotation(SCREEN_HORIZONTAL_2);
  ILI9341_Fill_Screen(BLACK);
  /* USER CODE END 2 */

  while (1)
  {
    ShowStartScreen();

    InitSnakeGame(); // Инициализация игры
    RunSnakeGame();
  }
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
  /* USER CODE END 3 */
}

void ShowStartScreen(void)
{
    ILI9341_Fill_Screen(BLACK); 

    const uint8_t *lines[] = {line1, line2, line3, line4};
    const size_t line_sizes[] = {sizeof(line1), sizeof(line2), sizeof(line3), sizeof(line4)};
    const int num_lines = 4;

    // Задание размеров строк (ширина и высота в пикселях)
    const int line_widths[] = {274, 240, 231, 245};
    const int line_heights[] = {19, 17, 18, 17};
    const int line_spacing = 10; // Расстояние между строками в пикселях

    // Начальная позиция Y для первой строки
    int currentY = 60; // Верхний отступ (можно изменить по необходимости)

    for (int i = 0; i < num_lines; i++)
    {
        // Центрирование строки по горизонтали
        int x = (SCREEN_WIDTH - line_widths[i]) / 2;
        int y = currentY;

        // Отображение строки
        ILI9341_Draw_Image(lines[i], x, y, line_widths[i], line_heights[i], line_sizes[i]);

        HAL_Delay(500); // Задержка между отображением строк (0.5 секунды)

        // Обновление позиции Y для следующей строки
        currentY += line_heights[i] + line_spacing;
    }

    // Задержка после отображения всех строк для прочтения пользователем
    HAL_Delay(3000); // 3 секунды (можно настроить по необходимости)

    ILI9341_Fill_Screen(BLACK);

    // Расчет позиций для логотипа и надписи
    int spacing_Y = 10; // Расстояние между логотипом и надписью
    int total_height = LOGO_HEIGHT + spacing_Y + LETTER_HEIGHT; // Общая высота группы (логотип + расстояние + надпись)
    int startY = (SCREEN_HEIGHT - total_height) / 2; // Начальная позиция по вертикали для логотипа

    // Отображение логотипа по центру и чуть выше надписи
    logoX = (SCREEN_WIDTH - LOGO_WIDTH) / 2;
    logoY = startY;

    // Отображение букв "APPLINER" по центру экрана
    letters = "APPLINER";
    num_letters = strlen(letters);

    // Общая ширина надписи (70% ширины экрана)
    int total_word_width = (SCREEN_WIDTH * 70) / 100; // 224 пикселя

    // Ширина всех букв
    int letters_width = num_letters * LETTER_WIDTH; // 8 * 16 = 128 пикселей

    // Общее расстояние между буквами
    int total_spacing = total_word_width - letters_width; // 224 -128=96 пикселей

    // Расстояние между буквами
    spacing = total_spacing / (num_letters - 1); // 96 /7≈13 пикселей

    // Начальная позиция по горизонтали для первой буквы
    startX = (SCREEN_WIDTH - total_word_width) / 2;

    // Позиция по вертикали для букв
    lettersY = startY + LOGO_HEIGHT + spacing_Y;

    // Область, где находится надпись "APPLINER" и логотип
    textAreaTop = startY;
    textAreaBottom = startY + total_height;
    
    // Отображение логотипа
    ILI9341_Draw_Image(img_logo, logoX, logoY, LOGO_WIDTH, LOGO_HEIGHT, sizeof(img_logo));

    // Отображение надписи "APPLINER"
    for (int i = 0; i < num_letters; i++)
    {
        const uint8_t *letter_image;
        size_t letter_size;
        switch (letters[i])
        {
            case 'A':
                letter_image = A;
                letter_size = sizeof(A);
                break;
            case 'P':
                letter_image = P;
                letter_size = sizeof(P);
                break;
            case 'L':
                letter_image = L;
                letter_size = sizeof(L);
                break;
            case 'I':
                letter_image = I;
                letter_size = sizeof(I);
                break;
            case 'N':
                letter_image = N;
                letter_size = sizeof(N);
                break;
            case 'E':
                letter_image = E;
                letter_size = sizeof(E);
                break;
            case 'R':
                letter_image = R;
                letter_size = sizeof(R);
                break;
            default:
                continue;
        }

        int x = startX + i * (LETTER_WIDTH + spacing);
        int y = lettersY;

        // Отображение буквы
        ILI9341_Draw_Image(letter_image, x, y, LETTER_WIDTH, LETTER_HEIGHT, letter_size);

        HAL_Delay(200); // Задержка перед отображением следующей буквы
    }

    InitializeSnowflakes();
    isFirstFrame = 1; // Сбрасываем флаг первого кадра перед началом анимации

    // Инициализация предыдущего состояния кнопки текущим состоянием пина
    buttonPrevRight = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4); // Инициализируем предыдущее состояние кнопки

    // Анимация снежинок до нажатия правой кнопки
    while (1)
    {
        uint32_t frameStartTime = HAL_GetTick();

        UpdateAndDrawSnowflakes();

        // Проверяем нажатие правой кнопки для выхода из анимации и начала игры
        if (IsButtonPressed(GPIOA, GPIO_PIN_4, &buttonPrevRight))
        {
            lastButtonPressTime = HAL_GetTick(); // Фиксируем время нажатия
            break;
        }

        // Ограничение частоты кадров
        uint32_t frameTime = HAL_GetTick() - frameStartTime;
        if (frameTime < 30)
        {
            HAL_Delay(30 - frameTime);
        }
    }

    ILI9341_Fill_Screen(BLACK); // Очищаем экран перед началом игры
}




void ShowGameOverScreen(void)
{
    // Массивы изображений, их размеров и размеров в пикселях
    const uint8_t *gameoverImages[] = {gameover1, gameover2, gameover3, gameover4, gameover5, gameover6};
    const size_t gameoverSizes[] = {sizeof(gameover1), sizeof(gameover2), sizeof(gameover3), sizeof(gameover4), sizeof(gameover5), sizeof(gameover6)};
    const int gameoverWidths[] = {212, 144, 193, 271, 276, 65};
    const int gameoverHeights[] = {23, 15, 16, 13, 13, 15};
    const int num_images = 6;

    // Отображение gameover1
    int x = (SCREEN_WIDTH - gameoverWidths[0]) / 2;
    int y = (SCREEN_HEIGHT - gameoverHeights[0]) / 2;

    ILI9341_Draw_Image(gameoverImages[0], x, y, gameoverWidths[0], gameoverHeights[0], gameoverSizes[0]);
    HAL_Delay(1000); // Задержка в 1 секунду

    // Заливка экрана черным
    ILI9341_Fill_Screen(BLACK);

    // Последовательное отображение оставшихся изображений
    for (int i = 1; i < num_images; i++)
    {
        x = (SCREEN_WIDTH - gameoverWidths[i]) / 2;
        y = (SCREEN_HEIGHT - gameoverHeights[i]) / 2;

        ILI9341_Draw_Image(gameoverImages[i], x, y, gameoverWidths[i], gameoverHeights[i], gameoverSizes[i]);
        HAL_Delay(1000); // Задержка между изображениями
        ILI9341_Fill_Screen(BLACK);
    }

    // Ожидание нажатия правой кнопки для перезапуска игры
    buttonPrevRight = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4); // Инициализируем предыдущее состояние кнопки

    while (1)
    {
        if (IsButtonPressed(GPIOA, GPIO_PIN_4, &buttonPrevRight))
        {
            lastButtonPressTime = HAL_GetTick(); // Фиксируем время нажатия
            break;
        }
        HAL_Delay(100);
    }

    ILI9341_Fill_Screen(BLACK); // Очищаем экран перед началом новой игры
}

void InitSnakeGame(void)
{
    snakeLength = 5;
    // int gridX = (SCREEN_WIDTH / 2) / SNAKE_SIZE;
    // int gridY = (SCREEN_HEIGHT / 2) / SNAKE_SIZE;
    int gridX = ((GAME_AREA_LEFT + GAME_AREA_RIGHT) / 2) / SNAKE_SIZE;
    int gridY = ((GAME_AREA_TOP + GAME_AREA_BOTTOM) / 2) / SNAKE_SIZE;

    for (uint16_t i = 0; i < snakeLength; i++) {
        snakeX[i] = gridX * SNAKE_SIZE;
        snakeY[i] = (gridY + i) * SNAKE_SIZE;
    }
    direction = UP;
    prevDirection = UP;
    snakeSpeed = INITIAL_DELAY;
    GenerateFood();
    lastMoveTime = HAL_GetTick();
    directionChanged = 0;
    prevHeadX = snakeX[0];
    prevHeadY = snakeY[0];
    prevTailX = snakeX[snakeLength - 1];
    prevTailY = snakeY[snakeLength - 1];

    snakeDead = 0; // Сбрасываем флаг смерти змейки

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET); // Выключаем светодиод при инициализации
}

void RunSnakeGame(void)
{
    while (1) // Основной цикл игры
    {
        gameRunning = 1; // Флаг, указывающий на состояние игры

        while (gameRunning)
        {
            uint32_t currentTime = HAL_GetTick();

            // Логика мигания светодиода
            if (currentTime - lastLedToggleTime >= ledBlinkInterval) {
                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0); // Переключаем состояние светодиода
                lastLedToggleTime = currentTime;
            }

            // Обработка нажатий кнопок с антидребезгом
            if (!directionChanged && (currentTime - lastButtonPressTime >= buttonDebounceDelay)) {
                if (IsButtonPressed(GPIOA, GPIO_PIN_4, &buttonPrevRight)) {
                    int newDirection = (direction + 1) % 4;
                    if (newDirection != (direction + 2) % 4) { // Предотвращаем разворот на 180 градусов
                        direction = newDirection;
                        directionChanged = 1;
                        lastButtonPressTime = currentTime;
                    }
                } else if (IsButtonPressed(GPIOA, GPIO_PIN_3, &buttonPrevLeft)) {
                    int newDirection = (direction - 1 + 4) % 4;
                    if (newDirection != (direction + 2) % 4) {
                        direction = newDirection;
                        directionChanged = 1;
                        lastButtonPressTime = currentTime;
                    }
                }
            }

            if (currentTime - lastMoveTime >= snakeSpeed) {
                lastMoveTime = currentTime;

                // Сохраняем предыдущие позиции перед перемещением
                prevHeadX = snakeX[0];
                prevHeadY = snakeY[0];
                prevDirection = direction;
                prevTailX = snakeX[snakeLength - 1];
                prevTailY = snakeY[snakeLength - 1];

                // Стираем предыдущую голову и хвост
                EraseHead();
                EraseTail();

                // Обновляем позиции змейки
                UpdateSnakePosition();

                // Проверяем столкновения
                CheckCollision();

                // Рисуем змейку и еду
                DrawSnake();
                DrawFood();

                directionChanged = 0; // Сброс флага смены направления

                if (snakeDead) {
                    gameRunning = 0; // Завершаем текущую игру
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET); // Выключаем светодиод при смерти змейки
                }
            }
        }

        if (snakeDead)
        {
            ShowGameOverScreen();
            // После отображения экрана Game Over, перезапускаем игру
            InitSnakeGame();
        }
        else
        {
            break; // Выходим из основного цикла игры, если игра завершилась не из-за смерти змейки
        }
    }

    // После выхода из игрового цикла выключаем светодиод
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
}


void UpdateSnakePosition(void)
{
    // Перемещаем тело змейки
    for (int i = snakeLength - 1; i > 0; i--) {
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
    }
    
    // Перемещаем голову
    switch (direction) {
        case UP:
            snakeY[0] -= SNAKE_SIZE;
            break;
        case RIGHT:
            snakeX[0] += SNAKE_SIZE;
            break;
        case DOWN:
            snakeY[0] += SNAKE_SIZE;
            break;
        case LEFT:
            snakeX[0] -= SNAKE_SIZE;
            break;
    }

    // Проверяем выход за пределы области игры по X
    if (snakeX[0] >= GAME_AREA_RIGHT) {
        snakeX[0] = GAME_AREA_LEFT;
    } else if (snakeX[0] < GAME_AREA_LEFT) {
        snakeX[0] = GAME_AREA_RIGHT - SNAKE_SIZE;
    }

    // Проверяем выход за пределы области игры по Y
    if (snakeY[0] >= GAME_AREA_BOTTOM) {
        snakeY[0] = GAME_AREA_TOP;
    } else if (snakeY[0] < GAME_AREA_TOP) {
        snakeY[0] = GAME_AREA_BOTTOM - SNAKE_SIZE;
    }

    // Если змейка съела еду, увеличиваем длину
    if (snakeJustAteFood) {
        snakeLength++;
        snakeJustAteFood = 0;
    }
}

void DrawSnake(void)
{
    // Отрисовка головы
    int headX = snakeX[0];
    int headY = snakeY[0];
    switch (direction) {
        case UP:
            ILI9341_Draw_Image(snake_head_up, headX - (SNAKE_HEAD_WIDTH - SNAKE_SIZE) / 2, headY - (SNAKE_HEAD_HEIGHT - SNAKE_SIZE) / 2, SNAKE_HEAD_WIDTH, SNAKE_HEAD_HEIGHT, sizeof(snake_head_up));
            break;
        case RIGHT:
            ILI9341_Draw_Image(snake_head_right, headX - (SNAKE_HEAD_WIDTH - SNAKE_SIZE) / 2, headY - (SNAKE_HEAD_HEIGHT - SNAKE_SIZE) / 2, SNAKE_HEAD_WIDTH, SNAKE_HEAD_HEIGHT, sizeof(snake_head_right));
            break;
        case DOWN:
            ILI9341_Draw_Image(snake_head_down, headX - (SNAKE_HEAD_WIDTH - SNAKE_SIZE) / 2, headY - (SNAKE_HEAD_HEIGHT - SNAKE_SIZE) / 2, SNAKE_HEAD_WIDTH, SNAKE_HEAD_HEIGHT, sizeof(snake_head_down));
            break;
        case LEFT:
            ILI9341_Draw_Image(snake_head_left, headX - (SNAKE_HEAD_WIDTH - SNAKE_SIZE) / 2, headY - (SNAKE_HEAD_HEIGHT - SNAKE_SIZE) / 2, SNAKE_HEAD_WIDTH, SNAKE_HEAD_HEIGHT, sizeof(snake_head_left));
            break;
    }

    // Отрисовка тела
    for (uint16_t i = 1; i < snakeLength - 1; i++) {
        int dx1 = GetWrappedDelta(snakeX[i - 1], snakeX[i], GAME_AREA_RIGHT);
        int dy1 = GetWrappedDelta(snakeY[i - 1], snakeY[i], GAME_AREA_BOTTOM);
        int dx2 = GetWrappedDelta(snakeX[i], snakeX[i + 1], GAME_AREA_RIGHT);
        int dy2 = GetWrappedDelta(snakeY[i], snakeY[i + 1], GAME_AREA_BOTTOM);

        if ((dx1 == 0 && dy1 == -SNAKE_SIZE && dx2 == SNAKE_SIZE && dy2 == 0) ||
            (dx1 == -SNAKE_SIZE && dy1 == 0 && dx2 == 0 && dy2 == SNAKE_SIZE)) {
            ILI9341_Draw_Image(snake_turn_up_to_right, snakeX[i] - (SNAKE_BODY_WIDTH - SNAKE_SIZE) / 2, snakeY[i] - (SNAKE_BODY_HEIGHT - SNAKE_SIZE) / 2, SNAKE_BODY_WIDTH, SNAKE_BODY_HEIGHT, sizeof(snake_turn_up_to_right));
        } else if ((dx1 == 0 && dy1 == SNAKE_SIZE && dx2 == -SNAKE_SIZE && dy2 == 0) ||
                   (dx1 == SNAKE_SIZE && dy1 == 0 && dx2 == 0 && dy2 == -SNAKE_SIZE)) {
            ILI9341_Draw_Image(snake_turn_down_to_left, snakeX[i] - (SNAKE_BODY_WIDTH - SNAKE_SIZE) / 2, snakeY[i] - (SNAKE_BODY_HEIGHT - SNAKE_SIZE) / 2, SNAKE_BODY_WIDTH, SNAKE_BODY_HEIGHT, sizeof(snake_turn_down_to_left));
        } else if ((dx1 == 0 && dy1 == SNAKE_SIZE && dx2 == SNAKE_SIZE && dy2 == 0) ||
                   (dx1 == -SNAKE_SIZE && dy1 == 0 && dx2 == 0 && dy2 == -SNAKE_SIZE)) {
            ILI9341_Draw_Image(snake_turn_down_to_right, snakeX[i] - (SNAKE_BODY_WIDTH - SNAKE_SIZE) / 2, snakeY[i] - (SNAKE_BODY_HEIGHT - SNAKE_SIZE) / 2, SNAKE_BODY_WIDTH, SNAKE_BODY_HEIGHT, sizeof(snake_turn_down_to_right));
        } else if ((dx1 == 0 && dy1 == -SNAKE_SIZE && dx2 == -SNAKE_SIZE && dy2 == 0) ||
                   (dx1 == SNAKE_SIZE && dy1 == 0 && dx2 == 0 && dy2 == SNAKE_SIZE)) {
            ILI9341_Draw_Image(snake_turn_up_to_left, snakeX[i] - (SNAKE_BODY_WIDTH - SNAKE_SIZE) / 2, snakeY[i] - (SNAKE_BODY_HEIGHT - SNAKE_SIZE) / 2, SNAKE_BODY_WIDTH, SNAKE_BODY_HEIGHT, sizeof(snake_turn_up_to_left));
        } else {
            if (dx1 == 0) {
                ILI9341_Draw_Image(snake_body_vertical, snakeX[i] - (SNAKE_BODY_WIDTH - SNAKE_SIZE) / 2, snakeY[i] - (SNAKE_BODY_HEIGHT - SNAKE_SIZE) / 2, SNAKE_BODY_WIDTH, SNAKE_BODY_HEIGHT, sizeof(snake_body_vertical));
            } else {
                ILI9341_Draw_Image(snake_body_horizontal, snakeX[i] - (SNAKE_BODY_WIDTH - SNAKE_SIZE) / 2, snakeY[i] - (SNAKE_BODY_HEIGHT - SNAKE_SIZE) / 2, SNAKE_BODY_WIDTH, SNAKE_BODY_HEIGHT, sizeof(snake_body_horizontal));
            }
        }
    }

    if (snakeLength > 1) {
        uint16_t tailIndex = snakeLength - 1;
        int tailX = snakeX[tailIndex];
        int tailY = snakeY[tailIndex];

        // Вычисляем разницу координат между предпоследним и последним сегментами
        int dx = GetWrappedDelta(snakeX[tailIndex - 1], snakeX[tailIndex], GAME_AREA_RIGHT);
        int dy = GetWrappedDelta(snakeY[tailIndex - 1], snakeY[tailIndex], GAME_AREA_BOTTOM);

        int tailDirection;
        if (dx == 0 && dy == -SNAKE_SIZE)
            tailDirection = UP; // Хвост должен смотреть вверх
        else if (dx == 0 && dy == SNAKE_SIZE)
            tailDirection = DOWN; // Хвост должен смотреть вниз
        else if (dx == -SNAKE_SIZE && dy == 0)
            tailDirection = LEFT; // Хвост должен смотреть влево
        else if (dx == SNAKE_SIZE && dy == 0)
            tailDirection = RIGHT; // Хвост должен смотреть вправо

        switch (tailDirection) {
            case UP:
                ILI9341_Draw_Image(snake_tail_up, tailX - (SNAKE_TAIL_WIDTH - SNAKE_SIZE) / 2,
                                   tailY - (SNAKE_TAIL_HEIGHT - SNAKE_SIZE) / 2,
                                   SNAKE_TAIL_WIDTH, SNAKE_TAIL_HEIGHT, sizeof(snake_tail_up));
                break;
            case RIGHT:
                ILI9341_Draw_Image(snake_tail_right, tailX - (SNAKE_TAIL_WIDTH - SNAKE_SIZE) / 2,
                                   tailY - (SNAKE_TAIL_HEIGHT - SNAKE_SIZE) / 2,
                                   SNAKE_TAIL_WIDTH, SNAKE_TAIL_HEIGHT, sizeof(snake_tail_right));
                break;
            case DOWN:
                ILI9341_Draw_Image(snake_tail_down, tailX - (SNAKE_TAIL_WIDTH - SNAKE_SIZE) / 2,
                                   tailY - (SNAKE_TAIL_HEIGHT - SNAKE_SIZE) / 2,
                                   SNAKE_TAIL_WIDTH, SNAKE_TAIL_HEIGHT, sizeof(snake_tail_down));
                break;
            case LEFT:
                ILI9341_Draw_Image(snake_tail_left, tailX - (SNAKE_TAIL_WIDTH - SNAKE_SIZE) / 2,
                                   tailY - (SNAKE_TAIL_HEIGHT - SNAKE_SIZE) / 2,
                                   SNAKE_TAIL_WIDTH, SNAKE_TAIL_HEIGHT, sizeof(snake_tail_left));
                break;
        }
    }
}
void CheckCollision(void)
{
    // Проверка столкновения с собой
    for (uint16_t i = 1; i < snakeLength; i++) {
        if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
            snakeDead = 1;
            return;
        }
    }

    // Проверка столкновения с едой
    if (snakeX[0] == foodX && snakeY[0] == foodY) {
        snakeJustAteFood = 1;
        GenerateFood();
        if (snakeSpeed > SPEED_INCREMENT) {
            snakeSpeed -= SPEED_INCREMENT;
        }
    }
}

void GenerateFood(void)
{
    uint8_t validPosition = 0;
    while (!validPosition) {
        validPosition = 1; // Предполагаем, что позиция валидна
        foodX = ((rand() % ((GAME_AREA_RIGHT - GAME_AREA_LEFT) / SNAKE_SIZE)) * SNAKE_SIZE) + GAME_AREA_LEFT;
        foodY = ((rand() % ((GAME_AREA_BOTTOM - GAME_AREA_TOP) / SNAKE_SIZE)) * SNAKE_SIZE) + GAME_AREA_TOP;

        // Проверяем, чтобы еда не появилась на теле змейки
        for (uint16_t i = 0; i < snakeLength; i++) {
            if (snakeX[i] == foodX && snakeY[i] == foodY) {
                validPosition = 0; // Позиция не валидна, повторяем генерацию
                break;
            }
        }
    }
}

void DrawFood(void)
{
    ILI9341_Draw_Image(chiken, foodX, foodY, 16, 16, sizeof(chiken));
}

uint8_t IsButtonPressed(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState* prevState)
{
    GPIO_PinState currentState = HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
    uint8_t pressed = 0;

    if (currentState == GPIO_PIN_RESET && *prevState == GPIO_PIN_SET) {
        pressed = 1;
    }
    *prevState = currentState;
    return pressed;
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL13;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */
  /* Link DMA handle to SPI handle */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

 
  /* DMA interrupt init */
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, TFT_RST_Pin|TFT_DC_Pin|TFT_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level for LED */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET); // Изначально светодиод выключен

  /*Configure GPIO pins : PA3 PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 (LED) */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // Настраиваем как выход
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : TFT_RST_Pin TFT_DC_Pin TFT_CS_Pin */
  GPIO_InitStruct.Pin = TFT_RST_Pin|TFT_DC_Pin|TFT_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET); // Выключаем светодиод при ошибке
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
