#include "C2M_ADC.h"
#include "C2M_gpio.h"
#include "DMAChannel.h"
#include <algorithm>

namespace C2M {

/*static*/ ::ADC ADC::adc_;
/*static*/ ADC::CalibrationData *ADC::calibration_data_;
/*static*/ uint32_t ADC::raw_[ADC_CHANNEL_LAST];
/*static*/ uint32_t ADC::smoothed_[ADC_CHANNEL_LAST];
/*static*/ volatile bool ADC::ready_;

constexpr uint16_t ADC::SCA_CHANNEL_ID[DMA_NUM_CH]; // ADCx_SCA register channel numbers
DMAChannel* dma0 = new DMAChannel(false); // dma0 channel, fills adcbuffer_0
DMAChannel* dma1 = new DMAChannel(false); // dma1 channel, updates ADC0_SC1A which holds the channel/pin IDs
DMAMEM static volatile uint16_t __attribute__((aligned(DMA_BUF_SIZE+0))) adcbuffer_0[DMA_BUF_SIZE];
  
/*static*/ void ADC::Init(CalibrationData *calibration_data) {

  adc_.setReference(ADC_REF_3V3);
  adc_.setResolution(kAdcScanResolution);
  adc_.setConversionSpeed(kAdcConversionSpeed);
  adc_.setSamplingSpeed(kAdcSamplingSpeed);
  adc_.setAveraging(kAdcScanAverages);
  adc_.enableDMA();
  
  calibration_data_ = calibration_data;
  std::fill(raw_, raw_ + ADC_CHANNEL_LAST, 0);
  std::fill(smoothed_, smoothed_ + ADC_CHANNEL_LAST, 0);
}

void ADC::DMA_ISR() {

  ADC::ready_ = true;
  dma0->TCD->DADDR = &adcbuffer_0[0];
  dma0->clearInterrupt();
  // move enable to SCAN_DMA() ?
  dma0->enable();
}

/*
 * 
 * DMA/ADC à la https://forum.pjrc.com/threads/30171-Reconfigure-ADC-via-a-DMA-transfer-to-allow-multiple-Channel-Acquisition
 * basically, this sets up two DMA channels and cycles through the 10 (effectively: 16) adc channels once (until the buffer is full), resets, and so on; dma1 advances SCA_CHANNEL_ID
 * somewhat like https://www.nxp.com/docs/en/application-note/AN4590.pdf but w/o the PDB.
 * 
*/

void ADC::Init_DMA() {
  
  dma0->begin(true); // allocate the DMA channel 
  dma0->TCD->SADDR = &ADC0_RA; 
  dma0->TCD->SOFF = 0;
  dma0->TCD->ATTR = 0x101;
  dma0->TCD->NBYTES = 2;
  dma0->TCD->SLAST = 0;
  dma0->TCD->DADDR = &adcbuffer_0[0];
  dma0->TCD->DOFF = 2; 
  dma0->TCD->DLASTSGA = -(2 * DMA_BUF_SIZE);
  dma0->TCD->BITER = DMA_BUF_SIZE;
  dma0->TCD->CITER = DMA_BUF_SIZE; 
  dma0->triggerAtHardwareEvent(DMAMUX_SOURCE_ADC0);
  dma0->disableOnCompletion();
  dma0->interruptAtCompletion();
  dma0->attachInterrupt(DMA_ISR);

  dma1->begin(true); // allocate the DMA channel 
  dma1->TCD->SADDR = &ADC::SCA_CHANNEL_ID[0];
  dma1->TCD->SOFF = 2; // source increment each transfer (n bytes)
  dma1->TCD->ATTR = 0x101;
  dma1->TCD->SLAST = - DMA_NUM_CH*2; // num ADC0 samples * 2
  dma1->TCD->BITER = DMA_NUM_CH;
  dma1->TCD->CITER = DMA_NUM_CH;
  dma1->TCD->DADDR = &ADC0_SC1A;
  dma1->TCD->DLASTSGA = 0;
  dma1->TCD->NBYTES = 2;
  dma1->TCD->DOFF = 0;
  dma1->triggerAtTransfersOf(*dma0);
  dma1->triggerAtCompletionOf(*dma0);

  dma0->enable();
  dma1->enable();
} 

/*static*/void FASTRUN ADC::Scan_DMA() {

  if (ADC::ready_) 
  {  
    ADC::ready_ = false;
    /* 
     *  starts collecting results at adcbuffer_0[1] because of weird offset (--> discard adcbuffer_0[0]). 
     *  there's 16 samples in the buffer, 1-5 / 11-15 are the pitch inputs: 
    */
    update<ADC_CHANNEL_1_1>((adcbuffer_0[1] + adcbuffer_0[11])/2); 
    update<ADC_CHANNEL_2_1>((adcbuffer_0[2] + adcbuffer_0[12])/2); 
    update<ADC_CHANNEL_3_1>((adcbuffer_0[3] + adcbuffer_0[13])/2); 
    update<ADC_CHANNEL_4_1>((adcbuffer_0[4] + adcbuffer_0[14])/2); 
    update<ADC_CHANNEL_5_1>((adcbuffer_0[5] + adcbuffer_0[15])/2);
    /*  6-10 are the velocity inputs: */
    update<ADC_CHANNEL_1_2>(adcbuffer_0[6]);
    update<ADC_CHANNEL_2_2>(adcbuffer_0[7]);
    update<ADC_CHANNEL_3_2>(adcbuffer_0[8]);
    update<ADC_CHANNEL_4_2>(adcbuffer_0[9]);
    update<ADC_CHANNEL_5_2>(adcbuffer_0[10]);
  }
}

/*static*/ void ADC::CalibratePitch(int32_t c2, int32_t c4) {
  // This is the method used by the Mutable Instruments calibration and
  // extrapolates from two octaves. 
  if (c2 < c4) {
    int32_t scale = (24 * 128 * 4096L) / (c4 - c2);
    calibration_data_->pitch_cv_scale = scale;
  }
}

}; // namespace C2M
