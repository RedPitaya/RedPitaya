#include "trigger_commands.h"

using namespace streaming_lib;

auto streaming_lib::triggerStatusOf(uio_lib::CHardware& hw) -> TriggerStatus {
    TriggerStatus st;
    auto f = hw.features();
    if (f.adc) {
        auto a = hw.adcStatus();
        st.adcArmed = a.armed;
        st.adcTriggerCount = a.triggerCount;
    }
    if (f.dac) {
        auto d = hw.dacStatus();
        st.dacArmed = (d.armed[0] ? 0x1 : 0) | (d.armed[1] ? 0x2 : 0);
    }
    if (f.gpio) {
        auto g = hw.gpioStatus();
        st.gpioInArmed = g.inputArmed;
        st.gpioOutArmed = g.outputArmed;
        st.gpioInTriggerCount = g.triggerCount;
        st.gpioOutWait = g.outputWait;
    }
    return st;
}

auto streaming_lib::connectTriggerCommands(std::shared_ptr<ServerNetConfigManager> config, uio_lib::CHardware::Ptr hw) -> void {
    if (!config)
        return;
    std::weak_ptr<ServerNetConfigManager> wc = config;
    std::weak_ptr<uio_lib::CHardware> wh = hw;

    config->softwareTriggerNofiy.connect([wc, wh](CNetConfigManager::ETriggerTarget target) {
        auto c = wc.lock();
        if (!c)
            return;
        auto h = wh.lock();
        bool done = false;
        if (h) {
            auto f = h->features();
            switch (target) {
                case CNetConfigManager::TT_ADC:
                    if ((done = f.adc))
                        h->adcSoftwareTrigger();
                    break;
                case CNetConfigManager::TT_DAC:
                    if ((done = f.dac))
                        h->dacSoftwareTrigger();
                    break;
                case CNetConfigManager::TT_GPIO:
                    if ((done = f.gpio))
                        h->gpioSoftwareTrigger();
                    break;
            }
        }
        if (done)
            c->sendSoftwareTriggerDone(target);
        else
            c->sendSoftwareTriggerFail(target);
    });

    config->triggerStatusRequestNofiy.connect([wc, wh]() {
        auto c = wc.lock();
        if (!c)
            return;
        auto h = wh.lock();
        c->sendTriggerStatus(h ? triggerStatusOf(*h) : TriggerStatus());
    });
}
