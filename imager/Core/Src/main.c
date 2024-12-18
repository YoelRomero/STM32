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

#define STAR_WIDTH        16
#define STAR_HEIGHT       16




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


#define NUM_ELEMENTS 4




// Конфигурация звёзд
#define NUM_STAR_XSMALL 20
#define NUM_STAR_SMALL 10
#define NUM_STAR_MEDIUM 7
#define NUM_STAR_LARGE 3
#define TOTAL_STARS (NUM_STAR_XSMALL + NUM_STAR_SMALL + NUM_STAR_MEDIUM + NUM_STAR_LARGE)
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
void DisplayStarsAndLogo(void);
void DrawNewYearGreeting(void);

void InitializeStars(void);
void DrawStarsGradually(void);
void UpdateStarPositions(void);
/* USER CODE BEGIN PFP */
int isOverlappingExclusion(int x, int y, int width, int height);

/* USER CODE END PFP */


/* USER CODE END PFP */

uint8_t IsButtonPressed(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState* prevState);





// void DrawCoordinates(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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


// Определение типов звёзд
typedef enum {
    STAR_XSMALL,
    STAR_SMALL,
    STAR_MEDIUM,
    STAR_LARGE
} StarType;

// Структура звезды
typedef struct {
    StarType type;
    int x;
    int y;
} Star;

/* Массив звёзд */
Star stars[TOTAL_STARS];
/* USER CODE END 0 */
// Интервал обновления звёзд (10 секунд)
#define STAR_UPDATE_INTERVAL 5000 // миллисекунд


typedef struct {
    int x;
    int y;
    int width;
    int height;
} Rectangle;

#define NUM_EXCLUSION_AREAS 3
Rectangle exclusionAreas[NUM_EXCLUSION_AREAS];


// Объявление размеров элементов поздравления "С новым годом!!!"
const int element_widths[NUM_ELEMENTS] = {
    13,  // Примерная ширина символа "С"
    102, // Примерная ширина строки "новым"
    99,  // Примерная ширина строки "годом"
    24   // Примерная ширина "!!!"
};

const int element_heights[NUM_ELEMENTS] = {
    16, // Высота символа "С"
    17, // Высота строки "новым"
    17, // Высота строки "годом"
    18  // Высота "!!!"
};

// Функция для инициализации звёзд
void InitializeStars(void) {
    int index = 0;
    // Инициализация звёзд xsmall
    for (int i = 0; i < NUM_STAR_XSMALL; i++) {
        do {
            stars[index].x = rand() % (SCREEN_WIDTH - STAR_WIDTH);
            stars[index].y = rand() % (SCREEN_HEIGHT - STAR_HEIGHT);
        } while (isOverlappingExclusion(stars[index].x, stars[index].y, STAR_WIDTH, STAR_HEIGHT));
        stars[index].type = STAR_XSMALL;
        index++;
    }

    // Инициализация звёзд small
    for (int i = 0; i < NUM_STAR_SMALL; i++) {
        do {
            stars[index].x = rand() % (SCREEN_WIDTH - STAR_WIDTH);
            stars[index].y = rand() % (SCREEN_HEIGHT - STAR_HEIGHT);
        } while (isOverlappingExclusion(stars[index].x, stars[index].y, STAR_WIDTH, STAR_HEIGHT));
        stars[index].type = STAR_SMALL;
        index++;
    }

    // Инициализация звёзд medium
    for (int i = 0; i < NUM_STAR_MEDIUM; i++) {
        do {
            stars[index].x = rand() % (SCREEN_WIDTH - STAR_WIDTH);
            stars[index].y = rand() % (SCREEN_HEIGHT - STAR_HEIGHT);
        } while (isOverlappingExclusion(stars[index].x, stars[index].y, STAR_WIDTH, STAR_HEIGHT));
        stars[index].type = STAR_MEDIUM;
        index++;
    }

    // Инициализация звёзд large
    for (int i = 0; i < NUM_STAR_LARGE; i++) {
        do {
            stars[index].x = rand() % (SCREEN_WIDTH - STAR_WIDTH);
            stars[index].y = rand() % (SCREEN_HEIGHT - STAR_HEIGHT);
        } while (isOverlappingExclusion(stars[index].x, stars[index].y, STAR_WIDTH, STAR_HEIGHT));
        stars[index].type = STAR_LARGE;
        index++;
    }
}


