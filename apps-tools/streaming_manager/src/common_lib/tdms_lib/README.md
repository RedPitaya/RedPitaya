# tdms_lib

Reading and writing NI TDMS files: properties of any kind at the file, group and channel
level, and samples by channel. Everything is in one include:

```cpp
#include "tdms_lib/tdms.h"
```

| Type | For |
|---|---|
| `Value`, `TimeStamp`, `Properties` | property values |
| `SegmentBuilder` | one segment: properties and samples |
| `FileWriter` | a file written segment by segment |
| `FileReader` | a file read by channel |
| `Waveform`, `setWaveform`, `waveformOf` | the NI time properties of a channel |

`File`, `Reader` and `Writer` (`file.h`) are the former API. They stay until reader_lib and
writer_lib use this one, then go.

## The file

A TDMS file holds a file object, groups and channels (`/`, `/'group'`, `/'group'/'channel'`).
Every object can carry properties: a name and a typed value. A channel holds samples of one
type. The file is a sequence of segments; each segment adds samples to channels and may set
properties. A property keeps its last written value.

## Copies

Streaming on the board is limited by copying memory, so the API states every copy:

| Call | Samples |
|---|---|
| `SegmentBuilder::addSamples(..., shared_ptr)` | referenced: written to the file from the caller's memory |
| `SegmentBuilder::copySamples(..., pointer or vector)` | copied into the builder |
| `SegmentBuilder::parts()` | the segment as a few pieces: a small head, then the sample blocks in place |
| `FileWriter::write(segment)` | the parts go to the file one after another, no buffer in between |
| `FileReader::read(..., void* out)` | from the file straight into the caller's memory |
| `FileReader::read<T>()`, `readAll<T>()` | from the file straight into the returned vector |
| `SegmentBuilder::bytes()` | the whole segment in a string (tests, small segments) |

A referenced block must not change until its segment is written. Any owner can be referenced
with the aliasing constructor: `std::shared_ptr<const std::uint8_t[]>(owner, pointer)`.

## Writing

One segment per buffer:

```cpp
TDMS::FileWriter file;
if (!file.open(path))
    return error(file.error());

bool first = true;
while (auto buffer = nextBuffer()) {
    TDMS::SegmentBuilder s;
    if (first) {
        first = false;
        s.setProperty("device", "STEMlab 125-14");
        s.setGroupProperty("Group", "decimation", uint32_t(8));
        TDMS::setWaveform(s, "Group", "ch1", {TDMS::TimeStamp::fromUnixNs(startNs), 64e-9});
        s.setChannelProperty("Group", "ch1", "unit_string", "V");
    }
    s.setGroupProperty("Group", "lost", lostSoFar);       // updated with every buffer
    s.addSamples("Group", "ch1", buffer->ch1);            // std::shared_ptr<std::vector<int16_t>>: Integer16
    s.addSamples("Group", "ch2", TDMS::TDMSType::Integer16,
                 std::shared_ptr<const std::uint8_t[]>(buffer, buffer->ch2Data()), buffer->samples());
    if (!file.write(s))
        return error(file.error());                       // disk full
}
file.close();
```

The builder writes every object it was given, so the columns of a file are whatever the
recorder puts into it, and a channel may be left out of some segments. Groups need no
declaration. A segment can also go to any `std::ostream` with `write()`, or to a file thread
as `parts()`.

## Reading

`open()` walks the file once and notes where the samples of every channel are; no samples are
read until asked for. A file of a long recording opens in moments, and channels that are not
needed cost nothing.

```cpp
TDMS::FileReader f;
if (!f.open(path))
    return error(f.error());

for (const auto& c : f.channels())                       // pick the columns
    std::cout << c.group << "/" << c.name << ": " << c.count << " samples\n";

auto decimation = TDMS::getProperty<uint32_t>(f.groupProperties("Group"), "decimation");
auto ch1 = f.read<int16_t>("Group", "ch1", 0, 16384);   // T must match the channel type
uint64_t n = f.read("Group", "ch2", first, count, dacBlock);   // straight into a buffer
```

`read` returns the samples it could read: fewer at the end of the channel, none for a channel
that is not there or holds strings.

Read correctly: TDMS 1.0 and 2.0 files of other writers (LabVIEW, DIAdem, npTDMS) with
incremental metadata, the "same index as before" and "no data" raw data indexes, several chunks
of samples in a segment, interleaved channels, and a file cut off in its last segment (the whole
samples that are there count; a segment cut in its metadata is left out and `truncated()` says
so). Refused with an error: big-endian files, DAQmx raw data. String channels are listed with
their count; their samples are not read.

## Properties

`Value` takes its TDMS type from the C++ type: `int8_t` … `uint64_t`, `float`, `double`, `bool`,
`std::string` (or a string literal), `TimeStamp`. Plain `char` is refused: its signedness depends
on the platform.

`get<T>()` converts only when the value fits: `Value(int32_t(300)).get<int8_t>()` and
`Value(8.5).get<uint32_t>()` give nothing. `getProperty<T>(properties, key)` does the lookup too.

`TimeStamp` is the TDMS time: seconds since 1904-01-01 UTC and a fraction in 2^-64 s;
`fromUnix(seconds, ns)`, `fromUnixNs(ns)` and `unixNs()` convert to nanoseconds exactly.

## Waveforms

LabVIEW, DIAdem and npTDMS give the samples of a channel their time from `wf_start_time`,
`wf_start_offset` and `wf_increment`: sample i is at start + offset + i · increment.
`setWaveform` writes them, `waveformOf` reads them. They hold for evenly spaced samples; after
a gap in a recording whose lost samples are not filled, later samples would get wrong times.

## Threads

A `SegmentBuilder`, `FileWriter` or `FileReader` is used by one thread at a time. A reader moves
the position of its stream on every read; open one reader per thread to read in parallel.

## Tests

`tests/`: GoogleTest suites, run after every build. What the writer produces and what the
reader has to accept are pinned as bytes stated by hand from the TDMS 2.0 specification
(`tests/support/spec_builder.h`, which shares no code with the library). Corrupted and cut off
files must open or fail cleanly. `tests/compat/compat_nptdms.py` checks both directions against
[npTDMS](https://github.com/adamreeve/npTDMS); it runs when Python has `npTDMS` installed
(`pip install npTDMS`, the interpreter set by `TDMS_COMPAT_PYTHON`) and is reported skipped
otherwise.
