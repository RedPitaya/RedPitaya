#ifndef TDMS_LIB_WAVEFORM_H
#define TDMS_LIB_WAVEFORM_H

#include <optional>
#include <string>

#include "file_reader.h"
#include "segment_builder.h"

namespace TDMS {

// The channel properties by which LabVIEW, DIAdem and npTDMS give the samples of a channel their
// time: sample i is at start + offset + i * increment. They hold for evenly spaced samples; after
// a gap in the recording (lost samples not filled) the times of the later samples are wrong.
namespace WaveformProperty {
constexpr const char* StartTime = "wf_start_time";      // TimeStamp of the first sample
constexpr const char* StartOffset = "wf_start_offset";  // double, seconds added to the start
constexpr const char* Increment = "wf_increment";       // double, seconds from one sample to the next
constexpr const char* Samples = "wf_samples";           // int32, samples per waveform (informative)
constexpr const char* Unit = "unit_string";             // string, the unit of the samples
}  // namespace WaveformProperty

struct Waveform {
    TimeStamp start;
    double increment = 0;  // seconds
    double offset = 0;     // seconds
};

// Writes the waveform properties of a channel into the segment
inline auto setWaveform(SegmentBuilder& segment, const std::string& group, const std::string& channel, const Waveform& waveform) -> void {
    segment.setChannelProperty(group, channel, WaveformProperty::StartTime, waveform.start);
    segment.setChannelProperty(group, channel, WaveformProperty::StartOffset, waveform.offset);
    segment.setChannelProperty(group, channel, WaveformProperty::Increment, waveform.increment);
}

// The waveform of a channel, when it has a start time and an increment
inline auto waveformOf(const ChannelInfo& channel) -> std::optional<Waveform> {
    auto start = getProperty<TimeStamp>(channel.properties, WaveformProperty::StartTime);
    auto increment = getProperty<double>(channel.properties, WaveformProperty::Increment);
    if (!start || !increment)
        return std::nullopt;
    return Waveform{*start, *increment, getProperty<double>(channel.properties, WaveformProperty::StartOffset).value_or(0.0)};
}

}  // namespace TDMS

#endif