/**
 * @brief Плавно отображает звёзды по типам: сначала маленькие, затем средние, потом большие.
 */
void DrawStarsGradually(void)
{
    // Определяем порядок типов звёзд
    StarType starTypes[] = {STAR_XSMALL, STAR_SMALL, STAR_MEDIUM, STAR_LARGE};
    const int numTypes = sizeof(starTypes) / sizeof(StarType);

    for (int t = 0; t < numTypes; t++) {
        StarType currentType = starTypes[t];
        for (int i = 0; i < TOTAL_STARS; i++) {
            if (stars[i].type == currentType) {
                // Убедимся, что звезда не пересекается с областями исключения
                if (isOverlappingExclusion(stars[i].x, stars[i].y, STAR_WIDTH, STAR_HEIGHT)) {
                    continue; // Пропускаем эту звезду
                }

                // Отрисовка звезды
                switch (stars[i].type) {
                    case STAR_XSMALL:
                        ILI9341_Draw_Image(star_xsmall, stars[i].x, stars[i].y, STAR_WIDTH, STAR_HEIGHT, sizeof(star_xsmall));
                        break;
                    case STAR_SMALL:
                        ILI9341_Draw_Image(star_small, stars[i].x, stars[i].y, STAR_WIDTH, STAR_HEIGHT, sizeof(star_small));
                        break;
                    case STAR_MEDIUM:
                        ILI9341_Draw_Image(star_medium, stars[i].x, stars[i].y, STAR_WIDTH, STAR_HEIGHT, sizeof(star_medium));
                        break;
                    case STAR_LARGE:
                        ILI9341_Draw_Image(star_large, stars[i].x, stars[i].y, STAR_WIDTH, STAR_HEIGHT, sizeof(star_large));
                        break;
                    default:
                        break;
                }
                HAL_Delay(20); // Задержка между отображением звёзд одного типа (0.01 секунды)
            }
        }
        HAL_Delay(50); // Задержка между типами звёзд (0.05 секунды)
    }
}




// Функция для обновления позиций звёзд
void UpdateStarPositions(void) {
    for (int i = 0; i < TOTAL_STARS; i++) {
        // Очистка старой позиции звезды
        DrawClippedFilledRectangle(stars[i].x, stars[i].y, STAR_WIDTH, STAR_HEIGHT, BLACK);

        // Обновление координат звезды
        do {
            stars[i].x = rand() % (SCREEN_WIDTH - STAR_WIDTH);
            stars[i].y = rand() % (SCREEN_HEIGHT - STAR_HEIGHT);
        } while (isOverlappingExclusion(stars[i].x, stars[i].y, STAR_WIDTH, STAR_HEIGHT));

        // Перерисовка звезды в новой позиции
        switch (stars[i].type) {
            case STAR_XSMALL:
                ILI9341_Draw_Image(star_xsmall, stars[i].x, stars[i].y, STAR_WIDTH, STAR_HEIGHT, sizeof(star_xsmall));
                break;
            case STAR_SMALL:
                ILI9341_Draw_Image(star_small, stars[i].x, stars[i].y, STAR_WIDTH, STAR_HEIGHT, sizeof(star_small));
                break;
            case STAR_MEDIUM:
                ILI9341_Draw_Image(star_medium, stars[i].x, stars[i].y, STAR_WIDTH, STAR_HEIGHT, sizeof(star_medium));
                break;
            case STAR_LARGE:
                ILI9341_Draw_Image(star_large, stars[i].x, stars[i].y, STAR_WIDTH, STAR_HEIGHT, sizeof(star_large));
                break;
            default:
                break;
        }
    }

    // Повторное рисование логотипа и надписей, чтобы они оставались поверх звёзд
    ILI9341_Draw_Image(img_logo, exclusionAreas[0].x, exclusionAreas[0].y, exclusionAreas[0].width, exclusionAreas[0].height, sizeof(img_logo));

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

        int x = exclusionAreas[1].x + i * (LETTER_WIDTH + spacing);
        int y = exclusionAreas[1].y;

        // Отображение буквы
        ILI9341_Draw_Image((const unsigned char*)letter_image, x, y, LETTER_WIDTH, LETTER_HEIGHT, letter_size);
    }

    // Повторное рисование поздравления "С новым годом!!!"
    DrawNewYearGreeting();
}




