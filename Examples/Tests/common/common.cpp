#include "common.h"

auto getADCChannels() -> uint8_t{
    uint8_t c = 0;
    if (rp_HPGetFastADCChannelsCount(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast ADC channels count\n");
        exit(-1);
    }
    return c;
}

auto getDACChannels() -> uint8_t{
    uint8_t c = 0;
    if (rp_HPGetFastDACChannelsCount(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast DAC channels count\n");
        exit(-1);
    }
    return c;
}

auto getADCRate() -> uint32_t{
    uint32_t c = 0;
    if (rp_HPGetBaseFastADCSpeedHz(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast ADC rate\n");
        exit(-1);
    }
    return c;
}

auto getClock() -> double {
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return ((double)tp.tv_sec * 1000.f) + ((double)tp.tv_nsec / 1000000.f);
}

auto getModel() -> models_t{
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get board model\n");
        exit(-1);
    }

    switch (c)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_LN_BO_v1_1:
        case STEM_125_14_LN_CE1_v1_1:
        case STEM_125_14_LN_CE2_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
            return RP_125_14;

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return RP_122_16;

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
            return RP_125_14_4CH;

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_v1_2b:
            return RP_250_12;
        default:
            fprintf(stderr,"[Error] Can't get board model\n");
            exit(-1);
    }
    return RP_125_14;
}

auto getTrigName(rp_acq_trig_src_t tr) -> std::string{
    switch (tr)
    {
      case RP_TRIG_SRC_DISABLED: return "Disabled";
    case RP_TRIG_SRC_NOW: return "NOW";
    case RP_TRIG_SRC_CHA_PE: return "Ch1 PE";
    case RP_TRIG_SRC_CHA_NE: return "Ch1 NE";
    case RP_TRIG_SRC_CHB_PE: return "Ch2 PE";
    case RP_TRIG_SRC_CHB_NE: return "Ch2 NE";
    case RP_TRIG_SRC_EXT_PE: return "EXT PE";
    case RP_TRIG_SRC_EXT_NE: return "EXT NE";
    case RP_TRIG_SRC_AWG_PE: return "AWG PE";
    case RP_TRIG_SRC_AWG_NE: return "AWG NE";
    case RP_TRIG_SRC_CHC_PE: return "Ch3 PE";
    case RP_TRIG_SRC_CHC_NE: return "Ch3 NE";
    case RP_TRIG_SRC_CHD_PE: return "Ch4 PE";
    case RP_TRIG_SRC_CHD_NE: return "Ch4 NE";

    default:
        break;
    }
    return "ERROR";
}

auto getTrigNameDir(rp_acq_trig_src_t tr) -> float{
    switch (tr)
    {
      case RP_TRIG_SRC_DISABLED: return 1;
        case RP_TRIG_SRC_NOW: return 0;
        case RP_TRIG_SRC_CHA_PE: return 1;
        case RP_TRIG_SRC_CHA_NE: return -1;
        case RP_TRIG_SRC_CHB_PE: return 1;
        case RP_TRIG_SRC_CHB_NE: return -1;
        case RP_TRIG_SRC_EXT_PE: return 1;
        case RP_TRIG_SRC_EXT_NE: return -1;
        case RP_TRIG_SRC_AWG_PE: return 1;
        case RP_TRIG_SRC_AWG_NE: return -1;
        case RP_TRIG_SRC_CHC_PE: return 1;
        case RP_TRIG_SRC_CHC_NE: return -1;
        case RP_TRIG_SRC_CHD_PE: return 1;
        case RP_TRIG_SRC_CHD_NE: return -1;

    default:
        break;
    }
    return 0;
}

auto printBuffer(buffers_t *data,int indexOffset,int size) -> void {
    for(int ch = 0 ; ch < data->channels ; ch++){
        printf("Ch %d :",ch+1);
        for(int i = 0; i < size; i++){
            int pos = (data->size + i + indexOffset) % data->size;
            printf("[%d = %d]", i + indexOffset, data->ch_i[ch][pos]);
        }
        printf("\n");
    }
}

auto printBufferF(buffers_t *data,int indexOffset,int size) -> void {
    for(int ch = 0 ; ch < data->channels ; ch++){
        printf("Ch %d :",ch+1);
        for(int i = 0; i < size; i++){
            int pos = (data->size + i + indexOffset) % data->size;
            printf("[%d = %f]", i + indexOffset, data->ch_f[ch][pos]);
        }
        printf("\n");
    }
}

auto printTestResult(list<string>  &_result, string _testName,bool result) -> void {
    Color::Modifier red(Color::FG_RED);
    Color::Modifier green(Color::FG_GREEN);

    string ok = "[OK]";
    string fail = "[FAIL]";

    string res = string("Result of ") + _testName + " " + (result ?  green.color("[OK]") : red.color("[FAIL]"));
    std::cout << res << "\n";
    int allSize = 150;
    allSize -= _testName.size();
    allSize -= result ? ok.size() : fail.size();

    _result.push_back(_testName + string(allSize,'.') + (result ? green.color(ok) : red.color(fail)));
}

auto printAllResult(list<string>  &_result) -> void {
    for (auto const& i : _result) {
        std::cout << i << "\n";
    }
}


auto getDACGainCh1() -> float{
    float c = 0;
    if (rp_HPGetFastDACGain(RP_CH_1, &c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast DAC gain\n");
        exit(-1);
    }
    return c;
}
