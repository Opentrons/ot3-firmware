#include "common/firmware/can.h"




HAL_StatusTypeDef MX_FDCAN1_Init(FDCAN_HandleTypeDef * handle)
{
    handle->Instance = FDCAN1;
    handle->Init.ClockDivider = FDCAN_CLOCK_DIV1;
    handle->Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    handle->Init.Mode = FDCAN_MODE_NORMAL;
    handle->Init.AutoRetransmission = DISABLE;
    handle->Init.TransmitPause = DISABLE;
    handle->Init.ProtocolException = DISABLE;
    handle->Init.NominalPrescaler = 20;
    handle->Init.NominalSyncJumpWidth = 2;
    handle->Init.NominalTimeSeg1 = 14;
    handle->Init.NominalTimeSeg2 = 2;
    handle->Init.DataPrescaler = 20;
    handle->Init.DataSyncJumpWidth = 1;
    handle->Init.DataTimeSeg1 = 14;
    handle->Init.DataTimeSeg2 = 1;
    handle->Init.StdFiltersNbr = 0;
    handle->Init.ExtFiltersNbr = 0;
    handle->Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
    return HAL_FDCAN_Init(handle);
}

