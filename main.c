#include "stm32f4xx.h"

// ================= CONFIG =================
#define LONG_PRESS_TIME   1000   // ms
#define BLINK_PERIOD      300    // ms

#define BUTTON_PRESSED    0
#define BUTTON_RELEASED   1

// ================= TYPES =================
typedef enum {
    IDLE = 0,
    LEFT_ACTIVE,
    RIGHT_ACTIVE,
    HAZARD_ACTIVE
} state_t;

// ================= GLOBALS =================
volatile uint32_t sys_tick = 0;

state_t state = IDLE;

uint32_t left_press_time = 0;
uint32_t right_press_time = 0;

uint8_t left_event = 0;
uint8_t right_event = 0;
uint8_t both_event = 0;

uint32_t blink_timer = 0;
uint8_t led_state = 0;

// ================= SYSTICK =================
void SysTick_Handler(void)
{
    sys_tick++;
}

void delay_ms(uint32_t ms)
{
    uint32_t start = sys_tick;
    while ((sys_tick - start) < ms);
}

// ================= GPIO INIT =================
void GPIO_Init(void)
{
    // Enable GPIOA + GPIOB clock
    RCC->AHB1ENR |= (1 << 0) | (1 << 1);

    // PA0, PA1 as input (buttons)
    GPIOA->MODER &= ~((3 << (0*2)) | (3 << (1*2)));

    // Enable pull-up
    GPIOA->PUPDR |= (1 << (0*2)) | (1 << (1*2));

    // PB4 (TIM3_CH1), PB5 (TIM3_CH2) alternate function
    GPIOB->MODER |= (2 << (4*2)) | (2 << (5*2));

    // AF2 for TIM3
    GPIOB->AFR[0] |= (2 << (4*4)) | (2 << (5*4));
}

// ================= TIM3 PWM INIT =================
void TIM3_PWM_Init(void)
{
    RCC->APB1ENR |= (1 << 1);  // TIM3 clock

    TIM3->PSC = 16000 - 1;     // 16 MHz / 16000 = 1 kHz
    TIM3->ARR = 1000 - 1;      // 1 Hz base (for duty scaling)

    // PWM mode 1
    TIM3->CCMR1 |= (6 << 4) | (6 << 12);

    TIM3->CCER |= (1 << 0) | (1 << 4); // Enable CH1, CH2

    TIM3->CCR1 = 0;
    TIM3->CCR2 = 0;

    TIM3->CR1 |= (1 << 0); // Enable timer
}

// ================= BUTTON READ =================
uint8_t read_left_button(void)
{
    return (GPIOA->IDR & (1 << 0)) ? BUTTON_RELEASED : BUTTON_PRESSED;
}

uint8_t read_right_button(void)
{
    return (GPIOA->IDR & (1 << 1)) ? BUTTON_RELEASED : BUTTON_PRESSED;
}

// ================= BUTTON PROCESS =================
void process_buttons(void)
{
    uint8_t left = read_left_button();
    uint8_t right = read_right_button();

    // LEFT
    if (left == BUTTON_PRESSED)
        left_press_time += 100;
    else
        left_press_time = 0;

    // RIGHT
    if (right == BUTTON_PRESSED)
        right_press_time += 100;
    else
        right_press_time = 0;

    // Events
    left_event  = (left_press_time  >= LONG_PRESS_TIME);
    right_event = (right_press_time >= LONG_PRESS_TIME);
    both_event  = left_event && right_event;
}

// ================= STATE MACHINE =================
void handle_state(void)
{
    // Hazard activation
    if (both_event)
    {
        state = HAZARD_ACTIVE;
        left_press_time = 0;
        right_press_time = 0;
        return;
    }

    // Hazard deactivation
    if (state == HAZARD_ACTIVE)
    {
        if (left_event || right_event)
        {
            state = IDLE;
            left_press_time = 0;
            right_press_time = 0;
        }
        return;
    }

    // LEFT
    if (left_event)
    {
        if (state == LEFT_ACTIVE)
            state = IDLE;
        else
            state = LEFT_ACTIVE;

        left_press_time = 0;
        right_press_time = 0;
        return;
    }

    // RIGHT
    if (right_event)
    {
        if (state == RIGHT_ACTIVE)
            state = IDLE;
        else
            state = RIGHT_ACTIVE;

        left_press_time = 0;
        right_press_time = 0;
        return;
    }
}

// ================= LED UPDATE =================
void update_leds(void)
{
    uint16_t pwm = led_state ? 500 : 0;

    switch (state)
    {
        case LEFT_ACTIVE:
            TIM3->CCR1 = pwm;
            TIM3->CCR2 = 0;
            break;

        case RIGHT_ACTIVE:
            TIM3->CCR1 = 0;
            TIM3->CCR2 = pwm;
            break;

        case HAZARD_ACTIVE:
            TIM3->CCR1 = pwm;
            TIM3->CCR2 = pwm;
            break;

        default:
            TIM3->CCR1 = 0;
            TIM3->CCR2 = 0;
            break;
    }
}

// ================= MAIN =================
int main(void)
{
    SystemInit();

    SysTick_Config(SystemCoreClock / 1000); // 1 ms tick

    GPIO_Init();
    TIM3_PWM_Init();

    uint32_t last_tick = 0;

    while (1)
    {
        if ((sys_tick - last_tick) >= 100)
        {
            last_tick = sys_tick;

            process_buttons();
            handle_state();

            // Blink
            blink_timer += 100;
            if (blink_timer >= BLINK_PERIOD)
            {
                blink_timer = 0;
                led_state ^= 1;
            }

            update_leds();
        }
    }
}
