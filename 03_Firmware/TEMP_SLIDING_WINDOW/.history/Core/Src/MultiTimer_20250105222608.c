#include "MultiTimer.h"
#include <stdio.h>
#include "main.h"
#include "tim.h"
#include "DS18B20.h"
#include "usart.h"
#include "string.h"

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
    for(short i =0;i<7*BUFFER_SIZE;i=i+7)
    {
        tx_buf[i] = 0xaa;
    }
    for(short i =6;i<7*BUFFER_SIZE;i=i+7)
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

uint32_t times =0;
float slide_windows_fliter(float value)
{
    float output = 0;
    times++;
    printf("slide times:%d,buffer_cur:%d,window_cur%d\r\n",times);
    if(temps.buffer_cur < WINDOW_SIZE)
    {
        temps.window_buffer[temps.window_cur++] = value; //先把窗口填满
        output = value;
    }else{
        // float sum = 0;
        memcpy(&temps.window_buffer[0],&temps.window_buffer[1],(WINDOW_SIZE - 1)*sizeof(float));
        temps.window_buffer[WINDOW_SIZE - 1] = value;
        for(uint8_t i = 0;i<WINDOW_SIZE;i++)
        {
            output+= temps.window_buffer[i];
        }
        output = output / WINDOW_SIZE;
        if(output>=temps.warning_temp)
        {
            trigger_warn(1);
        }else{
            trigger_warn(0);
        }
         OLED_Showdecimal(40,3,output,2,2,16,0);
        // printf("temp:%.2f\r\n",output);
    }
    return output;
}


// uint8_t printf_buf = 0;
extern DMA_HandleTypeDef hdma_usart1_tx;

void get_tempbuffer(void)
{
    float temp = DS18B20_Get_Temp();
    temps.buffer[temps.buffer_cur++] = slide_windows_fliter(temp);
    if(temps.buffer_cur>=BUFFER_SIZE)
    {
        temps.buffer_cur = 0;
        printf("buffer_size done\r\n");
        // float_to_bytes_update();
        // huart1.gState = HAL_UART_STATE_READY;
        // hdma_usart1_tx.State = HAL_DMA_STATE_READY;
        // __HAL_UNLOCK(&hdma_usart1_tx);
        // HAL_UART_Transmit_DMA(&huart1,tx_buf,BUFFER_SIZE*7);
    }
}


MultiTimer g_timer_get_temp;
void Timer_gettempCallback(MultiTimer* Timer,void* userdata)
{

    get_tempbuffer();
    multiTimerStart(&g_timer_get_temp,20,Timer_gettempCallback,NULL);//
}

/*gettemp*/


void timers_init(void)
{
  HAL_TIM_Base_Start_IT(&htim1);
  multiTimerInstall(timer1_tick_get);
  multiTimerStart(&g_timer_get_temp,20,Timer_gettempCallback,NULL);
}
