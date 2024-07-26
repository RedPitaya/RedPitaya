#pragma once


#include <DataManager.h>
#include <CustomParameters.h>
#include <sys/syslog.h> //Add custom RP_LCR LOG system

#include "lcrApp.h"
#include "rp.h"


#define IF_VALUE_CHANGED(X, ACTION) \
if (X.Value() != X.NewValue()) { \
    int res = ACTION;\
    if (res == RP_OK) { \
        X.Update(); \
    } \
}

#define IF_VALUE_CHANGED_BOOL(X, ACTION1, ACTION2) \
if (X.Value() != X.NewValue()) { \
    if (X.NewValue()) { \
        ACTION1;    X.Update(); \
    } else { \
        ACTION2;    X.Update(); }}

#define IS_NEW(X) X.Value() != X.NewValue()


#ifdef __cplusplus
extern "C" {
#endif

/* Parameters description structure - must be the same for all RP controllers */
typedef struct rp_app_params_s {
    char  *name;
    float  value;
    int    fpga_update;
    int    read_only;
    float  min_val;
    float  max_val;
} rp_app_params_t;

/* Signal measurement results structure - filled in worker and updated when
 * also measurement signal is stored from worker
 */
typedef struct rp_osc_meas_res_s {
    float min;
    float max;
    float amp;
    float avg;
    float freq;
    float period;
} rp_osc_meas_res_t;

const char *rp_app_desc(void);
int rp_app_init(void);
int rp_app_exit(void);
int rp_set_params(rp_app_params_t *p, int len);
int rp_get_params(rp_app_params_t **p);
int rp_get_signals(float ***s, int *sig_num, int *sig_len);

class CStatistics
{
public:
    CStatistics(){}
    void initialize(int buffSize, double minAllowable, double maxAllowable)
    {
        m_writePosition = 0;
        m_buffSize = buffSize;
        m_buffer = new double[m_buffSize];
        m_minAllowable = minAllowable;
        m_maxAllowable = maxAllowable;
        clear();
    }
    CStatistics(int buffSize, double minAllowable, double maxAllowable)
    {
        initialize(buffSize, minAllowable, maxAllowable);
    }
    ~CStatistics() { delete m_buffer; }

    void add(double element)
    {
        if((element >= m_minAllowable) && (element <= m_maxAllowable)) {
            if(element > m_max) m_max = element;
            if(element < m_min) m_min = element;

            m_buffer[m_writePosition] = element;
            if(++m_writePosition >= m_buffSize)
                m_writePosition = 0;

            if(m_nActualVal < m_buffSize)
                ++m_nActualVal;
        }
    }

    double sum()
    {
        double sum = 0;
        for(int i = 0; i < m_buffSize; i++)
            sum = sum + m_buffer[i];

        return sum;
    }

    double avg()
    {
        if(m_nActualVal != 0) {
            return sum() / m_nActualVal;
        } else {
            return 0;
        }
    }

    double min() { return m_min; }

    double max() { return m_max; }

    void clear()
    {
        m_max = m_minAllowable;
        m_min = m_maxAllowable;
        m_nActualVal = 0;
        for(int i = 0; i < m_buffSize; i++)
            m_buffer[i] = 0;
    }

    double getElement(int pos)
    {
        /* Numeration of internal array:
         * [(...)(3)(2)(1)(0)(m_WritePosition | m_BuffSize-1)(m_BuffSize-2)(...)]
         * pos shold be from 0 to (m_BuffSize-1). Else it be forth convert to this interval */
        if(pos < 0) pos = 0;
        else if(pos >= m_buffSize) pos = m_buffSize-1;

        pos = m_writePosition - 1 - pos;
        if(pos < 0) pos += m_buffSize;

        return m_buffer[pos];
    }

private:
    int     m_buffSize;
    int     m_nActualVal; // for more correct avg
    double *m_buffer;
    double  m_max; // since initialization
    double  m_min; // since initialization
    double  m_maxAllowable;
    double  m_minAllowable;

    int     m_writePosition;
};

#ifdef __cplusplus
}
#endif
