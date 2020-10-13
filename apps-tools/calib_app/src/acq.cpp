#include <iostream>
#include <chrono>
#include "acq.h"


inline int32_t rawToInt32(uint32_t cnts)
{
    int32_t m;
    /* check sign */
    if(cnts & (1 << (ADC_BITS - 1))) {
        /* negative number */
        m = -1 *((cnts ^ ((1 << ADC_BITS) - 1)) + 1);
    } else {
        /* positive number */
        m = cnts;
    }
    return m;
}

COscilloscope::Ptr COscilloscope::Create(uint32_t _decimation)
{
    return std::make_shared<COscilloscope>(_decimation);
}

COscilloscope::COscilloscope(uint32_t _decimation):
m_decimation(_decimation),
m_index(0)
{
   
    
}

COscilloscope::~COscilloscope()
{
    stop();
}


void COscilloscope::stop(){
    m_OscThreadRun.clear();
    if (m_OscThread.joinable()){
        m_OscThread.join();
    }
}

void COscilloscope::start(){
    try {
        m_index = 0;
        m_OscThreadRun.test_and_set();
        m_OscThread = std::thread(&COscilloscope::oscWorker, this);        
    }
    catch (std::exception& e)
    {
        std::cerr << "Error: COscilloscope::start(), " << e.what() << std::endl;
    }
}


void COscilloscope::oscWorker(){
    try{
        
        while (m_OscThreadRun.test_and_set())
        {
            acquire();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }    
    }catch (std::exception& e)
    {
        std::cerr << "Error: oscWorker() -> %s\n" << e.what() << std::endl ;
    }
}


void COscilloscope::acquire(){
    uint32_t pos = 0;
    int16_t             timeout = 10000;
    bool                fillState = false;
    uint32_t            acq_u_size = ADC_BUFFER_SIZE;
    uint32_t            acq_u_size_raw = ADC_BUFFER_SIZE;
    rp_acq_trig_state_t trig_state = RP_TRIG_STATE_TRIGGERED;

    rp_AcqSetDecimationFactor(m_decimation);
    rp_AcqSetTriggerDelay( ADC_BUFFER_SIZE / 2.0 );
    rp_AcqStart();
    rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);

    for (;timeout > 0;) {
        rp_AcqGetTriggerState(&trig_state);

        if (trig_state == RP_TRIG_STATE_TRIGGERED) {
            break;
        } else {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            timeout--;
        }
    }

    while(!fillState && (timeout > 0)){
        rp_AcqGetBufferFillState(&fillState);
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        timeout--;
    }
    rp_AcqStop();
    rp_AcqGetWritePointer(&pos);
    rp_AcqGetDataV2(pos, &acq_u_size, m_buffer[0], m_buffer[1]);
    rp_AcqGetDataRawV2(pos, &acq_u_size_raw, m_buffer_raw[0],m_buffer_raw[1]);
    if (acq_u_size > 0) {
        DataPass localDP;
        localDP.index = m_index++;
        localDP.ch1_avg = 0;
        localDP.ch2_avg = 0;
        localDP.ch1_max = m_buffer[0][0];
        localDP.ch1_min = m_buffer[0][0];
        localDP.ch2_max = m_buffer[1][0];
        localDP.ch2_min = m_buffer[1][0];
        localDP.ch1_avg_raw = 0;
        localDP.ch2_avg_raw = 0;
        localDP.ch1_min_raw = rawToInt32(m_buffer_raw[0][0]);
        localDP.ch1_max_raw = rawToInt32(m_buffer_raw[0][0]);
        localDP.ch2_min_raw = rawToInt32(m_buffer_raw[1][0]);
        localDP.ch2_max_raw = rawToInt32(m_buffer_raw[1][0]);

        for(auto i = 0 ; i < acq_u_size ; ++i){
            if (localDP.ch1_max < m_buffer[0][i]) localDP.ch1_max = m_buffer[0][i];
            if (localDP.ch2_max < m_buffer[1][i]) localDP.ch2_max = m_buffer[1][i];
            if (localDP.ch1_min > m_buffer[0][i]) localDP.ch1_min = m_buffer[0][i];
            if (localDP.ch2_min > m_buffer[1][i]) localDP.ch2_min = m_buffer[1][i];
            localDP.ch1_avg += m_buffer[0][i];
            localDP.ch2_avg += m_buffer[1][i];
        }
        localDP.ch1_avg /= acq_u_size;
        localDP.ch2_avg /= acq_u_size;

        for(auto i = 0 ; i < acq_u_size_raw; ++i){
            auto ch1 = rawToInt32(m_buffer_raw[0][i]);
            auto ch2 = rawToInt32(m_buffer_raw[1][i]);
            
            if (localDP.ch1_max_raw < ch1) localDP.ch1_max_raw = ch1;
            if (localDP.ch2_max_raw < ch2) localDP.ch2_max_raw = ch2; 
            if (localDP.ch1_min_raw > ch1) localDP.ch1_min_raw = ch1;
            if (localDP.ch2_min_raw > ch2) localDP.ch2_min_raw = ch2;
            localDP.ch1_avg_raw += ch1;
            localDP.ch2_avg_raw += ch2;
        }
        localDP.ch1_avg_raw /= acq_u_size_raw;
        localDP.ch2_avg_raw /= acq_u_size_raw;

        pthread_mutex_lock(&m_mutex);
        m_crossData = localDP;
        pthread_mutex_unlock(&m_mutex);
    }
}

COscilloscope::DataPass COscilloscope::getData(){
    DataPass local_pass;
    pthread_mutex_lock(&m_mutex);
    local_pass = m_crossData;
    pthread_mutex_unlock(&m_mutex); 
    return local_pass;
}