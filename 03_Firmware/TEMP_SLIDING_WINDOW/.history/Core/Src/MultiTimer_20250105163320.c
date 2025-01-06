#include "MultiTimer.h"
#include <stdio.h>
#include "main.h"
#include "tim.h"
#include "DS18B20.h"
#include "usart.h"

static MultiTimer* timerList = NULL;
static PlatformTicksFunction_t platformTicksFunction = NULL;

int multiTimerInstall(PlatformTicksFunction_t ticksFunc) {
    if (ticksFunc == NULL) {
        return -1; // Indicate error if ticksFunc is NULL
    }
    platformTicksFunction = ticksFunc;
    return 0;
}

static void removeTimer(MultiTimer* timer) {
    MultiTimer** current = &timerList;
    while (*current) {
        if (*current == timer) {
            *current = timer->next;
            break;
        }
        current = &(*current)->next;
    }
}

int multiTimerStart(MultiTimer* timer, uint64_t timing, MultiTimerCallback_t callback, void* userData) {
    if (!timer || !callback || platformTicksFunction == NULL) {
        return -1; // Return error if any parameter is invalid
    }

    removeTimer(timer); // Centralize removal logic

    timer->deadline = platformTicksFunction() + timing;
    timer->callback = callback;
    timer->userData = userData;

    MultiTimer** current = &timerList;
    while (*current && ((*current)->deadline < timer->deadline)) {
        current = &(*current)->next;
    }
    timer->next = *current;
    *current = timer;

    return 0;
}

int multiTimerStop(MultiTimer* timer) {
    removeTimer(timer); // Use centralized removal function
    return 0;
}

int multiTimerYield(void) {
    if (platformTicksFunction == NULL) {
        return -1; // Indicate error if platformTicksFunction is NULL
    }
    uint64_t currentTicks = platformTicksFunction();
    while (timerList && (currentTicks >= timerList->deadline)) {
        MultiTimer* timer = timerList;
        timerList = timer->next; // Remove expired timer

        if (timer->callback) {
            timer->callback(timer, timer->userData); // Execute callback
        }
    }
    return timerList ? (int)(timerList->deadline - currentTicks) : 0;
}

static uint64_t g_timer1_tick;
uint64_t timer1_tick_get(void)
{
    return g_timer1_tick;
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == (&htim1))
    {
        g_timer1_tick++;
    }
}


/*timerbuzzer*/
MultiTimer g_timer_buzzer;
uint8_t g_buzzer_onoff = 0;

void Timer_buzzerCallback(MultiTimer* Timer,void* userdata)
{
    if(g_buzzer_onoff == 1)
    {
        HAL_GPIO_TogglePin(buzzer_GPIO_Port,buzzer_Pin);
        multiTimerStart(&g_timer_buzzer,2,Timer_buzzerCallback,NULL);
    }
}

void buzzer_onoff(uint8_t onoff)
{
    if(onoff == 1)
    {
        g_buzzer_onoff  = 1;
        multiTimerStart(&g_timer_buzzer,2,Timer_buzzerCallback,NULL);
    }else{
        multiTimerStop(&g_timer_buzzer);
    }    
}
/*timerbuzzer*/


/*gettemp*/
uint8_t tx_buf[7*BUFFER_SIZE];
void tx_buf_init(void)
{
    for(uint8_t i =0;i<140;i=i+7)
    {
        tx_buf[i] = 0xaa;
    }
    for(uint8_t i =6;i<140;i=i+7)
    {
        tx_buf[i] = 0xbb;
    }
}

uint8_t crc8_calculate(uint8_t* buf,uint8_t len)
{
    uint8_t crc8 = 0;
    for(uint8_t i=0;i<len;i++)
    {
        crc8+=buf[i];
    }
    return crc8;
}



void float_to_bytes_update(void)
{
    for(uint8_t i =0;i<BUFFER_SIZE;i++)
    {
        memcpy(&tx_buf[1+i*7],&temps.buffer[i],sizeof(float));
        tx_buf[5+i*7] = crc8_calculate(&tx_buf[0+i*7],5);
    }

}


void trigger_warn(uint8_t onoff)
{
    if(onoff)
    {
        buzzer_onoff(1);
        led_control(2,1);
    }else{
        buzzer_onoff(0);
        led_control(2,0);
    }
}


float slide_windows_get(uint8_t start_num)
{
    float slide_result = 0;
    for(uint8_t i = 0;i<OUTPUT_SIZE;i++)
    {
        for(uint)
    }
    // float slide_result = 0;
    // for(uint8_t i =0;i<SLIDING_WINDOW;i++)
    // {
    //     slide_result += temps.buffer[start_num+i];
    // }
    
    // return slide_result / SLIDING_WINDOW;
}


float slide_windows_add(void)
{
    float sum = 0;
    for(uint8_t i =0;i<WINDOW_SIZE;i++)
    {
        sum+=temps.temps_buffer[i];
    }
    sum = sum / WINDOW_SIZE;
    if(sum>=temps.warning_temp)
    {
       trigger_warn(1);
    }else{
        trigger_warn(0);
    }
    // printf("temps:%.2f\r\n",sum);
    OLED_Showdecimal(40,3,sum,2,2,16,0);
    return sum;
}

// uint8_t printf_buf = 0;
extern DMA_HandleTypeDef hdma_usart1_tx;
void get_buffer(void)
{
    temps.buffer[temps.buffer_cur++] = slide_windows_add();
    if(temps.buffer_cur>=BUFFER_SIZE)
    {
        temps.buffer_cur = 0;
        float_to_bytes_update();
        huart1.gState = HAL_UART_STATE_READY;
        hdma_usart1_tx.State = HAL_DMA_STATE_READY;
        __HAL_UNLOCK(&hdma_usart1_tx);
        HAL_UART_Transmit_DMA(&huart1,tx_buf,140);
    }
}


void get_tempbuffer(float temp)
{
   temps.temps_buffer[temps.temps_cur++] = temp;
   if(temps.temps_cur>=WINDOW_SIZE)
   {
    get_buffer();
    temps.temps_cur = 0;
   }
}


MultiTimer g_timer_get_temp;
void Timer_gettempCallback(MultiTimer* Timer,void* userdata)
{
    float temp;
    temp = DS18B20_Get_Temp();
    // printf("%.02f\r\n",temp);
    get_tempbuffer(temp);
    multiTimerStart(&g_timer_get_temp,5,Timer_gettempCallback,NULL);//
}

/*gettemp*/


void timers_init(void)
{
  HAL_TIM_Base_Start_IT(&htim1);
  multiTimerInstall(timer1_tick_get);
  multiTimerStart(&g_timer_get_temp,20,Timer_gettempCallback,NULL);
}