/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

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
  HAL_Delay(100);
  srand(HAL_GetTick()); // Инициализация генератора случайных чисел

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


/**
 * @brief Отображает надпись "С новым годом!!!" в одну строку ниже "APPLINER".
 */
void DrawNewYearGreeting(void)
{
    // Определение последовательности элементов надписи
    const uint8_t* greetingElements[NUM_ELEMENTS] = {
        symbol1,       // "С"
        string1,       // "новым"
        string2,       // "годом"
        symbol2        // "!!!"
    };
    
    // Определение размеров каждого элемента
    const int element_heights[NUM_ELEMENTS] = {
        16, // Высота символа "С"
        17, // Высота строки "новым"
        17, // Высота строки "годом"
        18  // Высота "!!!"
    };
    
    // Определение ширины каждого элемента
    const int element_widths[NUM_ELEMENTS] = {
        13,  // Примерная ширина символа "С"
        102, // Примерная ширина строки "новым"
        99, // Примерная ширина строки "годом"
        24   // Примерная ширина "!!!"
    };
    
    // Вычисление общей ширины надписи
    int total_width = 0;
    const int spacing = 10; // Расстояние между элементами в пикселях
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        total_width += element_widths[i];
        if (i < NUM_ELEMENTS - 1) {
            total_width += spacing;
        }
    }
    
    // Вычисление начальной позиции X для центрирования надписи
    int startX = (SCREEN_WIDTH - total_width) / 2;
    
    // Вычисление позиции Y ниже надписи "APPLINER"
    int startY = lettersY + LETTER_HEIGHT + 30; // 10 пикселей отступа
    /////////////////////////////////////////////////////////////////////////
    int total_width_greeting = total_width; // Используйте правильное имя переменной
    int max_element_height = 0;
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        if (element_heights[i] > max_element_height) {
            max_element_height = element_heights[i];
        }
    }
    exclusionAreas[2].width = total_width_greeting;
    exclusionAreas[2].height = max_element_height;


    // Отображение каждого элемента надписи
    for (int i = 0; i < NUM_ELEMENTS; i++)
    {
        const uint8_t* element = greetingElements[i];
        int width = element_widths[i];
        int height = element_heights[i];
        size_t size = 0;
        
        // Определение размера изображения
        switch (i)
        {
            case 0:
                size = sizeof(symbol1);
                break;
            case 1:
                size = sizeof(string1);
                break;
            case 2:
                size = sizeof(string2);
                break;
            case 3:
                size = sizeof(symbol2);
                break;
            default:
                size = 0;
                break;
        }
        
        // Отображение элемента
        ILI9341_Draw_Image(element, startX, startY, width, height, size);
        
        // Обновление позиции X для следующего элемента
        startX += width + spacing;
    }
}

/* USER CODE BEGIN 0 */
/**
 * @brief Проверяет, пересекается ли прямоугольник (x, y, width, height) с любым из областей исключения.
 * @param x Начальная координата X
 * @param y Начальная координата Y
 * @param width Ширина прямоугольника
 * @param height Высота прямоугольника
 * @return 1, если пересекается, иначе 0
 */
int isOverlappingExclusion(int x, int y, int width, int height) {
    for (int i = 0; i < NUM_EXCLUSION_AREAS; i++) {
        if (x < exclusionAreas[i].x + exclusionAreas[i].width &&
            x + width > exclusionAreas[i].x &&
            y < exclusionAreas[i].y + exclusionAreas[i].height &&
            y + height > exclusionAreas[i].y) {
            return 1; // Пересекается
        }
    }
    return 0; // Не пересекается
}

/* USER CODE END 0 */







// /**
//  * @brief Отображает звёзды различных размеров и логотип с надписью "APPLINER".
//  */
// void DisplayStarsAndLogo(void)
// {
//     // Обновляем позиции звёзд
//     UpdateStarPositions();

