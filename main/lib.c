#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"


/*
 * 2초 동안 키 입력 확인
 * Enter 입력 시 true 반환
 * 입력이 없으면 false 반환
 */
bool wait_for_enter(uint32_t timeout_ms)
{
    uint32_t elapsed = 0;

    while (elapsed < timeout_ms) {

        int ch = getchar();

        if (ch != EOF) {
            if (ch == '\r' || ch == '\n') {
                return true;
            }
        } else {
            /* STDIN EOF 상태 초기화 */
            clearerr(stdin);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
        elapsed += 10;
    }

    return false;
}