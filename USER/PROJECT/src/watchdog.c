#include "watchdog.h"

void watchdog_init(u16 time_ms)
{
    RCC_LSICmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_256);
    IWDG_SetReload((time_ms * 32) / 256);
    IWDG_ReloadCounter();
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
    IWDG_Enable();
}

void watchdog_reload(void)
{
    IWDG_ReloadCounter();
}