//     // Отрисовка логотипа и надписей
//     // Отрисовка логотипа
//     ILI9341_Draw_Image(img_logo, logoX, logoY, LOGO_WIDTH, LOGO_HEIGHT, sizeof(img_logo));

//     // Отрисовка надписи "APPLINER"
//     for (int i = 0; i < num_letters; i++)
//     {
//         const uint8_t *letter_image;
//         size_t letter_size;
//         switch (letters[i])
//         {
//             case 'A':
//                 letter_image = A;
//                 letter_size = sizeof(A);
//                 break;
//             case 'P':
//                 letter_image = P;
//                 letter_size = sizeof(P);
//                 break;
//             case 'L':
//                 letter_image = L;
//                 letter_size = sizeof(L);
//                 break;
//             case 'I':
//                 letter_image = I;
//                 letter_size = sizeof(I);
//                 break;
//             case 'N':
//                 letter_image = N;
//                 letter_size = sizeof(N);
//                 break;
//             case 'E':
//                 letter_image = E;
//                 letter_size = sizeof(E);
//                 break;
//             case 'R':
//                 letter_image = R;
//                 letter_size = sizeof(R);
//                 break;
//             default:
//                 continue;
//         }

//         int x = startX + i * (LETTER_WIDTH + spacing);
//         int y = lettersY;

//         // Отображение буквы
//         ILI9341_Draw_Image((const unsigned char*)letter_image, x, y, LETTER_WIDTH, LETTER_HEIGHT, letter_size);
//     }
// }


