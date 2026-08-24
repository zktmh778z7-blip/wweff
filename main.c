#include "stm32f10x.h"
#include "usart.h"
#include "delay.h"
#include "timer.h"
#include "port.h"
#include "mb.h"
#include "mb_m.h"
#include "user_mb_app.h"
#include "usart4.h"
USHORT   usMRegHoldBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_HOLDING_NREGS];
int main(void)
{
    // 1. 基础硬件及中断初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    systick_clock_config();
    USART1_Config(115200); // 调试串口，用于在电脑串口助手看数据
	printf("Hello World");
    UART4_Config(9600);    // 485串口连接E5CC温控表 (波特率9600, 偶校验)
    Timer_Init(0xFFFF, 72);// TIM2定时器初始化

    // 2. 初始化 usart4 主机协议栈
    if(eMBMasterInit(MB_RTU, 4, 9600, MB_PAR_NONE) != MB_ENOERR)
    {
        printf("usart4 主机初始化失败！\r\n");
        while(1);
    }

    // 3. 启动主机
    eMBMasterEnable();
    printf("usart4 主机已启动，开始与 E5CC 通信...\r\n");
	 printf("Hello\r\n");
    while(1)
    {
 // 4. 必须高频调用的状态机引擎
        eMBMasterPoll();

        // 5. 主动读取 1号从机(E5CC) 的保持寄存器 40001 (起始地址0，读取1个)
        // 注意：这里的 1000 就是超时时间，底层会自动在这里卡住等待，直到成功或超时！
        if(eMBMasterReqReadHoldingRegister(1, 0, 1, 1000) == MB_MRE_NO_ERR)
        {
            // 代码能走到这里，说明已经成功拿到了数据！
            float actual_temp = (float)usMRegHoldBuf[0][0] / 10.0;
            printf("【成功】E5CC 当前实时温度: %.1f ℃\r\n", actual_temp);
        }
        else
        {
            // 如果超时没回话，就会走到这里
            printf("【超时】从机未响应，请检查！\r\n");
        }

        Delay_nms(1000); // 间隔1秒再去读一次
    }
}