void ShowStartScreen(void)
{
    ILI9341_Fill_Screen(BLACK); 

    const uint8_t *lines[] = {line1, line2, line3, line4, line5};
    const size_t line_sizes[] = {sizeof(line1), sizeof(line2), sizeof(line3), sizeof(line4), sizeof(line5)};
    const int num_lines = 5;

    // Задание размеров строк (ширина и высота в пикселях)
    const int line_widths[] = {18, 137, 133, 50, 137};
    const int line_heights[] = {22, 23, 23, 22, 23};
    const int line_spacing = 20; // Расстояние между строками в пикселях
    const int horizontal_spacing = 15;

    // Начальная позиция Y для первой строки
    const int startY1 = 80;
    const int startY2 = startY1 + line_heights[0] + line_spacing;

    // Группа 1: lines1, lines2, lines3
    int currentX = (SCREEN_WIDTH - (line_widths[0] + horizontal_spacing + line_widths[1] + horizontal_spacing + line_widths[2])) / 2;
    int y1 = startY1;

    for (int i = 0; i < 3; i++)
    {
        // Отображение строки
        ILI9341_Draw_Image(lines[i], currentX, y1, line_widths[i], line_heights[i], line_sizes[i]);

        HAL_Delay(500); // Задержка между отображением изображений (0.5 секунды)

        // Обновление позиции X для следующего изображения в строке
        currentX += line_widths[i] + horizontal_spacing;
    }

    // Группа 2: lines4, lines5
    currentX = (SCREEN_WIDTH - (line_widths[3] + horizontal_spacing + line_widths[4])) / 2;
    int y2 = startY2;

    for (int i = 3; i < 5; i++)
    {
        // Отображение строки
        ILI9341_Draw_Image(lines[i], currentX, y2, line_widths[i], line_heights[i], line_sizes[i]);

        HAL_Delay(500); // Задержка между отображением изображений (0.5 секунды)

        // Обновление позиции X для следующего изображения в строке
        currentX += line_widths[i] + horizontal_spacing;
    }

    // Задержка после отображения всех строк для прочтения пользователем
    HAL_Delay(3000); // 3 секунды (можно настроить по необходимости)

    ILI9341_Fill_Screen(BLACK);

    // Инициализация и отрисовка звёзд
    InitializeStars();
    DrawStarsGradually();

    // Расчет позиций для логотипа и надписи
    int spacing_Y = 10; // Расстояние между логотипом и надписью
    int total_height = LOGO_HEIGHT + spacing_Y + LETTER_HEIGHT; // Общая высота группы (логотип + расстояние + надпись)
    int startY_logo = (SCREEN_HEIGHT - total_height) / 3; // Начальная позиция по вертикали для логотипа

    // Отображение логотипа по центру и чуть выше надписи
    logoX = (SCREEN_WIDTH - LOGO_WIDTH) / 2;
    logoY = startY_logo;

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
    lettersY = startY_logo + LOGO_HEIGHT + spacing_Y;

    // Отображение логотипа
    ILI9341_Draw_Image(img_logo, logoX, logoY, LOGO_WIDTH, LOGO_HEIGHT, sizeof(img_logo));

    // Отображение надписи "APPLINER" с задержкой между буквами
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
        ILI9341_Draw_Image((const unsigned char*)letter_image, x, y, LETTER_WIDTH, LETTER_HEIGHT, letter_size);

        HAL_Delay(200); // Задержка перед отображением следующей буквы
    }

    // Отображение поздравления "С новым годом!!!"
    DrawNewYearGreeting();

    // Инициализация областей исключения после рисования всех элементов
    // Область логотипа
    exclusionAreas[0].x = logoX;
    exclusionAreas[0].y = logoY;
    exclusionAreas[0].width = LOGO_WIDTH;
    exclusionAreas[0].height = LOGO_HEIGHT;

    // Область надписи "APPLINER"
    exclusionAreas[1].x = startX;
    exclusionAreas[1].y = lettersY;
    exclusionAreas[1].width = letters_width;
    exclusionAreas[1].height = LETTER_HEIGHT;

    // Область поздравления "С новым годом!!!"
    // Определяем размеры поздравления
    int total_width_greeting = 0;
    int max_element_height = 0;
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        total_width_greeting += element_widths[i];
        if (element_heights[i] > max_element_height) {
            max_element_height = element_heights[i];
        }
        if (i < NUM_ELEMENTS - 1) {
            total_width_greeting += spacing;
        }
    }
    int greetingX = (SCREEN_WIDTH - total_width_greeting) / 2;
    int greetingY = lettersY + LETTER_HEIGHT + 10;

    exclusionAreas[2].x = greetingX;
    exclusionAreas[2].y = greetingY;
    exclusionAreas[2].width = total_width_greeting;
    exclusionAreas[2].height = max_element_height;

    // Инициализация предыдущего состояния кнопки текущим состоянием пина
    buttonPrevRight = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4); // Инициализируем предыдущее состояние кнопки
    // Инициализация переменной для отслеживания времени обновления звёзд
    uint32_t lastStarUpdate = HAL_GetTick();
    
    // Вход в цикл ожидания нажатия кнопки
    while (1)
    {
        uint32_t currentTime = HAL_GetTick();

        // Проверяем, прошло ли 10 секунд с последнего обновления звёзд
        if ((currentTime - lastStarUpdate) >= STAR_UPDATE_INTERVAL) {
            UpdateStarPositions();
            lastStarUpdate = currentTime;
        }

        // Проверяем нажатие правой кнопки для выхода из стартового экрана и начала игры
        if (IsButtonPressed(GPIOA, GPIO_PIN_4, &buttonPrevRight))
        {
            lastButtonPressTime = HAL_GetTick(); // Фиксируем время нажатия
            break;
        }

        // Ограничение частоты обновлений
        HAL_Delay(100); // Задержка 100 мс
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

    // Определение параметров для вертикального размещения
    int num_vertical_images = 4; // gameover2, gameover3, gameover4, gameover5
    int startY = (SCREEN_HEIGHT - (gameoverHeights[1] + gameoverHeights[2] + gameoverHeights[3] + gameoverHeights[4] + 3 * 10)) / 2; // Центрирование вертикальной группы с отступами
    int spacing = 10; // Отступ между изображениями в пикселях

    // Отображение gameover2, gameover3, gameover4, gameover5 вертикально
    for(int i = 1; i <= 4; i++) // Индексы 1 до 4 включительно
    {
        x = (SCREEN_WIDTH - gameoverWidths[i]) / 2;
        ILI9341_Draw_Image(gameoverImages[i], x, startY, gameoverWidths[i], gameoverHeights[i], gameoverSizes[i]);
        startY += gameoverHeights[i] + spacing;
    }

    HAL_Delay(3000); // Задержка после отображения всех изображений
    
    // Заливка экрана черным перед отображением gameover6 или другими действиями
    ILI9341_Fill_Screen(BLACK);

    // Отображение gameover6
    int x1 = (SCREEN_WIDTH - gameoverWidths[5]) / 2;
    int y1 = (SCREEN_HEIGHT - gameoverHeights[5]) / 2;

    ILI9341_Draw_Image(gameoverImages[5], x1, y1, gameoverWidths[5], gameoverHeights[5], gameoverSizes[5]);
    HAL_Delay(1000); // Задержка в 1 секунду

    ILI9341_Fill_Screen(BLACK);


    InitializeStars();
    DrawStarsGradually();

    // Расчет позиций для логотипа и надписи
    int spacing_Y = 10; // Расстояние между логотипом и надписью
    int total_height = LOGO_HEIGHT + spacing_Y + LETTER_HEIGHT; // Общая высота группы (логотип + расстояние + надпись)
    int startY_logo = (SCREEN_HEIGHT - total_height) / 3; // Начальная позиция по вертикали для логотипа

    // Отображение логотипа по центру и чуть выше надписи
    logoX = (SCREEN_WIDTH - LOGO_WIDTH) / 2;
    logoY = startY_logo;

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
    lettersY = startY_logo + LOGO_HEIGHT + spacing_Y;

    // Отображение логотипа
    ILI9341_Draw_Image(img_logo, logoX, logoY, LOGO_WIDTH, LOGO_HEIGHT, sizeof(img_logo));

    // Отображение надписи "APPLINER" с задержкой между буквами
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
        ILI9341_Draw_Image((const unsigned char*)letter_image, x, y, LETTER_WIDTH, LETTER_HEIGHT, letter_size);

        HAL_Delay(200); // Задержка перед отображением следующей буквы
    }

    // Отображение поздравления "С новым годом!!!"
    DrawNewYearGreeting();

    // Инициализация областей исключения после рисования всех элементов
    // Область логотипа
    exclusionAreas[0].x = logoX;
    exclusionAreas[0].y = logoY;
    exclusionAreas[0].width = LOGO_WIDTH;
    exclusionAreas[0].height = LOGO_HEIGHT;

    // Область надписи "APPLINER"
    exclusionAreas[1].x = startX;
    exclusionAreas[1].y = lettersY;
    exclusionAreas[1].width = letters_width;
    exclusionAreas[1].height = LETTER_HEIGHT;

    // Область поздравления "С новым годом!!!"
    // Определяем размеры поздравления
    int total_width_greeting = 0;
    int max_element_height = 0;
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        total_width_greeting += element_widths[i];
        if (element_heights[i] > max_element_height) {
            max_element_height = element_heights[i];
        }
        if (i < NUM_ELEMENTS - 1) {
            total_width_greeting += spacing;
        }
    }
    int greetingX = (SCREEN_WIDTH - total_width_greeting) / 2;
    int greetingY = lettersY + LETTER_HEIGHT + 10;

    exclusionAreas[2].x = greetingX;
    exclusionAreas[2].y = greetingY;
    exclusionAreas[2].width = total_width_greeting;
    exclusionAreas[2].height = max_element_height;

    uint32_t lastStarUpdate = HAL_GetTick();

    // Ожидание нажатия правой кнопки для перезапуска игры
    buttonPrevRight = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4); // Инициализируем предыдущее состояние кнопки

   // Вход в цикл ожидания нажатия кнопки
    while (1)
    {
        uint32_t currentTime = HAL_GetTick();

        // Проверяем, прошло ли 10 секунд с последнего обновления звёзд
        if ((currentTime - lastStarUpdate) >= STAR_UPDATE_INTERVAL) {
            UpdateStarPositions();
            lastStarUpdate = currentTime;
        }

        // Проверяем нажатие правой кнопки для выхода из стартового экрана и начала игры
        if (IsButtonPressed(GPIOA, GPIO_PIN_4, &buttonPrevRight))
        {
            lastButtonPressTime = HAL_GetTick(); // Фиксируем время нажатия
            break;
        }

        // Ограничение частоты обновлений
        HAL_Delay(100); // Задержка 100 мс
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
